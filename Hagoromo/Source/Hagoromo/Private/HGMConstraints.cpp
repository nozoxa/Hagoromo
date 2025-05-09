// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HGMConstraints.h"
#include "HagoromoModule.h"
#include "HGMDebug.h"
#include "AnimNode_Hagoromo.h"
#include "HGMCollision.h"
#include "HGMSolvers.h"
#include "HGMAnimation.h"

#include "Animation/AnimNodeBase.h"

DECLARE_CYCLE_STAT(TEXT("Constraint VerticalStructuralConstraint"), STAT_ConstraintVerticalStructuralConstraint, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Constraint HorizontalStructuralConstraint"), STAT_ConstraintHorizontalStructuralConstraint, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Constraint ShearConstraint"), STAT_ConstraintShearConstraint, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Constraint VerticalBendConstraint"), STAT_ConstraintVerticalBendConstraint, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Constraint HorizontalBendConstraint"), STAT_ConstraintHorizontalBendConstraint, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Constraint RelativeLimitAngleConstraint"), STAT_ConstraintRelativeLimitAngleConstraint, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Constraint AnimPoseMovableRadiusConstraint"), STAT_ConstraintAnimPoseMovableRadiusConstraint, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Constraint AnimPoseLimitAngleConstraint"), STAT_ConstraintAnimPoseLimitAngleConstraint, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Constraint AnimPosePlanarConstraint"), STAT_ConstraintAnimPosePlanarConstraint, STATGROUP_Hagoromo);

namespace
{
	static FHGMSIMDReal ComputeDeltaLambda(const FHGMSIMDReal& sLength, const FHGMSIMDVector3& sDirection, const FHGMSIMDReal& sDesiredLength,
											const FHGMSIMDReal& sInverseSumMass, const FHGMSIMDReal& sCompliance, const FHGMSIMDReal& sLambda, const FHGMSIMDReal& sDeltaTimeSquared)
	{
		// Constrained only when extended to prevent vibration.
		const FHGMSIMDReal sConstraint = FHGMMathLibrary::Max(HGMSIMDConstants::ZeroReal, sLength - sDesiredLength);
		const FHGMSIMDReal sComplianceTilda = sCompliance / sDeltaTimeSquared;
		return (sConstraint - sComplianceTilda * sLambda) / (sInverseSumMass + sComplianceTilda);
	}
}


void FHGMConstraintLibrary::FixedBlendConstraint(TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDVector3>& PrevPositions, const TArray<FHGMSIMDVector3>& AnimPositions, const TArray<FHGMSIMDReal>& FixedBlends)
{
	for (int32 PackedIndex { 0 }; PackedIndex < Positions.Num(); PackedIndex++)
	{
		FHGMSIMDVector3& sPosition = Positions[PackedIndex];
		FHGMSIMDVector3& sPrevPosition = PrevPositions[PackedIndex];
		const FHGMSIMDVector3& sAnimPosition = AnimPositions[PackedIndex];
		const FHGMSIMDReal& sFixedBlend = FixedBlends[PackedIndex];

		sPosition = FHGMMathLibrary::Lerp(sPosition, sAnimPosition, sFixedBlend);
		sPrevPosition = FHGMMathLibrary::Lerp(sPrevPosition, sAnimPosition, sFixedBlend);
	}
}


void FHGMConstraintLibrary::ColliderContactConstraint(const TArray<FHGMSIMDColliderContact>& Contacts, const FHGMSIMDReal& sCollisionBlend, const FHGMSIMDReal& sColliderPenetrationDepth, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks)
{
	for (const FHGMSIMDColliderContact& Contact : Contacts)
	{
		FHGMSIMDVector3& sPosition = Positions[Contact.PackedIndex];
		const FHGMSIMDReal sPositionOffset = FHGMMathLibrary::DotProduct(sPosition, Contact.sSeparatingNormal);
		FHGMSIMDReal sPushAmount = FHGMMathLibrary::Max(Contact.sSeparatingOffset - sPositionOffset, HGMSIMDConstants::ZeroReal);
		FHGMSIMDReal sPushAmountWithBlend = sPushAmount * sCollisionBlend;
		FHGMSIMDReal sPushAmountWithPenetrationDepth = sPushAmountWithBlend - sColliderPenetrationDepth;
		FHGMSIMDReal sFinalPushAmount = FHGMSIMDLibrary::Select(sPushAmountWithPenetrationDepth > HGMSIMDConstants::ZeroReal, sPushAmountWithPenetrationDepth, sPushAmountWithBlend);

		sPosition += sFinalPushAmount * Contact.sSeparatingNormal * (HGMSIMDConstants::OneReal - FixedBlends[Contact.PackedIndex]) * (HGMSIMDConstants::OneReal - DummyBoneMasks[Contact.PackedIndex]);
	}
}


void FHGMConstraintLibrary::MakeVerticalStructure(const FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDVector3> Positions, TArray<FHGMSIMDStructure>& OutVerticalStructures)
{
	for (int32 PackedIndex = 0; PackedIndex < Positions.Num(); ++PackedIndex)
	{
		// Process ends here because no constraint target is found at end.
		const int32 VerticalBoneStep = PackedIndex / SimulationPlane.PackedHorizontalBoneNum;
		const bool bIsEndVerticalBone = (VerticalBoneStep == (SimulationPlane.UnpackedVerticalBoneNum - 1));
		if (bIsEndVerticalBone)
		{
			break;
		}

		const int32 LinkFirstBoneIndex = PackedIndex;
		const FHGMSIMDVector3& sLinkFirstBone = Positions[LinkFirstBoneIndex];

		FHGMSIMDInt sUnpackedLinkFirstBoneIndex {};
		const int32 UnpackedLinkFirstIndex = LinkFirstBoneIndex * 4;
		FHGMSIMDLibrary::Load(sUnpackedLinkFirstBoneIndex, UnpackedLinkFirstIndex + 0, UnpackedLinkFirstIndex + 1, UnpackedLinkFirstIndex + 2, UnpackedLinkFirstIndex + 3);

		FHGMSIMDInt sUnpackedLinkSecondBoneIndex {};
		const int32 LinkSecondBoneIndex = PackedIndex + SimulationPlane.PackedHorizontalBoneNum;
		const FHGMSIMDVector3& sLinkSecondBone = Positions[LinkSecondBoneIndex];
		const int32 UnpackedLinkSecondIndex = LinkSecondBoneIndex * 4;
		FHGMSIMDLibrary::Load(sUnpackedLinkSecondBoneIndex, UnpackedLinkSecondIndex + 0, UnpackedLinkSecondIndex + 1, UnpackedLinkSecondIndex + 2, UnpackedLinkSecondIndex + 3);

		FHGMSIMDStructure Constraint {};
		Constraint.FirstBonePackedIndex = LinkFirstBoneIndex;
		Constraint.SecondBonePackedIndex = LinkSecondBoneIndex;
		Constraint.sFirstBoneUnpackedIndex = sUnpackedLinkFirstBoneIndex;
		Constraint.sSecondBoneUnpackedIndex = sUnpackedLinkSecondBoneIndex;
		Constraint.sLength = FHGMMathLibrary::Length(sLinkSecondBone - sLinkFirstBone);
		Constraint.sLambda = HGMSIMDConstants::ZeroReal;

		OutVerticalStructures.Emplace(MoveTemp(Constraint));
	}
}


