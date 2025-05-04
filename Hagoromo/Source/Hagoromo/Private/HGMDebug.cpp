// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HGMDebug.h"
#include "HagoromoModule.h"
#include "AnimNode_Hagoromo.h"
#include "HGMSolvers.h"
#include "HGMAnimation.h"


FString FHGMDebugLibrary::ToString(const FHGMSIMDReal& A)
{
	TStaticArray<FHGMReal, 4> Components {};
	FHGMSIMDLibrary::Store(A, Components);

#if HGM_USE_FLOAT32
	return FString::Printf(TEXT("X= %f Y= %f Z= %f W= %f"), Components[0], Components[1], Components[2], Components[3]);
#else
	return FString::Printf(TEXT("X= %lf Y= %lf Z= %lf W= %lf"), Components[0], Components[1], Components[2], Components[3]);
#endif
}


FString FHGMDebugLibrary::ToString(const FHGMSIMDVector3& V)
{
	FString Result { TEXT("\n" "[SIMDRegister View]" "\n") };
	Result += TEXT("R0: ") + FHGMDebugLibrary::ToString(V.X) + TEXT("\n");
	Result += TEXT("R1: ") + FHGMDebugLibrary::ToString(V.Y) + TEXT("\n");
	Result += TEXT("R2: ") + FHGMDebugLibrary::ToString(V.Z) + TEXT("\n");
	Result += TEXT("\n");

	Result += TEXT("[Vector3 View]" "\n");
	FHGMVector3 V0, V1, V2, V3 {};
	FHGMSIMDLibrary::Store(V, V0, V1, V2, V3);
	Result += TEXT("V0: ") + V0.ToString() + TEXT("\n");
	Result += TEXT("V1: ") + V1.ToString() + TEXT("\n");
	Result += TEXT("V2: ") + V2.ToString() + TEXT("\n");
	Result += TEXT("V3: ") + V3.ToString();

	return Result;
}


#if ENABLE_ANIM_DRAW_DEBUG
void FHGMDebugLibrary::DrawSphere(FComponentSpacePoseContext& PoseContext, const FHGMSIMDVector3& sCenter, const FHGMSIMDReal& sRadius, int32 Segments, FColor Color, ESceneDepthPriorityGroup DepthPriority, FHGMReal Thickness)
{
	TStaticArray<FHGMVector3, 4> UnpackedCenters {};
	FHGMSIMDLibrary::Store(sCenter, UnpackedCenters);

	TStaticArray<FHGMReal, 4> UnpackedRadiuses {};
	FHGMSIMDLibrary::Store(sRadius, UnpackedRadiuses);

	for (int32 ComponentIndex = 0; ComponentIndex < UnpackedCenters.Num(); ++ComponentIndex)
	{
		PoseContext.AnimInstanceProxy->AnimDrawDebugSphere(UnpackedCenters[ComponentIndex], UnpackedRadiuses[ComponentIndex], Segments, Color, false, -1.0f, Thickness, DepthPriority);
	}
}


