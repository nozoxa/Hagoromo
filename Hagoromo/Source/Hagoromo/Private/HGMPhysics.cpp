// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HGMPhysics.h"
#include "HagoromoModule.h"
#include "Collision.h"

#include "Math/Axis.h"

DECLARE_CYCLE_STAT(TEXT("Physics ApplyForces"), STAT_PhysicsApplyForces, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Physics VerletIntegrate"), STAT_PhysicsVerletIntegrate, STATGROUP_Hagoromo);


void FHGMPhysicsLibrary::ApplyForces(FComponentSpacePoseContext& Output, const FHGMPhysicsContext& PhysicsContext, TArrayView<FHGMSIMDVector3> Positions, TArrayView<FHGMSIMDVector3> PrevPositions,
								TConstArrayView<FHGMSIMDReal> WorldVelocityDampings, TConstArrayView<FHGMSIMDReal> WorldAngularVelocityDampings, TConstArrayView<FHGMSIMDReal> SimulationVelocityDampings, TConstArrayView<FHGMSIMDReal> SimulationAngularVelocityDampings, TConstArrayView<FHGMSIMDReal> MasterDampings,
								TConstArrayView<FHGMSIMDReal> Frictions, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks)
{
	SCOPE_CYCLE_COUNTER(STAT_PhysicsApplyForces);

	// World Movement :
	FHGMVector3 WorldVelocity = PhysicsContext.SkeletalMeshComponentTransform.InverseTransformPosition(PhysicsContext.PrevSkeletalMeshComponentTransform.GetTranslation());
	if (FHGMMathLibrary::LengthSquared(WorldVelocity) > (PhysicsContext.PhysicsSettings.IgnoreWorldVelocityThreshold * PhysicsContext.PhysicsSettings.IgnoreWorldVelocityThreshold))
	{
		WorldVelocity = FHGMVector3::ZeroVector;
	}
	FHGMSIMDVector3 sWorldVelocity {};
	FHGMSIMDLibrary::Load(sWorldVelocity, WorldVelocity);

	// World Rotation :
	FHGMQuaternion RotationDifference = PhysicsContext.SkeletalMeshComponentTransform.InverseTransformRotation(PhysicsContext.PrevSkeletalMeshComponentTransform.GetRotation());
	RotationDifference.Normalize();
	if (FHGMMathLibrary::RadiansToDegrees(RotationDifference.GetAngle()) > PhysicsContext.PhysicsSettings.IgnoreWorldAngularVelocityThreshold)
	{
		RotationDifference = FHGMQuaternion::Identity;
	}

	FHGMSIMDQuaternion sWorldAngularVelocity;
	FHGMSIMDLibrary::Load(sWorldAngularVelocity, RotationDifference);

	FHGMSIMDVector3 sSimulationVelocity = FHGMSIMDVector3::ZeroVector;
	FHGMSIMDQuaternion sSimulationAngularVelocity = FHGMSIMDQuaternion::Identity;
	if (PhysicsContext.PhysicsSettings.bUseSimulationRootBone)
	{
		// SimulationRootBone Movement :
		FHGMVector3 SimulationVelocity = PhysicsContext.SimulationRootBoneTransform.InverseTransformPosition(PhysicsContext.PrevSimulationRootBoneTransform.GetTranslation());
		if (FHGMMathLibrary::LengthSquared(SimulationVelocity) > (PhysicsContext.PhysicsSettings.IgnoreSimulationVelocityThreshold * PhysicsContext.PhysicsSettings.IgnoreSimulationVelocityThreshold))
		{
			SimulationVelocity = FHGMVector3::ZeroVector;
		}
		FHGMSIMDLibrary::Load(sSimulationVelocity, SimulationVelocity);

		// SimulationRootBone Rotation :
		FHGMQuaternion SimulationRotationDifference = PhysicsContext.SimulationRootBoneTransform.InverseTransformRotation(PhysicsContext.PrevSimulationRootBoneTransform.GetRotation());
		if (FHGMMathLibrary::RadiansToDegrees(SimulationRotationDifference.GetAngle()) > PhysicsContext.PhysicsSettings.IgnoreSimulationAngularVelocityThreshold)
		{
			SimulationRotationDifference = FHGMQuaternion::Identity;
		}

		FHGMSIMDLibrary::Load(sSimulationAngularVelocity, SimulationRotationDifference);
	}

	// Gravity :
	static FHGMReal MeterToCentimeter = 100.0;
	const FHGMGravitySettings& GravitySettings = PhysicsContext.PhysicsSettings.GravitySettings;
	FHGMSIMDVector3 sGravity = FHGMSIMDVector3::ZeroVector;
	if (GravitySettings.bUseBoneSpaceGravity && GravitySettings.DrivingBone.IsValidToEvaluate())
	{
		const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
		const FCompactPoseBoneIndex DrivingBoneIndex = GravitySettings.DrivingBone.GetCompactPoseIndex(BoneContainer);
		const FHGMTransform& DrivingBoneTransform = Output.Pose.GetComponentSpaceTransform(DrivingBoneIndex);
		const FHGMVector3 Gravity = DrivingBoneTransform.GetUnitAxis(GravitySettings.Axis) * GravitySettings.Gravity * MeterToCentimeter;
		FHGMSIMDLibrary::Load(sGravity, Gravity);
	}
	else
	{
		const FHGMVector3 Gravity = PhysicsContext.SkeletalMeshComponentTransform.InverseTransformVector(-(FHGMVector3::UpVector * GravitySettings.Gravity * MeterToCentimeter));
		FHGMSIMDLibrary::Load(sGravity, Gravity);
	}

	// Apply force.
	// #OPTIMIZE
	// Considering cache hit rate, it may be better to separate loops by type of force.
	const FHGMSIMDReal sDeltaTimeChangeFactor = PhysicsContext.sDeltaTime / PhysicsContext.sPrevDeltaTime;
	for (int32 PackedIndex = 0; PackedIndex < Positions.Num(); ++PackedIndex)
	{
		const FHGMSIMDReal sFriction = HGMSIMDConstants::OneReal - Frictions[PackedIndex];

		// Calculate world movement.
		const FHGMSIMDReal sWorldVelocityInfluence = FHGMMathLibrary::Lerp(HGMSIMDConstants::OneReal, HGMSIMDConstants::ZeroReal, WorldVelocityDampings[PackedIndex]);
		const FHGMSIMDVector3 sAdjustedWorldVelocity = sWorldVelocityInfluence * sWorldVelocity;

		// Calculate world rotation.
		FHGMSIMDVector3 sLinearizedWorldAngularVelocity {};
		sLinearizedWorldAngularVelocity = FHGMMathLibrary::RotateVector(sWorldAngularVelocity, PrevPositions[PackedIndex]) - PrevPositions[PackedIndex];
		const FHGMSIMDReal sWorldAngularVelocityDamping = FHGMMathLibrary::Lerp(HGMSIMDConstants::OneReal, HGMSIMDConstants::ZeroReal, WorldAngularVelocityDampings[PackedIndex]);
		FHGMSIMDVector3 sDampedWorldAngularVelocity = sLinearizedWorldAngularVelocity * sWorldAngularVelocityDamping;

		FHGMSIMDVector3 sAdjustedSimulationVelocity = FHGMSIMDVector3::ZeroVector;
		FHGMSIMDVector3 sDampedSimulationAngularVelocity = FHGMSIMDVector3::ZeroVector;
		if (PhysicsContext.PhysicsSettings.bUseSimulationRootBone)
		{
			// Calculate simulation movement.
			const FHGMSIMDReal sSimulationVelocityDamping = FHGMMathLibrary::Lerp(HGMSIMDConstants::OneReal, HGMSIMDConstants::ZeroReal, SimulationVelocityDampings[PackedIndex]);
			sAdjustedSimulationVelocity = sSimulationVelocity * sSimulationVelocityDamping;

			// Calculate simulation rotation.
			FHGMSIMDVector3 sLinearizedSimulationAngularVelocity {};
			sLinearizedSimulationAngularVelocity = FHGMMathLibrary::RotateVector(sSimulationAngularVelocity, PrevPositions[PackedIndex]) - PrevPositions[PackedIndex];
			const FHGMSIMDReal sSimulationAngularVelocityDamping = FHGMMathLibrary::Lerp(HGMSIMDConstants::OneReal, HGMSIMDConstants::ZeroReal, SimulationAngularVelocityDampings[PackedIndex]);
			sDampedSimulationAngularVelocity = sLinearizedSimulationAngularVelocity * sSimulationAngularVelocityDamping;
		}

		// Add gravity.
		Positions[PackedIndex] += (sGravity * PhysicsContext.sDeltaTime * PhysicsContext.sDeltaTime) * sDeltaTimeChangeFactor * (HGMSIMDConstants::OneReal - DummyBoneMasks[PackedIndex]) * (HGMSIMDConstants::OneReal - FixedBlends[PackedIndex]);

		// Add velocities.
		Positions[PackedIndex] += (sAdjustedWorldVelocity + sDampedWorldAngularVelocity + sAdjustedSimulationVelocity + sDampedSimulationAngularVelocity) * sDeltaTimeChangeFactor * sFriction * (HGMSIMDConstants::OneReal - DummyBoneMasks[PackedIndex]) * (HGMSIMDConstants::OneReal - FixedBlends[PackedIndex]);
	}
}


void FHGMPhysicsLibrary::VerletIntegrate(const FHGMPhysicsContext& PhysicsContext, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDVector3>& PrevPositions, TConstArrayView<FHGMSIMDReal> Frictions, TConstArrayView<FHGMSIMDReal> MasterDampings, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks)
{
	SCOPE_CYCLE_COUNTER(STAT_PhysicsVerletIntegrate);

	// sDeltaTimeChangeFactor was adopted from 「 https://en.wikipedia.org/wiki/Verlet_integration > Non-constant time differences 」.
	const FHGMSIMDReal sDeltaTimeChangeFactor = PhysicsContext.sDeltaTime / PhysicsContext.sPrevDeltaTime;
	for (int32 PackedIndex = 0; PackedIndex < Positions.Num(); ++PackedIndex)
	{
		const FHGMSIMDReal sFriction = HGMSIMDConstants::OneReal - Frictions[PackedIndex];
		const FHGMSIMDReal sMasterDamping = (HGMSIMDConstants::OneReal - MasterDampings[PackedIndex]);

		const FHGMSIMDVector3 sCopiedPosition = Positions[PackedIndex];
		const FHGMSIMDVector3 sVelocity = (sCopiedPosition - PrevPositions[PackedIndex]) * sDeltaTimeChangeFactor;
		const FHGMSIMDVector3 sNextPosition = sCopiedPosition + (sVelocity * sFriction * sMasterDamping) * (HGMSIMDConstants::OneReal - FixedBlends[PackedIndex]) * (HGMSIMDConstants::OneReal - DummyBoneMasks[PackedIndex]);

		Positions[PackedIndex] = sNextPosition;
		PrevPositions[PackedIndex] = sCopiedPosition;
	}
}


void FHGMPhysicsLibrary::ResetFriction(TArray<FHGMSIMDReal>& Frictions)
{
	FMemory::Memzero(Frictions.GetData(), Frictions.GetAllocatedSize());
}


void FHGMPhysicsLibrary::CalculateFriction(TConstArrayView<FHGMSIMDReal> Frictions, TConstArrayView<FHGMSIMDColliderContact> ColliderContacts, TArray<FHGMSIMDReal>& OutFrictions)
{
	if (OutFrictions.Num() != Frictions.Num())
	{
		HGM_LOG(Error, TEXT("Friction is not working. There is error in calculation of number of elements in friction array."));
		return;
	}

	for (const FHGMSIMDColliderContact& ColliderContact : ColliderContacts)
	{
		OutFrictions[ColliderContact.PackedIndex] = FHGMSIMDLibrary::Select(ColliderContact.sHitMask, Frictions[ColliderContact.PackedIndex], HGMSIMDConstants::ZeroReal);
	}
}