void FHGMConstraintLibrary::VerticalStructuralConstraint(TArrayView<FHGMSIMDStructure> Structures, FHGMPhysicsContext& PhysicsContext, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> InverseMasses, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintVerticalStructuralConstraint);

	const FHGMSIMDReal sDeltaTimeSquared = PhysicsContext.sDeltaTime * PhysicsContext.sDeltaTime;

	FHGMSIMDReal sStructureStiffness {};
	FHGMSIMDLibrary::Load(sStructureStiffness, PhysicsContext.PhysicsSettings.StructureStiffness);

	for (FHGMSIMDStructure& Structure : Structures)
	{
		FHGMSIMDVector3& sFirstBonePosition = Positions[Structure.FirstBonePackedIndex];
		FHGMSIMDVector3& sSecondBonePosition = Positions[Structure.SecondBonePackedIndex];

		const FHGMSIMDVector3 sToFirst = sFirstBonePosition - sSecondBonePosition;
		const FHGMSIMDReal sCurrentLenght =FHGMMathLibrary::Length(sToFirst);
		const FHGMSIMDVector3 sToFirstDirection = FHGMMathLibrary::MakeSafeNormal(sToFirst);

		const FHGMSIMDReal& sFirstInverseMass = InverseMasses[Structure.FirstBonePackedIndex];
		const FHGMSIMDReal& sSecondInverseMass = InverseMasses[Structure.SecondBonePackedIndex];

		const FHGMSIMDReal& sFirstBoneFixedBlend = FixedBlends[Structure.FirstBonePackedIndex];
		const FHGMSIMDReal& sSecondBoneFixedBlend = FixedBlends[Structure.SecondBonePackedIndex];

		const FHGMSIMDReal& sFirstBoneDummyMask = DummyBoneMasks[Structure.FirstBonePackedIndex];
		const FHGMSIMDReal& sSecondBoneDummyMask = DummyBoneMasks[Structure.SecondBonePackedIndex];

		FHGMSIMDReal sFirstBoneCoefficient = HGMSIMDConstants::OneReal;
		sFirstBoneCoefficient *= (HGMSIMDConstants::OneReal - sFirstBoneFixedBlend);
		sFirstBoneCoefficient *= (HGMSIMDConstants::OneReal - sSecondBoneDummyMask);

		FHGMSIMDReal sSecondBoneCoefficient = HGMSIMDConstants::OneReal;
		sSecondBoneCoefficient *= (HGMSIMDConstants::OneReal - sSecondBoneFixedBlend);
		sSecondBoneCoefficient *= (HGMSIMDConstants::OneReal - sFirstBoneDummyMask);

		FHGMSIMDReal sDeltaLambda = ComputeDeltaLambda(sCurrentLenght, sToFirstDirection, Structure.sLength, sFirstInverseMass + sSecondInverseMass, sStructureStiffness, Structure.sLambda, sDeltaTimeSquared);
		sDeltaLambda = FHGMSIMDLibrary::Select(sFirstBoneCoefficient <= HGMSIMDConstants::ZeroReal & sSecondBoneCoefficient <= HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal, sDeltaLambda);
		Structure.sLambda += sDeltaLambda;

		sFirstBonePosition -= sToFirstDirection * sDeltaLambda * sFirstBoneCoefficient * sFirstInverseMass;
		sSecondBonePosition += sToFirstDirection * sDeltaLambda * sSecondBoneCoefficient * sSecondInverseMass;
	}
}


void FHGMConstraintLibrary::RigidVerticalStructuralConstraint(TArrayView<FHGMSIMDStructure> Structures, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> FixedBlends)
{
	for (FHGMSIMDStructure& Structure : Structures)
	{
		FHGMSIMDVector3& sFirstBonePosition = Positions[Structure.FirstBonePackedIndex];
		FHGMSIMDVector3& sSecondBonePosition = Positions[Structure.SecondBonePackedIndex];

		const FHGMSIMDVector3 sToFirst = sFirstBonePosition - sSecondBonePosition;
		const FHGMSIMDReal sCurrentLenght =FHGMMathLibrary::Length(sToFirst);
		const FHGMSIMDVector3 sToFirstDirection = FHGMMathLibrary::MakeSafeNormal(sToFirst);

		const FHGMSIMDVector3 sConstraintedSecondBonePosition = (sCurrentLenght - Structure.sLength) * sToFirstDirection + sSecondBonePosition;
		sSecondBonePosition = FHGMSIMDLibrary::Select(FixedBlends[Structure.SecondBonePackedIndex] <= HGMSIMDConstants::ZeroReal, sConstraintedSecondBonePosition, sSecondBonePosition);
	}
}