void FHGMDebugLibrary::DrawBodyCollider(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsContext& PhysicsContext, const FHGMBodyCollider& BodyCollider, ESceneDepthPriorityGroup DepthPriority)
{

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();

	for (const FHGMSphereCollider& SphereCollider : BodyCollider.SphereColliders)
	{
		FHGMVector3 Center = SphereCollider.Center;
		FHGMVector3 WorldSpaceCenter = SkeletalMeshComponentTransform.TransformPosition(Center);
		PoseContext.AnimInstanceProxy->AnimDrawDebugSphere(WorldSpaceCenter, SphereCollider.Radius, 16,
			FColor(162, 242, 204, 255), false, -1.0f, 0.0f, DepthPriority);
	}

	for (const FHGMCapsuleCollider& CapsuleCollider : BodyCollider.CapsuleColliders)
	{
		FHGMVector3 StartPoint = CapsuleCollider.StartPoint;
		FHGMVector3 EndPoint = CapsuleCollider.EndPoint;
		const FHGMVector3 WorldSpaceStartPoint = SkeletalMeshComponentTransform.TransformPosition(StartPoint);
		const FHGMVector3 WorldSpaceEndPoint = SkeletalMeshComponentTransform.TransformPosition(EndPoint);
		const FHGMVector3 WorldSpaceCenter = FHGMMathLibrary::Lerp(WorldSpaceStartPoint, WorldSpaceEndPoint, 0.5);
		const FHGMVector3 Segment = WorldSpaceStartPoint - WorldSpaceEndPoint;
		const FHGMReal Length = FHGMMathLibrary::Length(Segment);
		FHGMQuaternion Rotation = FHGMQuaternion::FindBetweenVectors(FHGMVector3::UpVector, Segment);

		PoseContext.AnimInstanceProxy->AnimDrawDebugCircle(WorldSpaceStartPoint, CapsuleCollider.Radius, 36,
			FColor(162, 242, 204, 255), Rotation.GetForwardVector(), false, -1.0f, DepthPriority);

		PoseContext.AnimInstanceProxy->AnimDrawDebugCircle(WorldSpaceStartPoint, CapsuleCollider.Radius, 36,
			FColor(162, 242, 204, 255), Rotation.GetRightVector(), false, -1.0f, DepthPriority);

		PoseContext.AnimInstanceProxy->AnimDrawDebugCircle(WorldSpaceEndPoint, CapsuleCollider.Radius, 36,
			FColor(162, 242, 204, 255), Rotation.GetForwardVector(), false, -1.0f, DepthPriority);

		PoseContext.AnimInstanceProxy->AnimDrawDebugCircle(WorldSpaceEndPoint, CapsuleCollider.Radius, 36,
			FColor(162, 242, 204, 255), Rotation.GetRightVector(), false, -1.0f, DepthPriority);

		FHGMVector3 EdgeStart = WorldSpaceStartPoint + Rotation.GetForwardVector() * CapsuleCollider.Radius;
		FHGMVector3 EdgeEnd = WorldSpaceEndPoint + Rotation.GetForwardVector() * CapsuleCollider.Radius;
		PoseContext.AnimInstanceProxy->AnimDrawDebugLine(EdgeStart, EdgeEnd, FColor(162, 242, 204, 255), false, -1.0f, 0.0f, DepthPriority);

		EdgeStart = WorldSpaceStartPoint - Rotation.GetForwardVector() * CapsuleCollider.Radius;
		EdgeEnd = WorldSpaceEndPoint - Rotation.GetForwardVector() * CapsuleCollider.Radius;
		PoseContext.AnimInstanceProxy->AnimDrawDebugLine(EdgeStart, EdgeEnd, FColor(162, 242, 204, 255), false, -1.0f, 0.0f, DepthPriority);

		EdgeStart = WorldSpaceStartPoint + Rotation.GetRightVector() * CapsuleCollider.Radius;
		EdgeEnd = WorldSpaceEndPoint + Rotation.GetRightVector() * CapsuleCollider.Radius;
		PoseContext.AnimInstanceProxy->AnimDrawDebugLine(EdgeStart, EdgeEnd, FColor(162, 242, 204, 255), false, -1.0f, 0.0f, DepthPriority);

		EdgeStart = WorldSpaceStartPoint - Rotation.GetRightVector() * CapsuleCollider.Radius;
		EdgeEnd = WorldSpaceEndPoint - Rotation.GetRightVector() * CapsuleCollider.Radius;
		PoseContext.AnimInstanceProxy->AnimDrawDebugLine(EdgeStart, EdgeEnd, FColor(162, 242, 204, 255), false, -1.0f, 0.0f, DepthPriority);
	}
}


void FHGMDebugLibrary::DrawBoneColliders(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	if (!Solver)
	{
		return;
	}

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	for (int32 PackedIndex = 0; PackedIndex < Solver->Positions.Num(); ++PackedIndex)
	{
		TStaticArray<FHGMVector3, 4> UnpackedComponentPositions {};
		FHGMSIMDLibrary::Store(Solver->Positions[PackedIndex], UnpackedComponentPositions);

		const FHGMSIMDVector3 sWorldPosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, Solver->Positions[PackedIndex]);
		TStaticArray<FHGMVector3, 4> UnpackedWorldPositions {};
		FHGMSIMDLibrary::Store(sWorldPosition, UnpackedWorldPositions);

		TStaticArray<FHGMReal, 4> UnpackedRadiuses {};
		FHGMSIMDLibrary::Store(Solver->BoneSphereColliderRadiuses[PackedIndex], UnpackedRadiuses);

		const FHGMSIMDReal& sDummyBoneMask = Solver->DummyBoneMasks[PackedIndex];
		TStaticArray<FHGMReal, 4> UnpackedDummyBoneMasks {};
		FHGMSIMDLibrary::Store(sDummyBoneMask, UnpackedDummyBoneMasks);

		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			if (UnpackedDummyBoneMasks[ComponentIndex] > 0.0f)
			{
				continue;
			}

			PoseContext.AnimInstanceProxy->AnimDrawDebugSphere(UnpackedWorldPositions[ComponentIndex], UnpackedRadiuses[ComponentIndex], 16,
				FColor(255, 94, 183, 255), false, -1.0f, 0.0f, DepthPriority);

			PoseContext.AnimInstanceProxy->AnimDrawDebugInWorldMessage(FString::SanitizeFloat(UnpackedRadiuses[ComponentIndex]), UnpackedComponentPositions[ComponentIndex], FColor::Black, 1.0f);
		}
	}
}


