// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"
#include "HGMCollision.h"

struct FHGMPhysicsContext;
struct FHGMSimulationPlane;
struct FHGMSIMDRelativeLimitAngle;
struct FHGMSIMDAnimPoseConstraintMovableRadius;
struct FHGMSIMDAnimPoseConstraintLimitAngle;
struct FComponentSpacePoseContext;


struct FHGMSIMDStructure
{
	int32 FirstBonePackedIndex = -1;
	int32 SecondBonePackedIndex = -1;
	FHGMSIMDInt sFirstBoneUnpackedIndex = HGMSIMDConstants::MinusOneInt;
	FHGMSIMDInt sSecondBoneUnpackedIndex = HGMSIMDConstants::MinusOneInt;
	FHGMSIMDReal sLength = HGMSIMDConstants::OneReal;
	FHGMSIMDReal sLambda = HGMSIMDConstants::ZeroReal;
};


struct FHGMSIMDShearStructure
{
	FHGMSIMDInt sFirstBoneUnpackedIndex = HGMSIMDConstants::MinusOneInt;
	FHGMSIMDInt sSecondBoneUnpackedIndex = HGMSIMDConstants::MinusOneInt;
	FHGMSIMDReal sLength = HGMSIMDConstants::OneReal;
	FHGMSIMDReal sLambda = HGMSIMDConstants::ZeroReal;
};


struct FHGMConstraintLibrary
{
	static void FixedBlendConstraint(TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDVector3>& PrevPositions, const TArray<FHGMSIMDVector3>& AnimPositions, const TArray<FHGMSIMDReal>& FixedBlends);

	static void ColliderContactConstraint(const TArray<FHGMSIMDColliderContact>& Contacts, const FHGMSIMDReal& sCollisionBlend, const FHGMSIMDReal& sColliderPenetrationDepth, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks);

	// Zero clear lambda used by XPBD.
	template<typename T>
	FORCEINLINE static void ResetLambda(TArrayView<T> StructureDataArray)
	{
		for (T& StructureData : StructureDataArray)
		{
			StructureData.sLambda = HGMSIMDConstants::ZeroReal;
		}
	}

	static void MakeVerticalStructure(const FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDVector3> Positions, TArray<FHGMSIMDStructure>& OutVerticalStructures);
	static void VerticalStructuralConstraint(TArrayView<FHGMSIMDStructure> Structures, FHGMPhysicsContext& PhysicsContext, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> InverseMasses, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks);

	static void RigidVerticalStructuralConstraint(TArrayView<FHGMSIMDStructure> Structures, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> FixedBlends);

	static void MakeHorizontalStructure(FHGMSimulationPlane& SimulationPlane, TArray<FHGMSIMDVector3>& Positions, bool bLoopHorizontalStructure, TArray<FHGMSIMDStructure>& OutHorizontalStructures);
	static void HorizontalStructuralConstraint(TArrayView<FHGMSIMDStructure> Structures, FHGMPhysicsContext& PhysicsContext, FHGMSimulationPlane& SimulationPlane, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDReal>& InverseMasses, TArray<FHGMSIMDReal>& FixedBlends, TArray<FHGMSIMDReal>& DummyBoneMasks);

	static void MakeShearStructure(const FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDVector3> Positions, bool bLoopHorizontalStructure, TArray<FHGMSIMDShearStructure>& OutShearStructures);
	static void ShearConstraint(TArrayView<FHGMSIMDShearStructure> Shears, FHGMPhysicsContext& PhysicsContext, const FHGMSimulationPlane& SimulationPlane, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> InverseMasses, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks);

	static void MakeVerticalBendStructure(const FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDVector3> Positions, TArray<FHGMSIMDStructure>& OutVerticalBendStructures);
	static void VerticalBendConstraint(TArrayView<FHGMSIMDStructure> BendStructures, FHGMPhysicsContext& PhysicsContext, TArrayView<FHGMSIMDVector3> Positions, TConstArrayView<FHGMSIMDReal> InverseMasses, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks);

	static void MakeHorizontalBendStructure(FHGMSimulationPlane& SimulationPlane, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDStructure>& OutHorizontalBendStructures);
	static void HorizontalBendConstraint(TArrayView<FHGMSIMDStructure> HorizontalBendStructures, FHGMPhysicsContext& PhysicsContext, FHGMSimulationPlane& SimulationPlane, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDReal>& InverseMasses, TArray<FHGMSIMDReal>& FixedBlends, TArray<FHGMSIMDReal>& DummyBoneMasks);

	static void RelativeLimitAngleConstraint(const FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDStructure> VerticalStructures, TConstArrayView<FHGMSIMDRelativeLimitAngle> RelativeLimitAngles, TConstArrayView<FHGMSIMDVector3> AnimPosePositions, TArray<FHGMSIMDVector3>& Positions, TConstArrayView<FHGMSIMDReal> DummyBoneMasks);

	static void AnimPoseMovableRadiusConstraint(TConstArrayView<FHGMSIMDAnimPoseConstraintMovableRadius> MovableRadiuses, TConstArrayView<FHGMSIMDVector3> AnimPosePositions, TArrayView<FHGMSIMDVector3> Positions);
	static void AnimPoseLimitAngleConstraint(TConstArrayView<FHGMSIMDStructure> VerticalStructures, TConstArrayView<FHGMSIMDAnimPoseConstraintLimitAngle> LimitAngles, TConstArrayView<FHGMSIMDVector3> AnimPosePositions, TArrayView<FHGMSIMDVector3> Positions);
	static void AnimPosePlanarConstraint(TConstArrayView<FHGMSIMDInt> PlanarConstraintAxes, TConstArrayView<FHGMSIMDStructure> VerticalStructures, const FHGMSimulationPlane& SimulationPlane, FComponentSpacePoseContext& Output, TArrayView<FBoneReference> Bones, TConstArrayView<FHGMSIMDVector3> AnimPosePositions, TArrayView<FHGMSIMDVector3> Positions);
};