void FHGMConstraintLibrary::MakeHorizontalStructure(FHGMSimulationPlane& SimulationPlane, TArray<FHGMSIMDVector3>& Positions, bool bLoopHorizontalStructure, TArray<FHGMSIMDStructure>& OutHorizontalStructures)
{
	FScopedTranspose ScopedTranspose(&SimulationPlane);
	ScopedTranspose.Add(&Positions);
	ScopedTranspose.Execute();

	FHGMConstraintLibrary::MakeVerticalStructure(SimulationPlane, Positions, OutHorizontalStructures);

	if (bLoopHorizontalStructure)
	{
		const int32 LastVerticalChainPackedIndex = (SimulationPlane.ActualUnpackedHorizontalBoneNum - 1) * SimulationPlane.PackedHorizontalBoneNum;
		for (int32 PackedVerticalBoneOffset = 0; PackedVerticalBoneOffset < SimulationPlane.PackedVerticalBoneNum; ++PackedVerticalBoneOffset)
		{
			const int32 PackedLinkFirstBoneIndex = LastVerticalChainPackedIndex + PackedVerticalBoneOffset;
			FHGMSIMDInt sUnpackedLinkFirstBoneIndex {};
			const int32 UnpackedLinkFirstIndex = PackedLinkFirstBoneIndex * 4;
			FHGMSIMDLibrary::Load(sUnpackedLinkFirstBoneIndex, UnpackedLinkFirstIndex + 0, UnpackedLinkFirstIndex + 1, UnpackedLinkFirstIndex + 2, UnpackedLinkFirstIndex + 3);

			const int32 PackedLinkSecondBoneIndex = PackedVerticalBoneOffset;
			FHGMSIMDInt sUnpackedLinkSecondBoneIndex {};
			const int32 UnpackedLinkSecondIndex = PackedLinkSecondBoneIndex * 4;
			FHGMSIMDLibrary::Load(sUnpackedLinkSecondBoneIndex, UnpackedLinkSecondIndex + 0, UnpackedLinkSecondIndex + 1, UnpackedLinkSecondIndex + 2, UnpackedLinkSecondIndex + 3);

			FHGMSIMDStructure Constraint {};
			Constraint.FirstBonePackedIndex = PackedLinkFirstBoneIndex;
			Constraint.SecondBonePackedIndex = PackedLinkSecondBoneIndex;
			Constraint.sFirstBoneUnpackedIndex = sUnpackedLinkFirstBoneIndex;
			Constraint.sSecondBoneUnpackedIndex = sUnpackedLinkSecondBoneIndex;
			Constraint.sLength = FHGMMathLibrary::Length(Positions[PackedLinkSecondBoneIndex] - Positions[PackedLinkFirstBoneIndex]);
			Constraint.sLambda = HGMSIMDConstants::ZeroReal;

			OutHorizontalStructures.Emplace(MoveTemp(Constraint));
		}
	}
}


void FHGMConstraintLibrary::HorizontalStructuralConstraint(TArrayView<FHGMSIMDStructure> Structures, FHGMPhysicsContext& PhysicsContext, FHGMSimulationPlane& SimulationPlane, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDReal>& InverseMasses, TArray<FHGMSIMDReal>& FixedBlends, TArray<FHGMSIMDReal>& DummyBoneMasks)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintHorizontalStructuralConstraint);

	FScopedTranspose ScopedTranspose(&SimulationPlane);
	ScopedTranspose.Add(&Positions);
	ScopedTranspose.Add(&InverseMasses);
	ScopedTranspose.Add(&FixedBlends);
	ScopedTranspose.Add(&DummyBoneMasks);
	ScopedTranspose.Execute();

	FHGMConstraintLibrary::VerticalStructuralConstraint(Structures, PhysicsContext, Positions, InverseMasses, FixedBlends, DummyBoneMasks);
}