void FHGMDebugLibrary::DrawPlaneColliders(FComponentSpacePoseContext& PoseContext, TConstArrayView <FHGMPlaneCollider> PlaneColliders)
{
	const FBoneContainer& BoneContainer =  PoseContext.Pose.GetPose().GetBoneContainer();

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();

	for (const FHGMPlaneCollider& PlaneCollider : PlaneColliders)
	{
		FHGMTransform BoneRefTransform = FHGMTransform::Identity;
		FHGMTransform PlaneTransform(PlaneCollider.RotationOffset, PlaneCollider.LocationOffset, FHGMVector3::OneVector);
		if (PlaneCollider.DrivingBone.IsValidToEvaluate())
		{
			const FCompactPoseBoneIndex CompactPoseIndex = PlaneCollider.DrivingBone.GetCompactPoseIndex(BoneContainer);
			const FHGMTransform& BoneTransform =  PoseContext.Pose.GetComponentSpaceTransform(CompactPoseIndex);
			PlaneTransform *= BoneTransform;

			BoneRefTransform = FHGMAnimationLibrary::GetComponentSpaceRefTransform(BoneContainer, CompactPoseIndex);
		}

		PlaneTransform *= SkeletalMeshComponentTransform;
		PoseContext.AnimInstanceProxy->AnimDrawDebugPlane(PlaneTransform, 50.0f, FColor(255, 243, 156, 255), false, -1.0f, 1.2f, ESceneDepthPriorityGroup::SDPG_Foreground);

		PoseContext.AnimInstanceProxy->AnimDrawDebugCoordinateSystem(PlaneTransform.GetTranslation(), BoneRefTransform.GetRotation().Rotator(), 12.0f, false, -1.0f, 1.2f, ESceneDepthPriorityGroup::SDPG_Foreground);

		PoseContext.AnimInstanceProxy->AnimDrawDebugDirectionalArrow(PlaneTransform.GetTranslation(), PlaneTransform.GetTranslation() + PlaneTransform.GetRotation().GetUpVector() * 24.0f, 12.0f, FColor::Black, false, -1.0f, 1.2f, ESceneDepthPriorityGroup::SDPG_Foreground);
	}
}


void FHGMDebugLibrary::DrawFixedBlends(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	if (!Solver)
	{
		return;
	}

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	for (int32 PackedIndex = 0; PackedIndex < Solver->Positions.Num(); ++PackedIndex)
	{
		const FHGMSIMDReal& sFixedBlend = Solver->FixedBlends[PackedIndex];
		TStaticArray<FHGMReal, 4> UnpackedFixedBlends {};
		FHGMSIMDLibrary::Store(sFixedBlend, UnpackedFixedBlends);

		const FHGMSIMDVector3& sPosition = Solver->Positions[PackedIndex];
		const FHGMSIMDVector3 sWorldPosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sPosition);
		TStaticArray<FHGMVector3, 4> UnpackedWorldPositions {};
		FHGMSIMDLibrary::Store(sWorldPosition, UnpackedWorldPositions);

		const FHGMSIMDReal& sDummyBoneMask = Solver->DummyBoneMasks[PackedIndex];
		TStaticArray<FHGMReal, 4> UnpackedDummyBoneMasks {};
		FHGMSIMDLibrary::Store(sDummyBoneMask, UnpackedDummyBoneMasks);

		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			if (UnpackedDummyBoneMasks[ComponentIndex] > 0.0f)
			{
				continue;
			}

			const FLinearColor Color = FMath::Lerp(FLinearColor::White, FLinearColor::Black, UnpackedFixedBlends[ComponentIndex]);
			PoseContext.AnimInstanceProxy->AnimDrawDebugSphere(UnpackedWorldPositions[ComponentIndex], 2.0f, 4, Color.QuantizeRound(), false, -1.0f, 0.0f, DepthPriority);
		}
	}
}


void FHGMDebugLibrary::DrawVerticalStructure(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	if (!Solver)
	{
		return;
	}

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	for (FHGMSIMDStructure& Structure : Solver->VerticalStructures)
	{
		const FHGMSIMDVector3& sFirstBonePosition = Solver->Positions[Structure.FirstBonePackedIndex];
		const FHGMSIMDVector3 sWorldFirstBonePosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sFirstBonePosition);
		TStaticArray<FHGMVector3, 4> UnpackedWorldFirstBonePositions {};
		FHGMSIMDLibrary::Store(sWorldFirstBonePosition, UnpackedWorldFirstBonePositions);

		const FHGMSIMDVector3& sSecondBonePosition = Solver->Positions[Structure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sWorldSecondBonePosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sSecondBonePosition);
		TStaticArray<FHGMVector3, 4> UnpackedWorldSecondBonePositions {};
		FHGMSIMDLibrary::Store(sWorldSecondBonePosition, UnpackedWorldSecondBonePositions);

		const FHGMSIMDReal& sFirstBoneDummyMask = Solver->DummyBoneMasks[Structure.FirstBonePackedIndex];
		TStaticArray<FHGMReal, 4> UnpackedFirstBoneDummyMasks {};
		FHGMSIMDLibrary::Store(sFirstBoneDummyMask, UnpackedFirstBoneDummyMasks);

		const FHGMSIMDReal& sSecondBoneDummyMask = Solver->DummyBoneMasks[Structure.SecondBonePackedIndex];
		TStaticArray<FHGMReal, 4> UnpackedSecondBoneDummyMasks {};
		FHGMSIMDLibrary::Store(sSecondBoneDummyMask, UnpackedSecondBoneDummyMasks);

		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			if (UnpackedFirstBoneDummyMasks[ComponentIndex] > 0.0 || UnpackedSecondBoneDummyMasks[ComponentIndex] > 0.0)
			{
				continue;
			}

			PoseContext.AnimInstanceProxy->AnimDrawDebugLine(UnpackedWorldFirstBonePositions[ComponentIndex], UnpackedWorldSecondBonePositions[ComponentIndex], FColor(184, 145, 255, 255), false, -1.0f, 0.5f, DepthPriority);
		}
	}
}


void FHGMDebugLibrary::DrawHorizontalStructure(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	FScopedTranspose ScopedTranspose(&Solver->SimulationPlane);
	ScopedTranspose.Add(&Solver->Positions);
	ScopedTranspose.Add(&Solver->DummyBoneMasks);
	ScopedTranspose.Execute();

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	for (FHGMSIMDStructure& Structure : Solver->HorizontalStructures)
	{
		const FHGMSIMDVector3& sFirstBonePosition = Solver->Positions[Structure.FirstBonePackedIndex];
		const FHGMSIMDVector3 sWorldFirstBonePosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sFirstBonePosition);

		TStaticArray<FHGMVector3, 4> UnpackedWorldFirstBonePositions {};
		FHGMSIMDLibrary::Store(sWorldFirstBonePosition, UnpackedWorldFirstBonePositions);

		const FHGMSIMDVector3& sSecondBonePosition = Solver->Positions[Structure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sWorldSecondBonePosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sSecondBonePosition);
		TStaticArray<FHGMVector3, 4> UnpackedWorldSecondBonePositions {};
		FHGMSIMDLibrary::Store(sWorldSecondBonePosition, UnpackedWorldSecondBonePositions);

		const FHGMSIMDReal& sFirstBoneDummyMask = Solver->DummyBoneMasks[Structure.FirstBonePackedIndex];
		TStaticArray<FHGMReal, 4> UnpackedFirstBoneDummyMasks {};
		FHGMSIMDLibrary::Store(sFirstBoneDummyMask, UnpackedFirstBoneDummyMasks);

		const FHGMSIMDReal& sSecondBoneDummyMask = Solver->DummyBoneMasks[Structure.SecondBonePackedIndex];
		TStaticArray<FHGMReal, 4> UnpackedSecondBoneDummyMasks {};
		FHGMSIMDLibrary::Store(sSecondBoneDummyMask, UnpackedSecondBoneDummyMasks);

		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			if (UnpackedFirstBoneDummyMasks[ComponentIndex] > 0.0 || UnpackedSecondBoneDummyMasks[ComponentIndex] > 0.0)
			{
				continue;
			}

			PoseContext.AnimInstanceProxy->AnimDrawDebugLine(UnpackedWorldFirstBonePositions[ComponentIndex], UnpackedWorldSecondBonePositions[ComponentIndex], FColor(184, 145, 255, 255), false, -1.0f, 0.5f, DepthPriority);
		}
	}
}