void FHGMConstraintLibrary::MakeShearStructure(const FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDVector3> Positions, bool bLoopHorizontalStructure, TArray<FHGMSIMDShearStructure>& OutShearStructures)
{
	OutShearStructures.Reset((SimulationPlane.PackedVerticalBoneNum - 1) * SimulationPlane.ActualPackedHorizontalBoneNum * 2);

	int32 PackedIndex = 0;
	while(PackedIndex < Positions.Num())
	{
		// Process ends here because no constraint target is found at end.
		const int32 VerticalBoneStep = PackedIndex / SimulationPlane.PackedHorizontalBoneNum;
		const bool bIsEndVerticalBone = (VerticalBoneStep == (SimulationPlane.UnpackedVerticalBoneNum - 1));
		if (bIsEndVerticalBone)
		{
			break;
		}

		int32 LoopIncreaseValue = 1;
		int32 RightUnpackedEndIndex = 0;
		int32 LowerRightUnpackedEndIndex = 0;
		int32 EndComponentIndex = 3;

		bool bIsEndHorizontalBone = (PackedIndex % SimulationPlane.PackedHorizontalBoneNum) + 1 == SimulationPlane.ActualPackedHorizontalBoneNum;
		if (bIsEndHorizontalBone)
		{
			LoopIncreaseValue = (SimulationPlane.PackedHorizontalBoneNum - SimulationPlane.ActualPackedHorizontalBoneNum) + 1;
			EndComponentIndex = (SimulationPlane.ActualUnpackedHorizontalBoneNum - 1) % 4;

			if (bLoopHorizontalStructure)
			{
				RightUnpackedEndIndex = (VerticalBoneStep * SimulationPlane.PackedHorizontalBoneNum) * 4;
				LowerRightUnpackedEndIndex = ((VerticalBoneStep + 1) * SimulationPlane.PackedHorizontalBoneNum) * 4;
			}
		}
		else
		{
			const int32 LowerBonePackedIndex = PackedIndex + SimulationPlane.PackedHorizontalBoneNum;
			const int32 LowerRightPackedIndex = LowerBonePackedIndex + 1;
			LowerRightUnpackedEndIndex = LowerRightPackedIndex * 4;

			const int32 RightBonePackedIndex = PackedIndex + 1;
			RightUnpackedEndIndex = RightBonePackedIndex * 4;
		}

		const int32 LowerBonePackedIndex = PackedIndex + SimulationPlane.PackedHorizontalBoneNum;

		// Diagonal constraint in lower right direction.
		FHGMSIMDShearStructure sLowerRightDiagonalShear {};
		FHGMSIMDVector3 sFirstBonePosition {};
		FHGMSIMDLibrary::Load(sLowerRightDiagonalShear.sFirstBoneUnpackedIndex, PackedIndex * 4 + 0, PackedIndex * 4 + 1, PackedIndex * 4 + 2, PackedIndex * 4 + 3);
		FHGMSIMDLibrary::Store(Positions, sLowerRightDiagonalShear.sFirstBoneUnpackedIndex, sFirstBonePosition);

		FHGMSIMDVector3 sSecondBonePosition {};
		FHGMSIMDLibrary::Load(sLowerRightDiagonalShear.sSecondBoneUnpackedIndex, LowerBonePackedIndex * 4 + 1, LowerBonePackedIndex * 4 + 2, LowerBonePackedIndex * 4 + 3, LowerRightUnpackedEndIndex);
		if (bLoopHorizontalStructure)
		{
			FHGMSIMDLibrary::Load(sLowerRightDiagonalShear.sSecondBoneUnpackedIndex, EndComponentIndex, LowerRightUnpackedEndIndex);
		}

		FHGMSIMDLibrary::Store(Positions, sLowerRightDiagonalShear.sSecondBoneUnpackedIndex, sSecondBonePosition);

		sLowerRightDiagonalShear.sLength = FHGMMathLibrary::Length(sSecondBonePosition - sFirstBonePosition);
		sLowerRightDiagonalShear.sLambda = HGMSIMDConstants::ZeroReal;
		OutShearStructures.Emplace(MoveTemp(sLowerRightDiagonalShear));

		// Diagonal constraint in lower left direction.
		FHGMSIMDShearStructure sLowerLeftDiagonalShear {};
		FHGMSIMDLibrary::Load(sLowerLeftDiagonalShear.sFirstBoneUnpackedIndex, PackedIndex * 4 + 1, PackedIndex * 4 + 2, PackedIndex * 4 + 3, RightUnpackedEndIndex);
		if (bLoopHorizontalStructure)
		{
			FHGMSIMDLibrary::Load(sLowerLeftDiagonalShear.sFirstBoneUnpackedIndex, EndComponentIndex, RightUnpackedEndIndex);
		}

		FHGMSIMDLibrary::Store(Positions, sLowerLeftDiagonalShear.sFirstBoneUnpackedIndex, sFirstBonePosition);

		FHGMSIMDLibrary::Load(sLowerLeftDiagonalShear.sSecondBoneUnpackedIndex, LowerBonePackedIndex * 4 + 0, LowerBonePackedIndex * 4 + 1, LowerBonePackedIndex * 4 + 2, LowerBonePackedIndex * 4 + 3);
		FHGMSIMDLibrary::Store(Positions, sLowerLeftDiagonalShear.sSecondBoneUnpackedIndex, sSecondBonePosition);

		sLowerLeftDiagonalShear.sLength = FHGMMathLibrary::Length(sSecondBonePosition - sFirstBonePosition);
		sLowerLeftDiagonalShear.sLambda = HGMSIMDConstants::ZeroReal;
		OutShearStructures.Emplace(MoveTemp(sLowerLeftDiagonalShear));

		PackedIndex += LoopIncreaseValue;
	}
}


void FHGMConstraintLibrary::ShearConstraint(TArrayView<FHGMSIMDShearStructure> Shears, FHGMPhysicsContext& PhysicsContext, const FHGMSimulationPlane& SimulationPlane, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> InverseMasses, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintShearConstraint);

	const FHGMSIMDReal sDeltaTimeSquared = PhysicsContext.sDeltaTime * PhysicsContext.sDeltaTime;

	FHGMSIMDReal sStiffness {};
	FHGMSIMDLibrary::Load(sStiffness, PhysicsContext.PhysicsSettings.ShearStiffness);

	for (int32 ShearIndexBase = 0; ShearIndexBase < Shears.Num(); ShearIndexBase += 2)
	{
		const int32 HorizontalChainIndex = ShearIndexBase / 2;
		bool bIsEndHorizontalBone = (HorizontalChainIndex % SimulationPlane.ActualPackedHorizontalBoneNum) + 1 == SimulationPlane.ActualPackedHorizontalBoneNum;

		int32 EndComponentIndex = 3;
		if (bIsEndHorizontalBone)
		{
			EndComponentIndex = (SimulationPlane.ActualUnpackedHorizontalBoneNum - 1) % 4;
		}

		for (int32 Offset = 0; Offset < 2; ++Offset)
		{
			const int32 ShearIndex = ShearIndexBase + Offset;
			FHGMSIMDShearStructure& sShear = Shears[ShearIndex];

			FHGMSIMDVector3 sFirstBonePosition {};
			FHGMSIMDLibrary::Store(Positions, sShear.sFirstBoneUnpackedIndex, sFirstBonePosition);

			FHGMSIMDVector3 sSecondBonePosition {};
			FHGMSIMDLibrary::Store(Positions, sShear.sSecondBoneUnpackedIndex, sSecondBonePosition);

			FHGMSIMDReal sFirstInverseMass {};
			FHGMSIMDLibrary::Store(InverseMasses, sShear.sFirstBoneUnpackedIndex, sFirstInverseMass);

			FHGMSIMDReal sSecondInverseMass {};
			FHGMSIMDLibrary::Store(InverseMasses, sShear.sSecondBoneUnpackedIndex, sSecondInverseMass);

			FHGMSIMDReal sFirstBoneFixedBlend {};
			FHGMSIMDLibrary::Store(FixedBlends, sShear.sFirstBoneUnpackedIndex, sFirstBoneFixedBlend);

			FHGMSIMDReal sSecondBoneFixedBlend {};
			FHGMSIMDLibrary::Store(FixedBlends, sShear.sSecondBoneUnpackedIndex, sSecondBoneFixedBlend);

			FHGMSIMDReal sFirstBoneDummyMask {};
			FHGMSIMDLibrary::Store(DummyBoneMasks, sShear.sFirstBoneUnpackedIndex, sFirstBoneDummyMask);

			FHGMSIMDReal sSecondBoneDummyMask {};
			FHGMSIMDLibrary::Store(DummyBoneMasks, sShear.sSecondBoneUnpackedIndex, sSecondBoneDummyMask);

			const FHGMSIMDVector3 sToFirst = sFirstBonePosition - sSecondBonePosition;
			const FHGMSIMDReal sCurrentLenght = FHGMMathLibrary::Length(sToFirst);
			const FHGMSIMDVector3 sToFirstDirection = FHGMMathLibrary::MakeSafeNormal(sToFirst);

			FHGMSIMDReal sFirstBoneCoefficient = HGMSIMDConstants::OneReal;
			sFirstBoneCoefficient *= (HGMSIMDConstants::OneReal - sFirstBoneFixedBlend);
			sFirstBoneCoefficient *= (HGMSIMDConstants::OneReal - sSecondBoneDummyMask);

			FHGMSIMDReal sSecondBoneCoefficient = HGMSIMDConstants::OneReal;
			sSecondBoneCoefficient *= (HGMSIMDConstants::OneReal - sSecondBoneFixedBlend);
			sSecondBoneCoefficient *= (HGMSIMDConstants::OneReal - sFirstBoneDummyMask);

			FHGMSIMDReal sDeltaLambda = ComputeDeltaLambda(sCurrentLenght, sToFirstDirection, sShear.sLength, sFirstInverseMass + sSecondInverseMass, sStiffness, sShear.sLambda, sDeltaTimeSquared);
			sDeltaLambda = FHGMSIMDLibrary::Select(sFirstBoneCoefficient <= HGMSIMDConstants::ZeroReal & sSecondBoneCoefficient <= HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal, sDeltaLambda);

			sShear.sLambda += sDeltaLambda;
			sFirstBonePosition -= sToFirstDirection * sDeltaLambda * sFirstBoneCoefficient * sFirstInverseMass;
			sSecondBonePosition += sToFirstDirection * sDeltaLambda * sSecondBoneCoefficient * sSecondInverseMass;

			if (bIsEndHorizontalBone)
			{
				TStaticArray<int32, 4> FirstBoneUnpackedIndexes {};
				FHGMSIMDLibrary::Store(sShear.sFirstBoneUnpackedIndex, FirstBoneUnpackedIndexes);

				TStaticArray<int32, 4> SecondBoneUnpackedIndexes {};
				FHGMSIMDLibrary::Store(sShear.sSecondBoneUnpackedIndex, SecondBoneUnpackedIndexes);

				TStaticArray<FHGMVector3, 4> FirstBoneUnpackedPositions {};
				FHGMSIMDLibrary::Store(sFirstBonePosition, FirstBoneUnpackedPositions);

				TStaticArray<FHGMVector3, 4> SecondBoneUnpackedPositions {};
				FHGMSIMDLibrary::Store(sSecondBonePosition, SecondBoneUnpackedPositions);

				for (int32 ComponentIndex = 0; ComponentIndex <= EndComponentIndex; ++ComponentIndex)
				{
					FHGMSIMDLibrary::Load(Positions, FHGMSIMDIndex(FirstBoneUnpackedIndexes[ComponentIndex]), FirstBoneUnpackedPositions[ComponentIndex]);
					FHGMSIMDLibrary::Load(Positions, FHGMSIMDIndex(SecondBoneUnpackedIndexes[ComponentIndex]), SecondBoneUnpackedPositions[ComponentIndex]);
				}
			}
			else
			{
				FHGMSIMDLibrary::Load(Positions, sShear.sFirstBoneUnpackedIndex, sFirstBonePosition);
				FHGMSIMDLibrary::Load(Positions, sShear.sSecondBoneUnpackedIndex, sSecondBonePosition);
			}
		}
	}
}