void FHGMDebugLibrary::DrawShear(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, bool bLoopHorizontalStructure, ESceneDepthPriorityGroup DepthPriority)
{
	if (!Solver)
	{
		return;
	}

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	static TStaticArray<FColor, 2> Colors {};
	Colors[0] = FColor(117, 253, 255, 255);
	Colors[1] = FColor(255, 186, 102, 255);

	for (int32 ShearIndexBase = 0; ShearIndexBase < Solver->ShearStructures.Num(); ShearIndexBase += 2)
	{
		const int32 HorizontalChainIndex = ShearIndexBase / 2;
		bool bIsEndHorizontalBone = (HorizontalChainIndex % Solver->SimulationPlane.ActualPackedHorizontalBoneNum) + 1 == Solver->SimulationPlane.ActualPackedHorizontalBoneNum;

		int32 EndComponentIndex = 3;
		if (bIsEndHorizontalBone)
		{
			EndComponentIndex = (Solver->SimulationPlane.ActualUnpackedHorizontalBoneNum - 1) % 4;
		}

		for (int32 Offset = 0; Offset < 2; ++Offset)
		{
			const int32 ShearIndex = ShearIndexBase + Offset;
			const FHGMSIMDShearStructure& sShear = Solver->ShearStructures[ShearIndex];

			FHGMSIMDVector3 sFirstBonePosition {};
			FHGMSIMDLibrary::Store(Solver->Positions, sShear.sFirstBoneUnpackedIndex, sFirstBonePosition);
			const FHGMSIMDVector3 sWorldFirstBonePosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sFirstBonePosition);

			TStaticArray<FHGMVector3, 4> FirstBonePositions {};
			FHGMSIMDLibrary::Store(sWorldFirstBonePosition, FirstBonePositions);

			FHGMSIMDVector3 sSecondBonePositions {};
			FHGMSIMDLibrary::Store(Solver->Positions, sShear.sSecondBoneUnpackedIndex, sSecondBonePositions);
			const FHGMSIMDVector3 sWorldSecondBonePosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sSecondBonePositions);

			TStaticArray<FHGMVector3, 4> SecondBonePositions {};
			FHGMSIMDLibrary::Store(sWorldSecondBonePosition, SecondBonePositions);

			TStaticArray<FHGMReal, 4> DummyFirstBoneMasks {};
			FHGMSIMDLibrary::Store(Solver->DummyBoneMasks, sShear.sFirstBoneUnpackedIndex, DummyFirstBoneMasks);

			TStaticArray<FHGMReal, 4> DummySecondBoneMasks {};
			FHGMSIMDLibrary::Store(Solver->DummyBoneMasks, sShear.sSecondBoneUnpackedIndex, DummySecondBoneMasks);

			for (int32 ComponentIndex = 0; ComponentIndex <= EndComponentIndex; ++ComponentIndex)
			{
				if (DummyFirstBoneMasks[ComponentIndex] > 0.0 || DummySecondBoneMasks[ComponentIndex] > 0.0)
				{
					continue;
				}

				PoseContext.AnimInstanceProxy->AnimDrawDebugLine(FirstBonePositions[ComponentIndex], SecondBonePositions[ComponentIndex], Colors[Offset], false, -1.0f, 0.5f, DepthPriority);
			}
		}
	}
}


void FHGMDebugLibrary::DrawAnimPoseMovableRadiusConstraint(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsSettings& PhysicsSettings, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	if (!Solver)
	{
		return;
	}

	if (!PhysicsSettings.bUseAnimPoseConstraint || !PhysicsSettings.bUseAnimPoseConstraintMovableRadius)
	{
		return;
	}

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	for (int32 PackedIndex = 0; PackedIndex < Solver->Positions.Num(); ++PackedIndex)
	{
		const FHGMSIMDReal& sDummyBoneMask = Solver->DummyBoneMasks[PackedIndex];

		const FHGMSIMDVector3& sAnimPosition = Solver->AnimPosePositions[PackedIndex];
		const FHGMSIMDVector3 sWorldAnimPosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sAnimPosition);
		const FHGMSIMDReal& sMovableRadius = Solver->AnimPoseConstraintMovableRadiuses[PackedIndex].sRadius;
		FHGMDebugLibrary::DrawSphere(PoseContext, sWorldAnimPosition, sMovableRadius * (HGMSIMDConstants::OneReal - sDummyBoneMask), 12, FColor(255, 196, 102, 255), DepthPriority);

		const FHGMSIMDVector3& sPosition = Solver->Positions[PackedIndex];
		const FHGMSIMDVector3 sWorldPosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sPosition);
		FHGMDebugLibrary::DrawSphere(PoseContext, sWorldPosition, HGMSIMDConstants::TwoReal * (HGMSIMDConstants::OneReal - sDummyBoneMask), 4, FColor::Black, ESceneDepthPriorityGroup::SDPG_Foreground);
	}
}


void FHGMDebugLibrary::DrawAnimPoseLimitAngleConstraint(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsSettings& PhysicsSettings, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	if (!Solver)
	{
		return;
	}

	if (!PhysicsSettings.bUseAnimPoseConstraint || !PhysicsSettings.bUseAnimPoseConstraintLimitAngle)
	{
		return;
	}

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	for (const FHGMSIMDStructure& VerticalStructure : Solver->VerticalStructures)
	{
		const FHGMSIMDVector3& sFirstBonePosition = Solver->AnimPosePositions[VerticalStructure.FirstBonePackedIndex];
		const FHGMSIMDVector3 sWorldFirstBonePosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sFirstBonePosition);
		TStaticArray<FHGMVector3, 4> UnpackedWorldFirstBonePositions {};
		FHGMSIMDLibrary::Store(sWorldFirstBonePosition, UnpackedWorldFirstBonePositions);

		const FHGMSIMDVector3& sSecondBonePosition = Solver->AnimPosePositions[VerticalStructure.SecondBonePackedIndex];
		const FHGMSIMDVector3 sWorldSecondBonePosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, sSecondBonePosition);
		TStaticArray<FHGMVector3, 4> UnpackedWorldSecondBonePositions {};
		FHGMSIMDLibrary::Store(sWorldSecondBonePosition, UnpackedWorldSecondBonePositions);

		TStaticArray<FHGMReal, 4> UnpackedFirstBoneDummyMasks {};
		FHGMSIMDLibrary::Store(Solver->DummyBoneMasks[VerticalStructure.FirstBonePackedIndex], UnpackedFirstBoneDummyMasks);

		TStaticArray<FHGMReal, 4> UnpackedSecondBoneDummyMasks {};
		FHGMSIMDLibrary::Store(Solver->DummyBoneMasks[VerticalStructure.SecondBonePackedIndex], UnpackedSecondBoneDummyMasks);

		TStaticArray<FHGMReal, 4> UnpackedLimitAngles {};
		FHGMSIMDLibrary::Store(Solver->AnimPoseConstraintLimitAngles[VerticalStructure.FirstBonePackedIndex].sAngle, UnpackedLimitAngles);

		TStaticArray<FHGMReal, 4> UnpackedLength {};
		FHGMSIMDLibrary::Store(VerticalStructure.sLength, UnpackedLength);

		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			if (UnpackedFirstBoneDummyMasks[ComponentIndex] > 0.0 || UnpackedSecondBoneDummyMasks[ComponentIndex] > 0.0)
			{
				continue;
			}

			const FHGMVector3 Direction = FHGMMathLibrary::MakeSafeNormal(UnpackedWorldSecondBonePositions[ComponentIndex] - UnpackedWorldFirstBonePositions[ComponentIndex]);
			const FHGMReal& Angle = FHGMMathLibrary::DegreesToRadians(UnpackedLimitAngles[ComponentIndex]);
			PoseContext.AnimInstanceProxy->AnimDrawDebugCone(UnpackedWorldFirstBonePositions[ComponentIndex], UnpackedLength[ComponentIndex], Direction, Angle, Angle, 12, FColor::Blue, false, -1.0f, DepthPriority, 0.0f);
		}
	}
}


void FHGMDebugLibrary::DrawAnimPosePlanarConstraint(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsSettings& PhysicsSettings, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	static TArray<FColor> PlaneColors { FColor::Red, FColor::Green, FColor::Blue };

	if (!Solver)
	{
		return;
	}

	if (!PhysicsSettings.bUseAnimPoseConstraint || !PhysicsSettings.bUseAnimPoseConstraintPlanar)
	{
		return;
	}

	const FBoneContainer& BoneContainer = PoseContext.Pose.GetPose().GetBoneContainer();

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	for (int32 PackedIndex = 0; PackedIndex < Solver->AnimPosePlanarConstraintAxes.Num(); ++PackedIndex)
	{
		TStaticArray<int32, 4> UnpackedAxes {};
		FHGMSIMDLibrary::Store(Solver->AnimPosePlanarConstraintAxes[PackedIndex], UnpackedAxes);

		const FHGMSIMDVector3 sWorldPosition = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, Solver->Positions[PackedIndex]);
		TStaticArray<FHGMVector3, 4> UnpackedWorldPositions {};
		FHGMSIMDLibrary::Store(sWorldPosition, UnpackedWorldPositions);

		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			if (UnpackedAxes[ComponentIndex] <= 0)
			{
				continue;
			}

			FHGMTransform BoneTransform = FHGMTransform::Identity;

			const FBoneReference& Bone = Solver->Bones[(PackedIndex * 4) + ComponentIndex];
			if (Bone.IsValidToEvaluate())
			{
				const FCompactPoseBoneIndex BoneIndex = Bone.GetCompactPoseIndex(BoneContainer);
				BoneTransform = PoseContext.Pose.GetComponentSpaceTransform(BoneIndex);
			}

			const EAxis::Type PlaneNormal = StaticCast<EAxis::Type>(UnpackedAxes[ComponentIndex]);
			const FHGMQuaternion PlaneRotation=  FHGMQuaternion::FindBetweenVectors(FHGMVector3::UpVector, BoneTransform.GetUnitAxis(PlaneNormal));
			const FHGMVector3& PlaneTranslation = UnpackedWorldPositions[ComponentIndex];
			FHGMTransform PlaneTransform(PlaneRotation, PlaneTranslation, FHGMVector3::OneVector);

			PoseContext.AnimInstanceProxy->AnimDrawDebugPlane(PlaneTransform, 2.0f, PlaneColors[UnpackedAxes[ComponentIndex] - 1], false, -1.0f, 0.25f, DepthPriority);
			PoseContext.AnimInstanceProxy->AnimDrawDebugDirectionalArrow(PlaneTranslation, PlaneTranslation + PlaneRotation.GetUpVector() * 5.0f, 2.0f, PlaneColors[UnpackedAxes[ComponentIndex] - 1], false, -1.0f, 0.25f, DepthPriority);
		}
	}
}