void FHGMConstraintLibrary::MakeVerticalBendStructure(const FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDVector3> Positions, TArray<FHGMSIMDStructure>& OutVerticalBendStructures)
{
	OutVerticalBendStructures.Reset(SimulationPlane.PackedHorizontalBoneNum * SimulationPlane.UnpackedVerticalBoneNum);

	for (int32 PackedIndex = 0; PackedIndex < Positions.Num(); ++PackedIndex)
	{
		const int32 VerticalBoneStep = PackedIndex / SimulationPlane.PackedHorizontalBoneNum;
		const bool bIsEndVerticalBone = (VerticalBoneStep == (SimulationPlane.UnpackedVerticalBoneNum - 2));
		if (bIsEndVerticalBone)
		{
			break;
		}

		const int32 LinkFirstBoneIndex = PackedIndex;
		const FHGMSIMDVector3& sLinkFirstBone = Positions[LinkFirstBoneIndex];

		FHGMSIMDInt sUnpackedLinkFirstBoneIndex {};
		const int32 UnpackedLinkFirstIndex = LinkFirstBoneIndex * 4;
		FHGMSIMDLibrary::Load(sUnpackedLinkFirstBoneIndex, UnpackedLinkFirstIndex + 0, UnpackedLinkFirstIndex + 1, UnpackedLinkFirstIndex + 2, UnpackedLinkFirstIndex + 3);

		FHGMSIMDInt sUnpackedLinkSecondBoneIndex {};
		const int32 LinkSecondBoneIndex = PackedIndex + (SimulationPlane.PackedHorizontalBoneNum * 2);
		const FHGMSIMDVector3& sLinkSecondBone = Positions[LinkSecondBoneIndex];
		const int32 UnpackedLinkSecondIndex = LinkSecondBoneIndex * 4;
		FHGMSIMDLibrary::Load(sUnpackedLinkSecondBoneIndex, UnpackedLinkSecondIndex + 0, UnpackedLinkSecondIndex + 1, UnpackedLinkSecondIndex + 2, UnpackedLinkSecondIndex + 3);

		FHGMSIMDStructure Constraint {};
		Constraint.FirstBonePackedIndex = LinkFirstBoneIndex;
		Constraint.SecondBonePackedIndex = LinkSecondBoneIndex;
		Constraint.sFirstBoneUnpackedIndex = sUnpackedLinkFirstBoneIndex;
		Constraint.sSecondBoneUnpackedIndex = sUnpackedLinkSecondBoneIndex;
		Constraint.sLength = FHGMMathLibrary::Length(sLinkSecondBone - sLinkFirstBone);
		Constraint.sLambda = HGMSIMDConstants::ZeroReal;

		OutVerticalBendStructures.Emplace(MoveTemp(Constraint));
	}
}