void FHGMDebugLibrary::DrawRelativeLimitAngleConstraint(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsSettings& PhysicsSettings, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	if (!Solver)
	{
		return;
	}

	if (!PhysicsSettings.bUseRelativeLimitAngleConstraint)
	{
		return;
	}

	const FHGMTransform& SkeletalMeshComponentTransform = PoseContext.AnimInstanceProxy->GetComponentTransform();
	FHGMSIMDTransform sSkeletalMeshComponentTransform {};
	FHGMSIMDLibrary::Load(sSkeletalMeshComponentTransform, SkeletalMeshComponentTransform);

	for (int32 StructureIndex = 0; StructureIndex < Solver->VerticalStructures.Num(); ++StructureIndex)
	{
		const int32 HorizontalIndex = StructureIndex % Solver->SimulationPlane.PackedHorizontalBoneNum;

		const FHGMSIMDStructure& sVerticalStructure = Solver->VerticalStructures[StructureIndex];
		const FHGMSIMDReal& sAngle = Solver->RelativeLimitAngles[StructureIndex].sAngle;

		const FHGMSIMDVector3 sStart = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, Solver->Positions[sVerticalStructure.FirstBonePackedIndex]);
		const FHGMSIMDVector3 sEnd = FHGMMathLibrary::TransformPosition(sSkeletalMeshComponentTransform, Solver->Positions[sVerticalStructure.SecondBonePackedIndex]);
		FHGMSIMDVector3 sDirection = FHGMMathLibrary::MakeSafeNormal(sEnd - sStart);
		sDirection = FHGMMathLibrary::TransformVector(sSkeletalMeshComponentTransform, sDirection);

		TStaticArray<FHGMVector3, 4> UnpackedStartArray {};
		FHGMSIMDLibrary::Store(sStart, UnpackedStartArray);

		TStaticArray<FHGMVector3, 4> UnpackedEndArray {};
		FHGMSIMDLibrary::Store(sEnd, UnpackedEndArray);

		TStaticArray<FHGMVector3, 4> UnpackedDirectionArray {};
		FHGMSIMDLibrary::Store(sDirection, UnpackedDirectionArray);

		TStaticArray<FHGMReal, 4> UnpackedLengthArray {};
		FHGMSIMDLibrary::Store(sVerticalStructure.sLength, UnpackedLengthArray);

		TStaticArray<FHGMReal, 4> UnpackedAngles {};
		FHGMSIMDLibrary::Store(sAngle, UnpackedAngles);

		TStaticArray<FHGMReal, 4> UnpackedFirstDummyMasks {};
		FHGMSIMDLibrary::Store(Solver->DummyBoneMasks[sVerticalStructure.FirstBonePackedIndex], UnpackedFirstDummyMasks);

		TStaticArray<FHGMReal, 4> UnpackedSecondDummyMasks {};
		FHGMSIMDLibrary::Store(Solver->DummyBoneMasks[sVerticalStructure.SecondBonePackedIndex], UnpackedSecondDummyMasks);

		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			if (UnpackedFirstDummyMasks[ComponentIndex] > 0.0f || UnpackedSecondDummyMasks[ComponentIndex] > 0.0f)
			{
				continue;
			}

			const FHGMReal AngleRad = FHGMMathLibrary::DegreesToRadians(UnpackedAngles[ComponentIndex]);
			PoseContext.AnimInstanceProxy->AnimDrawDebugLine(UnpackedStartArray[ComponentIndex], UnpackedEndArray[ComponentIndex], FColor::White, false, -1.0f, 0.5f, DepthPriority);
			PoseContext.AnimInstanceProxy->AnimDrawDebugCone(UnpackedStartArray[ComponentIndex], UnpackedLengthArray[ComponentIndex], UnpackedDirectionArray[ComponentIndex], AngleRad, AngleRad, 12, FColor::Blue, false, -1.0f, DepthPriority, 0.0f);
		}
	}
}


void FHGMDebugLibrary::DrawVelocities(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsContext& PhysicsContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority)
{
	if (!Solver)
	{
		return;
	}

	FHGMReal MaxVelocity = 0.0;
	for (int32 PackedIndex = 0; PackedIndex < Solver->Positions.Num(); ++PackedIndex)
	{
		const FHGMSIMDVector3 sVelocity = Solver->Positions[PackedIndex] - Solver->PrevPositions[PackedIndex];
		const FHGMSIMDReal sVelocitySize = FHGMMathLibrary::Length(sVelocity);
		TStaticArray<FHGMReal, 4> UnpackedVelocitySizeArray {};
		FHGMSIMDLibrary::Store(sVelocitySize, UnpackedVelocitySizeArray);

		const FHGMSIMDReal& sDummyBoneMask = Solver->DummyBoneMasks[PackedIndex];
		TStaticArray<FHGMReal, 4> UnpackedDummyBoneMasks {};
		FHGMSIMDLibrary::Store(sDummyBoneMask, UnpackedDummyBoneMasks);

		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			if (UnpackedDummyBoneMasks[ComponentIndex] > 0.0)
			{
				continue;
			}

			MaxVelocity = FHGMMathLibrary::Max(MaxVelocity, UnpackedVelocitySizeArray[ComponentIndex]);
		}
	}

	const FString MaxVelocityString = FString::Printf(TEXT("MaxVelocity: [%f]"), MaxVelocity);
	PoseContext.AnimInstanceProxy->AnimDrawDebugOnScreenMessage(MaxVelocityString, FColor::Black, FVector2D::UnitVector, ESceneDepthPriorityGroup::SDPG_Foreground);

	const FHGMVector3 WorldVelocity = PhysicsContext.SkeletalMeshComponentTransform.InverseTransformPosition(PhysicsContext.PrevSkeletalMeshComponentTransform.GetTranslation());
	const FString WorldVelocityString = FString::Printf(TEXT("WorldVelocity: [%f]"), FHGMMathLibrary::Length(WorldVelocity));
	PoseContext.AnimInstanceProxy->AnimDrawDebugOnScreenMessage(WorldVelocityString, FColor::Black, FVector2D::UnitVector, ESceneDepthPriorityGroup::SDPG_Foreground);

	FHGMQuaternion RotationDifference = PhysicsContext.SkeletalMeshComponentTransform.InverseTransformRotation(PhysicsContext.PrevSkeletalMeshComponentTransform.GetRotation());
	const FHGMReal WorldAngularVelocity = FHGMMathLibrary::RadiansToDegrees(RotationDifference.GetAngle());
	const FString WorldAngularVelocityString = FString::Printf(TEXT("WorldAngularVelocity: [%f]"), WorldAngularVelocity);
	PoseContext.AnimInstanceProxy->AnimDrawDebugOnScreenMessage(WorldAngularVelocityString, FColor::Black, FVector2D::UnitVector, ESceneDepthPriorityGroup::SDPG_Foreground);

	if (PhysicsContext.PhysicsSettings.bUseSimulationRootBone)
	{
		FHGMVector3 SimulationVelocity = PhysicsContext.SimulationRootBoneTransform.InverseTransformPosition(PhysicsContext.PrevSimulationRootBoneTransform.GetTranslation());
		const FString SimulationVelocityString = FString::Printf(TEXT("SimulationVelocity: [%f]"), FHGMMathLibrary::Length(SimulationVelocity));
		PoseContext.AnimInstanceProxy->AnimDrawDebugOnScreenMessage(SimulationVelocityString, FColor::Black, FVector2D::UnitVector, ESceneDepthPriorityGroup::SDPG_Foreground);

		FHGMQuaternion SimulationRotationDifference = PhysicsContext.SimulationRootBoneTransform.InverseTransformRotation(PhysicsContext.PrevSimulationRootBoneTransform.GetRotation());
		const FHGMReal SimulationAngularVelocity = FHGMMathLibrary::RadiansToDegrees(SimulationRotationDifference.GetAngle());
		const FString SimulationAngularVelocityString = FString::Printf(TEXT("SimulationAngularVelocity: [%f]"), SimulationAngularVelocity);
		PoseContext.AnimInstanceProxy->AnimDrawDebugOnScreenMessage(SimulationAngularVelocityString, FColor::Black, FVector2D::UnitVector, ESceneDepthPriorityGroup::SDPG_Foreground);
	}
}
#endif