void FHGMConstraintLibrary::VerticalBendConstraint(TArrayView<FHGMSIMDStructure> BendStructures, FHGMPhysicsContext& PhysicsContext, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> InverseMasses, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintVerticalBendConstraint);

	const FHGMSIMDReal sDeltaTimeSquared = PhysicsContext.sDeltaTime * PhysicsContext.sDeltaTime;

	FHGMSIMDReal sStiffness {};
	FHGMSIMDLibrary::Load(sStiffness, PhysicsContext.PhysicsSettings.VerticalBendStiffness);

	for (FHGMSIMDStructure& Structure : BendStructures)
	{
		FHGMSIMDVector3& sFirstBonePosition = Positions[Structure.FirstBonePackedIndex];
		FHGMSIMDVector3& sSecondBonePosition = Positions[Structure.SecondBonePackedIndex];

		const FHGMSIMDVector3 sToFirst = sFirstBonePosition - sSecondBonePosition;
		const FHGMSIMDReal sCurrentLenght =FHGMMathLibrary::Length(sToFirst);
		const FHGMSIMDVector3 sToFirstDirection = FHGMMathLibrary::MakeSafeNormal(sToFirst);

		const FHGMSIMDReal& sFirstInverseMass = InverseMasses[Structure.FirstBonePackedIndex];
		const FHGMSIMDReal& sSecondInverseMass = InverseMasses[Structure.SecondBonePackedIndex];

		const FHGMSIMDReal& sFirstBoneFixedBlend = FixedBlends[Structure.FirstBonePackedIndex];
		const FHGMSIMDReal& sSecondBoneFixedBlend = FixedBlends[Structure.SecondBonePackedIndex];

		const FHGMSIMDReal& sFirstBoneDummyMask = DummyBoneMasks[Structure.FirstBonePackedIndex];
		const FHGMSIMDReal& sSecondBoneDummyMask = DummyBoneMasks[Structure.SecondBonePackedIndex];

		FHGMSIMDReal sFirstBoneCoefficient = HGMSIMDConstants::OneReal;
		sFirstBoneCoefficient *= (HGMSIMDConstants::OneReal - sFirstBoneFixedBlend);
		sFirstBoneCoefficient *= (HGMSIMDConstants::OneReal - sSecondBoneDummyMask);

		FHGMSIMDReal sSecondBoneCoefficient = HGMSIMDConstants::OneReal;
		sSecondBoneCoefficient *= (HGMSIMDConstants::OneReal - sSecondBoneFixedBlend);
		sSecondBoneCoefficient *= (HGMSIMDConstants::OneReal - sFirstBoneDummyMask);

		FHGMSIMDReal sDeltaLambda = ComputeDeltaLambda(sCurrentLenght, sToFirstDirection, Structure.sLength, sFirstInverseMass + sSecondInverseMass, sStiffness, Structure.sLambda, sDeltaTimeSquared);
		sDeltaLambda = FHGMSIMDLibrary::Select(sFirstBoneCoefficient <= HGMSIMDConstants::ZeroReal & sSecondBoneCoefficient <= HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal, sDeltaLambda);

		Structure.sLambda += sDeltaLambda;
		sFirstBonePosition -= sToFirstDirection * sDeltaLambda * sFirstBoneCoefficient * sFirstInverseMass;
		sSecondBonePosition += sToFirstDirection * sDeltaLambda * sSecondBoneCoefficient * sSecondInverseMass;
	}
}


void FHGMConstraintLibrary::MakeHorizontalBendStructure(FHGMSimulationPlane& SimulationPlane, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDStructure>& OutHorizontalBendStructures)
{
	FScopedTranspose ScopedTranspose(&SimulationPlane);
	ScopedTranspose.Add(&Positions);
	ScopedTranspose.Execute();

	FHGMConstraintLibrary::MakeVerticalBendStructure(SimulationPlane, Positions, OutHorizontalBendStructures);
}


void FHGMConstraintLibrary::HorizontalBendConstraint(TArrayView<FHGMSIMDStructure> HorizontalBendStructures, FHGMPhysicsContext& PhysicsContext, FHGMSimulationPlane& SimulationPlane, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDReal>& InverseMasses, TArray<FHGMSIMDReal>& FixedBlends, TArray<FHGMSIMDReal>& DummyBoneMasks)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintHorizontalBendConstraint);

	FScopedTranspose ScopedTranspose(&SimulationPlane);
	ScopedTranspose.Add(&Positions);
	ScopedTranspose.Add(&InverseMasses);
	ScopedTranspose.Add(&FixedBlends);
	ScopedTranspose.Add(&DummyBoneMasks);
	ScopedTranspose.Execute();

	FHGMConstraintLibrary::VerticalBendConstraint(HorizontalBendStructures, PhysicsContext, Positions, InverseMasses, FixedBlends, DummyBoneMasks);
}


void FHGMConstraintLibrary::RelativeLimitAngleConstraint(const FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDStructure> VerticalStructures, TConstArrayView<FHGMSIMDRelativeLimitAngle> RelativeLimitAngles, TConstArrayView<FHGMSIMDVector3> AnimPosePositions, TArray<FHGMSIMDVector3>& Positions, TConstArrayView<FHGMSIMDReal> DummyBoneMasks)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintRelativeLimitAngleConstraint);

	static const FHGMSIMDReal sAngleEpsilon = FHGMSIMDLibrary::LoadConstant(0.1);

	// Parent of root bone is not subject to simulation, so it is angle constrained compared to animation pose.
	for (int32 FirstBoneIndex = 0; FirstBoneIndex < SimulationPlane.PackedHorizontalBoneNum; ++FirstBoneIndex)
	{
		const FHGMSIMDStructure& sFirstVerticalStructure =  VerticalStructures[FirstBoneIndex];
		FHGMSIMDVector3& sFirstBonePosition = Positions[sFirstVerticalStructure.FirstBonePackedIndex];
		FHGMSIMDVector3& sSecondBonePosition = Positions[sFirstVerticalStructure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sCurrentDirection = FHGMMathLibrary::MakeSafeNormal(sSecondBonePosition - sFirstBonePosition);

		const FHGMSIMDVector3& sFirstAnimPosition = AnimPosePositions[sFirstVerticalStructure.FirstBonePackedIndex];
		const FHGMSIMDVector3& sSecondAnimPosition = AnimPosePositions[sFirstVerticalStructure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sAnimDirection = FHGMMathLibrary::MakeSafeNormal(sSecondAnimPosition - sFirstAnimPosition);

		// Calculate overhang angle.
		const FHGMSIMDVector3 sRotationAxis = FHGMMathLibrary::CrossProduct(sAnimDirection, sCurrentDirection);
		const FHGMSIMDReal sAngle = FHGMMathLibrary::Atan2(FHGMMathLibrary::Length(sRotationAxis), FHGMMathLibrary::DotProduct(sAnimDirection, sCurrentDirection));

		const FHGMSIMDRelativeLimitAngle& sRelativeLimitAngleConstraint = RelativeLimitAngles[sFirstVerticalStructure.FirstBonePackedIndex];
		const FHGMSIMDReal sAngleOverLimit = ((sAngle * HGMSIMDConstants::RadiansToDegrees) - sRelativeLimitAngleConstraint.sAngle) * (HGMSIMDConstants::OneReal - sRelativeLimitAngleConstraint.sDamping);

		// Rotate backward by angle of the overhang to apply constraint.
		const FHGMSIMDVector3 sConstrainedDirection = FHGMMathLibrary::RotateAngleAxis(sCurrentDirection, -sAngleOverLimit, FHGMMathLibrary::MakeSafeNormal(sRotationAxis));
		const FHGMSIMDVector3 sConstrainedSecondBonePosition = sConstrainedDirection * FHGMMathLibrary::Length(sSecondBonePosition - sFirstBonePosition) + sFirstBonePosition;
		const FHGMSIMDReal sConstraintMask = sAngleOverLimit > sAngleEpsilon;
		sSecondBonePosition = FHGMSIMDLibrary::Select(sConstraintMask, sConstrainedSecondBonePosition, sSecondBonePosition);
	}

	for (int32 ChildBoneIndex = SimulationPlane.PackedHorizontalBoneNum; ChildBoneIndex < VerticalStructures.Num(); ++ChildBoneIndex)
	{
		const FHGMSIMDStructure& sParentVerticalStructure =  VerticalStructures[ChildBoneIndex - SimulationPlane.PackedHorizontalBoneNum];
		const FHGMSIMDVector3& sParentBoneStartPosition = Positions[sParentVerticalStructure.FirstBonePackedIndex];
		const FHGMSIMDVector3& sParentBoneEndPosition = Positions[sParentVerticalStructure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sParentBoneDirection = FHGMMathLibrary::MakeSafeNormal(sParentBoneEndPosition - sParentBoneStartPosition);

		const FHGMSIMDStructure& sChildVerticalStructure =  VerticalStructures[ChildBoneIndex];
		const FHGMSIMDVector3& sChildBoneStartPosition = Positions[sChildVerticalStructure.FirstBonePackedIndex];
		const FHGMSIMDVector3& sChildBoneEndPosition = Positions[sChildVerticalStructure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sChildBoneDirection = FHGMMathLibrary::MakeSafeNormal(sChildBoneEndPosition - sChildBoneStartPosition);

		// Calculate overhang angle.
		const FHGMSIMDVector3 sRotationAxis = FHGMMathLibrary::CrossProduct(sParentBoneDirection, sChildBoneDirection);
		const FHGMSIMDReal sAngle = FHGMMathLibrary::Atan2(FHGMMathLibrary::Length(sRotationAxis), FHGMMathLibrary::DotProduct(sParentBoneDirection, sChildBoneDirection));

		const FHGMSIMDRelativeLimitAngle& sRelativeLimitAngleConstraint = RelativeLimitAngles[sChildVerticalStructure.FirstBonePackedIndex];
		const FHGMSIMDReal sAngleOverLimit = ((sAngle * HGMSIMDConstants::RadiansToDegrees) - sRelativeLimitAngleConstraint.sAngle) * (HGMSIMDConstants::OneReal - sRelativeLimitAngleConstraint.sDamping);;

		// Rotate backward by angle of the overhang to apply constraint.
		const FHGMSIMDVector3 sConstrainedChildDirection = FHGMMathLibrary::MakeSafeNormal(FHGMMathLibrary::RotateAngleAxis(sChildBoneDirection, -sAngleOverLimit, FHGMMathLibrary::MakeSafeNormal(sRotationAxis)));
		const FHGMSIMDVector3 sConstrainedChildBoneEndPosition = sConstrainedChildDirection * FHGMMathLibrary::Length(sChildBoneEndPosition - sChildBoneStartPosition) + Positions[sChildVerticalStructure.FirstBonePackedIndex];

		const FHGMSIMDReal sConstraintMask = sAngleOverLimit > sAngleEpsilon;
		Positions[sChildVerticalStructure.SecondBonePackedIndex] = FHGMSIMDLibrary::Select(sConstraintMask, sConstrainedChildBoneEndPosition, sChildBoneEndPosition);
	}
}


void FHGMConstraintLibrary::AnimPoseMovableRadiusConstraint(TConstArrayView<FHGMSIMDAnimPoseConstraintMovableRadius> MovableRadiusConstraints, TConstArrayView<FHGMSIMDVector3> AnimPosePositions, TArrayView<FHGMSIMDVector3> Positions)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintAnimPoseMovableRadiusConstraint);

	static const FHGMSIMDReal sRadiusEpsilon = FHGMSIMDLibrary::LoadConstant(0.1);

	for (int32 PackedIndex = 0; PackedIndex < Positions.Num(); ++PackedIndex)
	{
		FHGMSIMDVector3& Position = Positions[PackedIndex];
		const FHGMSIMDVector3& AnimPosePosition = AnimPosePositions[PackedIndex];
		const FHGMSIMDVector3 sToAnimPose = AnimPosePosition - Position;
		const FHGMSIMDVector3 sToAnimPoseDirection = FHGMMathLibrary::MakeSafeNormal(sToAnimPose);
		const FHGMSIMDAnimPoseConstraintMovableRadius& sAnimPoseConstraintMovableRadius = MovableRadiusConstraints[PackedIndex];
		FHGMSIMDReal sDifference = (FHGMMathLibrary::Length(sToAnimPose) - sAnimPoseConstraintMovableRadius.sRadius) * (HGMSIMDConstants::OneReal - sAnimPoseConstraintMovableRadius.sDamping);
		sDifference = FHGMSIMDLibrary::Select(sDifference > sRadiusEpsilon, sDifference, HGMSIMDConstants::ZeroReal);

		const FHGMSIMDVector3 sConstraint = sDifference * sToAnimPoseDirection;
		Position += sConstraint;
	}
}


void FHGMConstraintLibrary::AnimPoseLimitAngleConstraint(TConstArrayView<FHGMSIMDStructure> VerticalStructures, TConstArrayView<FHGMSIMDAnimPoseConstraintLimitAngle> LimitAngles, TConstArrayView<FHGMSIMDVector3> AnimPosePositions, TArrayView<FHGMSIMDVector3> Positions)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintAnimPoseLimitAngleConstraint);

	static const FHGMSIMDReal sAngleEpsilon = FHGMSIMDLibrary::LoadConstant(0.1);

	for (const FHGMSIMDStructure& Structure : VerticalStructures)
	{
		FHGMSIMDVector3& sFirstBonePosition = Positions[Structure.FirstBonePackedIndex];
		FHGMSIMDVector3& sSecondBonePosition = Positions[Structure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sCurrentDirection = FHGMMathLibrary::MakeSafeNormal(sSecondBonePosition - sFirstBonePosition);

		const FHGMSIMDVector3& sFirstAnimPosition = AnimPosePositions[Structure.FirstBonePackedIndex];
		const FHGMSIMDVector3& sSecondAnimPosition = AnimPosePositions[Structure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sAnimDirection = FHGMMathLibrary::MakeSafeNormal(sSecondAnimPosition - sFirstAnimPosition);

		// Calculate overhang angle.
		const FHGMSIMDVector3 sRotationAxis = FHGMMathLibrary::CrossProduct(sAnimDirection, sCurrentDirection);
		const FHGMSIMDReal sAngle = FHGMMathLibrary::Atan2(FHGMMathLibrary::Length(sRotationAxis), FHGMMathLibrary::DotProduct(sAnimDirection, sCurrentDirection));
		const FHGMSIMDAnimPoseConstraintLimitAngle& sAnimPoseConstraintLimitAngle = LimitAngles[Structure.FirstBonePackedIndex];
		const FHGMSIMDReal sAngleOverLimit = ((sAngle * HGMSIMDConstants::RadiansToDegrees) - sAnimPoseConstraintLimitAngle.sAngle) * (HGMSIMDConstants::OneReal - sAnimPoseConstraintLimitAngle.sDamping);

		// Rotate backward by angle of the overhang to apply constraint.
		const FHGMSIMDVector3 sConstrainedDirection = FHGMMathLibrary::RotateAngleAxis(sCurrentDirection, -sAngleOverLimit, FHGMMathLibrary::MakeSafeNormal(sRotationAxis));
		const FHGMSIMDVector3 sConstrainedSecondBonePosition = sConstrainedDirection * FHGMMathLibrary::Length(sSecondBonePosition - sFirstBonePosition) + sFirstBonePosition;
		const FHGMSIMDReal sConstraintMask = sAngleOverLimit > sAngleEpsilon;
		sSecondBonePosition = FHGMSIMDLibrary::Select(sConstraintMask, sConstrainedSecondBonePosition, sSecondBonePosition);
	}
}


void FHGMConstraintLibrary::AnimPosePlanarConstraint(TConstArrayView<FHGMSIMDInt> PlanarConstraintAxes, TConstArrayView<FHGMSIMDStructure> VerticalStructures, const FHGMSimulationPlane& SimulationPlane, FComponentSpacePoseContext& Output, TArrayView<FBoneReference> Bones, TConstArrayView<FHGMSIMDVector3> AnimPosePositions, TArrayView<FHGMSIMDVector3> Positions)
{
	SCOPE_CYCLE_COUNTER(STAT_ConstraintAnimPosePlanarConstraint);

	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	for (int32 StructureIndex = 0; StructureIndex < VerticalStructures.Num(); ++StructureIndex)
	{
		const FHGMSIMDStructure& VerticalStructure = VerticalStructures[StructureIndex];

		// Gets position and orientation of bone in animation pose.
		TStaticArray<int32, 4> BoneUnpackedIndexes {};
		FHGMSIMDLibrary::Store(VerticalStructure.sSecondBoneUnpackedIndex, BoneUnpackedIndexes);

		TStaticArray<FHGMQuaternion, 4> BoneRotations {};
		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			const FBoneReference& Bone = Bones[BoneUnpackedIndexes[ComponentIndex]];
			if (!FHGMAnimationLibrary::IsValidBone(Bone))
			{
				BoneRotations[ComponentIndex] = FHGMQuaternion::Identity;
				continue;
			}

			const FCompactPoseBoneIndex CompactPoseBoneIndex = Bone.GetCompactPoseIndex(BoneContainer);
			const FHGMTransform& BoneTransform = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneIndex);
			BoneRotations[ComponentIndex] = BoneTransform.GetRotation();
		}

		const int32 PackedVerticalChainIndex = StructureIndex % SimulationPlane.PackedHorizontalBoneNum;
		const FHGMSIMDInt& sPlanarConstraintAxis = PlanarConstraintAxes[PackedVerticalChainIndex];

		// Gets normal of plane used for constraint.
		FHGMSIMDQuaternion sBoneRotation {};
		FHGMSIMDLibrary::Load(sBoneRotation, BoneRotations);
		FHGMSIMDVector3 sBoneDirection = FHGMMathLibrary::GetUnitAxis(sBoneRotation, sPlanarConstraintAxis);

		// Origin of plane.
		const FHGMSIMDVector3& sBoneLocation = AnimPosePositions[VerticalStructure.SecondBonePackedIndex];

		// Coordinates of constraint target. This will be tip of bone.
		FHGMSIMDVector3& sTargetPosition = Positions[VerticalStructure.SecondBonePackedIndex];

		const FHGMSIMDVector3 sConstraintedLocation = FHGMMathLibrary::PointPlaneProject(sTargetPosition, sBoneLocation, sBoneDirection);

		// Constrains only if axis is specified.
		sTargetPosition = FHGMSIMDLibrary::Select(sPlanarConstraintAxis > HGMSIMDConstants::ZeroInt, sConstraintedLocation, sTargetPosition);
	}
}
