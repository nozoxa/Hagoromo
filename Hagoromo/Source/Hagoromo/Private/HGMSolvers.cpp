// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HGMSolvers.h"
#include "HagoromoModule.h"
#include "HGMMath.h"
#include "HGMAnimation.h"
#include "HGMPhysics.h"
#include "HGMConstraints.h"

#include "Engine/Engine.h"

DECLARE_CYCLE_STAT(TEXT("Solver Initialize"), STAT_SolverInitialize, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Solver PreSimulate"), STAT_SolverPreSimulate, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Solver Simulate"), STAT_SolverSimulate, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Solver OutputSimulateResult"), STAT_SolverOutputSimulateResult, STATGROUP_Hagoromo);

// Specify an internal linkage as unnamed space may not work depending on unity build.
namespace SolverInternal
{
	// Referring FReferenceSkeleton::GetDirectChildBones().
	static int32 GatherDirectChildBoneIndexes(const FReferenceSkeleton& RefSkeleton, int32 ParentBoneIndex, TArray<int32>& ChildBones)
	{
		ChildBones.Reset();

		const int32 BoneMaxNum = RefSkeleton.GetNum();
		for (int32 ChildBoneIndex = ParentBoneIndex + 1; ChildBoneIndex < BoneMaxNum; ChildBoneIndex++)
		{
			if (ParentBoneIndex == RefSkeleton.GetParentIndex(ChildBoneIndex))
			{
				ChildBones.Add(ChildBoneIndex);
			}
		}

		return ChildBones.Num();
	}


	static bool GatherChain(const FBoneContainer& BoneContainer, const FReferenceSkeleton& RefSkeleton, TConstArrayView<FBoneReference> ExcludeBones, int32 BoneIndex,
							TArray<FBoneReference>& Bones, TArray<FHGMVector3>& BonePositions)
	{
		if (BoneIndex < 0 || RefSkeleton.GetNum() < BoneIndex)
		{
			return false;
		}

		FBoneReference Bone {};
		Bone.BoneName = RefSkeleton.GetBoneName(BoneIndex);
		Bone.Initialize(BoneContainer);

		// ExcludeBones and subsequent bones will be excluded from simulation and collection will be suspended.
		if (ExcludeBones.Num() > 0 && ExcludeBones.Find(Bone) != INDEX_NONE)
		{
			return true;
		}

		if (Bone.CachedCompactPoseIndex == INDEX_NONE)
		{
			return false;
		}

		Bones.Emplace(Bone);

		const FHGMTransform& BoneTransform = FHGMAnimationLibrary::GetComponentSpaceRefTransform(BoneContainer, Bone.CachedCompactPoseIndex);
		BonePositions.Emplace(BoneTransform.GetTranslation());

		TArray<int32> ChildBoneIndexes {};
		SolverInternal::GatherDirectChildBoneIndexes(RefSkeleton, BoneIndex, ChildBoneIndexes);
		for (int32 ChildBoneIndex : ChildBoneIndexes)
		{
			SolverInternal::GatherChain(BoneContainer, RefSkeleton, ExcludeBones, ChildBoneIndex, Bones, BonePositions);
		}

		return true;
	}


	static void AddDummyBone(TArray<FHGMVector3>& ChainPositions, TArray<FBoneReference>& ChainBones, TArray<FHGMReal>& UnpackedDummyChainBoneMask, int32 DummyChainBoneNum)
	{
		const FHGMVector3& EndLinkFirst = ChainPositions.Last(1);
		const FHGMVector3& EndLinkSecond = ChainPositions.Last(0);
		const FHGMVector3 FirstToSecond = EndLinkSecond - EndLinkFirst;

		for (int32 DummyChainBoneCount = 0; DummyChainBoneCount < DummyChainBoneNum; ++DummyChainBoneCount)
		{
			const FHGMVector3& EndPosition = ChainPositions.Last();
			ChainPositions.Emplace(EndPosition + FirstToSecond);
			ChainBones.Emplace(HGMAnimationConstants::DummyBone);
			UnpackedDummyChainBoneMask.Emplace(1.0);
		}
	}


	static void AddDummyChain(TArray<FHGMChainSetting>& ChainSettings, TArray<TArray<FHGMVector3>>& UnpackedChainPositions, TArray<TArray<FBoneReference>>& UnpackedChainBones, TArray<TArray<FHGMReal>>& UnpackedDummyChainBoneMasks, int32 DummyChainNum)
	{
		const int32 ReservingNum = ChainSettings.Num() + DummyChainNum;
		UnpackedChainPositions.Reserve(ReservingNum);
		ChainSettings.Reserve(ReservingNum);
		UnpackedChainBones.Reserve(ReservingNum);
		UnpackedDummyChainBoneMasks.Reserve(ReservingNum);

		TArray<FBoneReference> DummyChainBones {};
		DummyChainBones.Init(HGMAnimationConstants::DummyBone, UnpackedChainBones[0].Num());

		const FHGMChainSetting DummyChainSetting { HGMAnimationConstants::DummyBone };

		const TArray<FHGMVector3>& DummyChainBase = UnpackedChainPositions.Last(0);
		TArray<FHGMReal> DummyChainBoneMask {};
		DummyChainBoneMask.Init(1.0, DummyChainBase.Num());

		const FHGMReal Offset = 5.0;
		for (int32 DummyChainCount = 0; DummyChainCount < DummyChainNum; ++DummyChainCount)
		{
			ChainSettings.Emplace(DummyChainSetting);

			TArray<FHGMVector3> DummyChain = DummyChainBase;
			for (FHGMVector3& Position : DummyChain)
			{
				// Coordinates are shifted to prevent division by zero when calculating cloth normal.
				// Offset can be anything non-zero since it is dummy.
				Position += FHGMVector3::RightVector * Offset * StaticCast<float>((DummyChainCount + 1));
			}

			UnpackedChainPositions.Emplace(DummyChain);
			UnpackedChainBones.Emplace(DummyChainBones);
			UnpackedDummyChainBoneMasks.Emplace(DummyChainBoneMask);
		}
	}


	// XPBD Compliance :
	// See https://blog.mmacklin.com/2016/10/12/xpbd-slides-and-stiffness/ .
	FHGMReal ConcreteCompliance = 25000000000.0;
	FHGMReal FatCompliance = 1000.0;

	static FHGMReal ConvertStiffnessToCompliance(FHGMReal Stiffness)
	{
		const FHGMReal Compliance = FHGMMathLibrary::Lerp<FHGMReal>(FatCompliance, ConcreteCompliance, Stiffness);
		return 1.0 / Compliance;
	}

	static void ConvertStiffnessesToCompliances(const FHGMPhysicsSettings PhysicsSettings,FHGMPhysicsContext& PhysicsContext)
	{
		PhysicsContext.PhysicsSettings.StructureStiffness = SolverInternal::ConvertStiffnessToCompliance(StaticCast<FHGMReal>(PhysicsSettings.StructureStiffness));
		PhysicsContext.PhysicsSettings.VerticalBendStiffness = SolverInternal::ConvertStiffnessToCompliance(StaticCast<FHGMReal>(PhysicsSettings.VerticalBendStiffness));
		PhysicsContext.PhysicsSettings.HorizontalBendStiffness = SolverInternal::ConvertStiffnessToCompliance(StaticCast<FHGMReal>(PhysicsSettings.HorizontalBendStiffness));
		PhysicsContext.PhysicsSettings.ShearStiffness = SolverInternal::ConvertStiffnessToCompliance(StaticCast<FHGMReal>(PhysicsSettings.ShearStiffness));
	}


	static void UpdateDeltaTime(FComponentSpacePoseContext& Output, FHGMPhysicsContext& PhysicsContext)
	{
		PhysicsContext.sPrevDeltaTime = PhysicsContext.sDeltaTime;
		const FHGMReal CurrentDeltaTime = Output.AnimInstanceProxy->GetDeltaSeconds();
		const FHGMReal FixedDeltaTime = 1.0 / GEngine->FixedFrameRate;
		const FHGMReal AdjustedDeltaTime = CurrentDeltaTime <= 0.0 ? FixedDeltaTime : CurrentDeltaTime;
		FHGMSIMDLibrary::Load(PhysicsContext.sDeltaTime, AdjustedDeltaTime);
		FHGMSIMDLibrary::Load(PhysicsContext.sDeltaTimeExponent, HGMGlobal::TargetFrameRate * AdjustedDeltaTime);

		// Specify same values that physics do not get out of control first time it is run.
		if (PhysicsContext.bIsFirstUpdate)
		{
			FHGMSIMDLibrary::Load(PhysicsContext.sPrevDeltaTime, AdjustedDeltaTime);
		}

		// Hitch countermeasures.
		const FHGMSIMDReal sDeltaTimeChangeFactor = PhysicsContext.sDeltaTime / PhysicsContext.sPrevDeltaTime;
		TStaticArray<FHGMReal, 4> DeltaTimeChangeFactors {};
		FHGMSIMDLibrary::Store(sDeltaTimeChangeFactor, DeltaTimeChangeFactors);
		if (DeltaTimeChangeFactors[0] >= 1.2)
		{
			PhysicsContext.sDeltaTime = PhysicsContext.sPrevDeltaTime;
		}
	}


	static void UpdateSkeletalMeshComponentTransform(FComponentSpacePoseContext& Output, FHGMPhysicsContext& PhysicsContext)
	{
		PhysicsContext.PrevSkeletalMeshComponentTransform = PhysicsContext.SkeletalMeshComponentTransform;
		PhysicsContext.SkeletalMeshComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();

		if (PhysicsContext.bIsFirstUpdate)
		{
			PhysicsContext.PrevSkeletalMeshComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();
			PhysicsContext.SkeletalMeshComponentTransform = PhysicsContext.PrevSkeletalMeshComponentTransform;
		}
	}


	static void UpdateSimulationRootBoneTransform(FComponentSpacePoseContext& Output, FHGMPhysicsContext& PhysicsContext)
	{
		if (!PhysicsContext.PhysicsSettings.SimulationRootBone.IsValidToEvaluate())
		{
			HGM_LOG(Warning, TEXT("SimulationRootBone was invalid."));
			return;
		}

		const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
		const FCompactPoseBoneIndex SimulationRootBoneIndex = PhysicsContext.PhysicsSettings.SimulationRootBone.GetCompactPoseIndex(BoneContainer);
		const FHGMTransform& SimulationRootBoneTransform = Output.Pose.GetComponentSpaceTransform(SimulationRootBoneIndex);

		// Specify same transform so that physics do not get out of control first time it is run.
		if (PhysicsContext.bIsFirstUpdate)
		{
			PhysicsContext.PrevSimulationRootBoneTransform = SimulationRootBoneTransform;
			PhysicsContext.SimulationRootBoneTransform = SimulationRootBoneTransform;
		}
		else
		{
			PhysicsContext.PrevSimulationRootBoneTransform = PhysicsContext.SimulationRootBoneTransform;
			PhysicsContext.SimulationRootBoneTransform = SimulationRootBoneTransform;
		}
	}


	static void ApplySimulationRootBone(const FHGMPhysicsContext& PhysicsContext, TArrayView<FHGMSIMDVector3> Positions, TArrayView<FHGMSIMDVector3> PrevPositions)
	{
		FHGMSIMDTransform sSimulationForce {};
		FHGMSIMDLibrary::Load(sSimulationForce, PhysicsContext.PrevSimulationRootBoneTransform.Inverse() * PhysicsContext.SimulationRootBoneTransform);

		auto Apply = [&PhysicsContext, &sSimulationForce](TArrayView<FHGMSIMDVector3>& Vectors)
			{
				for (int32 PackedIndex = 0; PackedIndex < Vectors.Num(); ++PackedIndex)
				{
					Vectors[PackedIndex] = FHGMMathLibrary::TransformPosition(sSimulationForce, Vectors[PackedIndex]);
				}
			};

		Apply(Positions);
		Apply(PrevPositions);
	}


	static void UpdateAnimationCurveValues(FComponentSpacePoseContext& Output, FHGMPhysicsContext& PhysicsContext, int32 AnimationCurveNumber)
	{
		if (!Output.AnimInstanceProxy)
		{
			return;
		}

		UAnimInstance* AnimInstance = Cast<UAnimInstance>(Output.AnimInstanceProxy->GetAnimInstanceObject());
		if (!AnimInstance)
		{
			return;
		}

		static FName HagoromoAlphaName = FName(TEXT("Hagoromo_Alpha"));
		float HagoromoAlpha = 1.0f;
		AnimInstance->GetCurveValue(HagoromoAlphaName, HagoromoAlpha);

		FName HagoromoAlphaNameWithCurveNmuber = HagoromoAlphaName;
		HagoromoAlphaNameWithCurveNmuber.SetNumber(AnimationCurveNumber + 1);
		AnimInstance->GetCurveValue(HagoromoAlphaNameWithCurveNmuber, HagoromoAlpha);

		PhysicsContext.Alpha = FHGMMathLibrary::Clamp<FHGMReal>(StaticCast<FHGMReal>(HagoromoAlpha), 0.0, 1.0);
	}


	static void CopyAnimationPositions(FComponentSpacePoseContext& Output, const TArray<FBoneReference>& Bones, TArray<FHGMSIMDVector3>& AnimationPositions)
	{
		for (int32 UnpackedBoneIndex = 0; UnpackedBoneIndex < Bones.Num(); ++UnpackedBoneIndex)
		{
			const FBoneReference& BoneReference = Bones[UnpackedBoneIndex];

			if (!FHGMAnimationLibrary::IsValidBone(BoneReference))
			{
				continue;
			}

			const FCompactPoseBoneIndex CompactBoneIndex(BoneReference.BoneIndex);
			const FHGMTransform& AnimPoseTransform = Output.Pose.GetComponentSpaceTransform(CompactBoneIndex);

			const FHGMSIMDIndex SIMDIndex(UnpackedBoneIndex);
			FHGMSIMDLibrary::Load(AnimationPositions[SIMDIndex.PackedIndex], SIMDIndex.ComponentIndex, AnimPoseTransform.GetTranslation());
		}
	}
}


// ---------------------------------------------------------------------------------------
// DynamicBoneSolver
// ---------------------------------------------------------------------------------------
#define GATHER_SETTINGS_PARAMETERS(Parameter, Curve, Dist) \
{ \
	const FRichCurve* RichCurve = Curve.GetRichCurveConst(); \
	if (!RichCurve->IsEmpty()) \
	{ \
		Dist[ChainIndex].Reserve(BoneLengthNum); \
		for (int32 BoneIndex = 0; BoneIndex < BoneLengthNum; ++BoneIndex) \
		{ \
			const FHGMReal Value = Parameter * RichCurve->Eval(UnpackedNormalizedBoneLengths[BoneIndex]); \
			Dist[ChainIndex].Emplace(Value); \
		} \
	} \
	else \
	{ \
		Dist[ChainIndex].Init(Parameter, BoneLengthNum); \
	} \
} \


bool FHGMDynamicBoneSolver::Initialize(const FBoneContainer& RequiredBones, const TArray<FHGMChainSetting>& ChainSettings, FHGMPhysicsSettings& PhysicsSettings, FHGMPhysicsContext& PhysicsContext)
{
	SCOPE_CYCLE_COUNTER(STAT_SolverInitialize);

	bHasInitialized = false;

	// Initialize SimulationRootBone.
	const FReferenceSkeleton& RefSkeleton = RequiredBones.GetReferenceSkeleton();
	if (PhysicsSettings.bUseSimulationRootBone)
	{
		if (!PhysicsSettings.SimulationRootBone.Initialize(RequiredBones))
		{
			FString SkeletonName {};
			if (USkeleton* SkeletonAsset = RequiredBones.GetSkeletonAsset())
			{
				SkeletonName = GetNameSafe(SkeletonAsset);
			}

			HGM_LOG(Error, TEXT("SimulationRootBone was not exist in skeleton: %s ."), *SkeletonName);
			return false;
		}
	}

	// Initialize LocalGravity.
	if (PhysicsSettings.GravitySettings.bUseBoneSpaceGravity)
	{
		if (!PhysicsSettings.GravitySettings.DrivingBone.Initialize(RequiredBones))
		{
			FString SkeletonName {};
			if (USkeleton* SkeletonAsset = RequiredBones.GetSkeletonAsset())
			{
				SkeletonName = GetNameSafe(SkeletonAsset);
			}

			HGM_LOG(Error, TEXT("DrivingBone was not exist in skeleton: %s ."), *SkeletonName);
			return false;
		}
	}

	// Gather chain from chain settings.
	TArray<FHGMChainSetting> CopiedChainSettings = ChainSettings;
	SimulationPlane.UnpackedHorizontalBoneNum = CopiedChainSettings.Num();
	TArray<TArray<FHGMVector3>> UnpackedChainPositions {};
	TArray<TArray<FBoneReference>> UnpackedChainBones {};
	TArray<TArray<FHGMReal>> UnpackedNormalizedBoneLengthsArray {};
	TArray<TArray<FHGMReal>> UnpackedDummyChainBoneMasks {};
	TArray<int32> UnpackedPlanarConstraintAxes {};

	UnpackedChainPositions.SetNum(SimulationPlane.UnpackedHorizontalBoneNum);
	UnpackedChainBones.SetNum(SimulationPlane.UnpackedHorizontalBoneNum);
	UnpackedNormalizedBoneLengthsArray.SetNum(SimulationPlane.UnpackedHorizontalBoneNum);
	UnpackedDummyChainBoneMasks.SetNum(SimulationPlane.UnpackedHorizontalBoneNum);
	const bool bUseAnimPosePlanarConstraint = PhysicsSettings.bUseAnimPoseConstraint && PhysicsSettings.bUseAnimPoseConstraintPlanar;
	if (bUseAnimPosePlanarConstraint)
	{
		UnpackedPlanarConstraintAxes.SetNum(SimulationPlane.UnpackedHorizontalBoneNum);
	}

	for (int32 ChainIndex = 0; ChainIndex < SimulationPlane.UnpackedHorizontalBoneNum; ++ChainIndex)
	{
		FHGMChainSetting& ChainSetting = CopiedChainSettings[ChainIndex];
		const bool bGatherChainSuccessfully = SolverInternal::GatherChain(RequiredBones, RefSkeleton, ChainSetting.ExcludeBones,
			RefSkeleton.FindBoneIndex(ChainSetting.RootBone.BoneName), UnpackedChainBones[ChainIndex], UnpackedChainPositions[ChainIndex]);

		if (!bGatherChainSuccessfully)
		{
			HGM_LOG(Error, TEXT("Chain collection failed. Bones may have been specified that are not included in mesh."));
			return false;
		}

		const int32 BoneMaxNum = UnpackedChainBones[ChainIndex].Num();
		if (BoneMaxNum <= 1)
		{
			HGM_LOG(Error, TEXT("Chain requires at least two bones."));
			return false;
		}

		UnpackedNormalizedBoneLengthsArray[ChainIndex].Reserve(BoneMaxNum);
		for (int32 BoneIndex = 0; BoneIndex < BoneMaxNum; ++BoneIndex)
		{
			UnpackedNormalizedBoneLengthsArray[ChainIndex].Emplace(StaticCast<FHGMReal>(BoneIndex) / StaticCast<FHGMReal>(BoneMaxNum - 1));
		}

		UnpackedDummyChainBoneMasks[ChainIndex].Init(0.0, BoneMaxNum);

		if (bUseAnimPosePlanarConstraint)
		{
			UnpackedPlanarConstraintAxes[ChainIndex] = ChainSetting.AnimPoseConstraintPlanarAxis;
		}
	}

	// Initialization of parameters affected by normalized bone length.
	const int32 BoneLengthArrayNum = UnpackedNormalizedBoneLengthsArray.Num();
	TArray<TArray<FHGMReal>> UnpackedRadiusesArray {};
	TArray<TArray<FHGMReal>> UnpackedFrictionsArray {};
	TArray<TArray<FHGMReal>> UnpackedWorldVelocityDampingsArray {};
	TArray<TArray<FHGMReal>> UnpackedWorldAngularVelocityDampingsArray {};
	TArray<TArray<FHGMReal>> UnpackedSimulationVelocityDampingsArray {};
	TArray<TArray<FHGMReal>> UnpackedSimulationAngularVelocityDampingsArray {};
	TArray<TArray<FHGMReal>> UnpackedMasterDampingsArray {};
	TArray<TArray<FHGMReal>> UnpackedMassesArray {};
	TArray<TArray<FHGMReal>> UnpackedRelativeLimitAnglesArray {};
	TArray<TArray<FHGMReal>> UnpackedRelativeLimitAngleDampingsArray {};
	TArray<TArray<FHGMReal>> UnpackedAnimPoseConstraintRadiusesArray {};
	TArray<TArray<FHGMReal>> UnpackedAnimPoseConstraintRadiusDampingsArray {};
	TArray<TArray<FHGMReal>> UnpackedAnimPoseConstraintAnglesArray {};
	TArray<TArray<FHGMReal>> UnpackedAnimPoseConstraintAngleDampingsArray {};

	UnpackedRadiusesArray.SetNum(BoneLengthArrayNum);
	UnpackedFrictionsArray.SetNum(BoneLengthArrayNum);
	UnpackedWorldVelocityDampingsArray.SetNum(BoneLengthArrayNum);
	UnpackedWorldAngularVelocityDampingsArray.SetNum(BoneLengthArrayNum);
	UnpackedSimulationVelocityDampingsArray.SetNum(BoneLengthArrayNum);
	UnpackedSimulationAngularVelocityDampingsArray.SetNum(BoneLengthArrayNum);
	UnpackedMasterDampingsArray.SetNum(BoneLengthArrayNum);
	UnpackedMassesArray.SetNum(BoneLengthArrayNum);
	UnpackedRelativeLimitAnglesArray.SetNum(BoneLengthArrayNum);
	UnpackedRelativeLimitAngleDampingsArray.SetNum(BoneLengthArrayNum);
	UnpackedAnimPoseConstraintRadiusesArray.SetNum(BoneLengthArrayNum);
	UnpackedAnimPoseConstraintRadiusDampingsArray.SetNum(BoneLengthArrayNum);
	UnpackedAnimPoseConstraintAnglesArray.SetNum(BoneLengthArrayNum);
	UnpackedAnimPoseConstraintAngleDampingsArray.SetNum(BoneLengthArrayNum);

	const bool bEnableAnimPoseConstraintMovableRadius = PhysicsSettings.bUseAnimPoseConstraint && PhysicsSettings.bUseAnimPoseConstraintMovableRadius;
	const bool bEnableAnimPoseConstraintLimitAngle = PhysicsSettings.bUseAnimPoseConstraint && PhysicsSettings.bUseAnimPoseConstraintLimitAngle;

	for (int32 ChainIndex = 0; ChainIndex < BoneLengthArrayNum; ++ChainIndex)
	{
		FHGMChainSetting& ChainSetting = CopiedChainSettings[ChainIndex];
		const TArray<FHGMReal>& UnpackedNormalizedBoneLengths = UnpackedNormalizedBoneLengthsArray[ChainIndex];
		const int32 BoneLengthNum = UnpackedNormalizedBoneLengths.Num();

		// Bone sphere collider radius :
		if (ChainSetting.bOverrideBoneSphereColliderRadiusEachBone)
		{
			GATHER_SETTINGS_PARAMETERS(ChainSetting.BoneSphereColliderSettings.Radius, ChainSetting.BoneSphereColliderSettings.MultiplierCurve, UnpackedRadiusesArray);
		}
		else
		{
			GATHER_SETTINGS_PARAMETERS(PhysicsSettings.BoneSphereColliderSettings.Radius, PhysicsSettings.BoneSphereColliderSettings.MultiplierCurve, UnpackedRadiusesArray);
		}

		// Friction :
		if (ChainSetting.bOverrideFrictionEachBone)
		{
			GATHER_SETTINGS_PARAMETERS(ChainSetting.FrictionSettings.Friction, ChainSetting.FrictionSettings.MultiplierCurve, UnpackedFrictionsArray);
		}
		else
		{
			GATHER_SETTINGS_PARAMETERS(PhysicsSettings.FrictionSettings.Friction, PhysicsSettings.FrictionSettings.MultiplierCurve, UnpackedFrictionsArray);
		}

		// WorldVelocityDamping :
		if (ChainSetting.bOverrideWorldVelocityDampingEachBone)
		{
			GATHER_SETTINGS_PARAMETERS(ChainSetting.WorldVelocityDampingSettings.Damping, ChainSetting.WorldVelocityDampingSettings.MultiplierCurve, UnpackedWorldVelocityDampingsArray);
		}
		else
		{
			GATHER_SETTINGS_PARAMETERS(PhysicsSettings.WorldVelocityDampingSettings.Damping, PhysicsSettings.WorldVelocityDampingSettings.MultiplierCurve, UnpackedWorldVelocityDampingsArray);
		}

		// WorldAngularVelocityDamping :
		if (ChainSetting.bOverrideWorldAngularVelocityDampingEachBone)
		{
			GATHER_SETTINGS_PARAMETERS(ChainSetting.WorldAngularVelocityDampingSettings.Damping, ChainSetting.WorldAngularVelocityDampingSettings.MultiplierCurve, UnpackedWorldAngularVelocityDampingsArray);
		}
		else
		{
			GATHER_SETTINGS_PARAMETERS(PhysicsSettings.WorldAngularVelocityDampingSettings.Damping, PhysicsSettings.WorldAngularVelocityDampingSettings.MultiplierCurve, UnpackedWorldAngularVelocityDampingsArray);
		}

		// SimulationVelocityDamping :
		if (ChainSetting.bOverrideSimulationVelocityDampingEachBone)
		{
			GATHER_SETTINGS_PARAMETERS(ChainSetting.SimulationVelocityDampingSettings.Damping, ChainSetting.SimulationVelocityDampingSettings.MultiplierCurve, UnpackedSimulationVelocityDampingsArray);
		}
		else
		{
			GATHER_SETTINGS_PARAMETERS(PhysicsSettings.SimulationVelocityDampingSettings.Damping, PhysicsSettings.SimulationVelocityDampingSettings.MultiplierCurve, UnpackedSimulationVelocityDampingsArray);
		}

		// SimulationAngularVelocityDamping :
		if (ChainSetting.bOverrideSimulationAngularVelocityDampingEachBone)
		{
			GATHER_SETTINGS_PARAMETERS(ChainSetting.SimulationAngularVelocityDampingSettings.Damping, ChainSetting.SimulationAngularVelocityDampingSettings.MultiplierCurve, UnpackedSimulationAngularVelocityDampingsArray);
		}
		else
		{
			GATHER_SETTINGS_PARAMETERS(PhysicsSettings.SimulationAngularVelocityDampingSettings.Damping, PhysicsSettings.SimulationAngularVelocityDampingSettings.MultiplierCurve, UnpackedSimulationAngularVelocityDampingsArray);
		}

		// MasterDamping :
		if (ChainSetting.bOverrideMasterDampingEachBone)
		{
			GATHER_SETTINGS_PARAMETERS(ChainSetting.MasterDampingSettings.MasterDamping, ChainSetting.MasterDampingSettings.MultiplierCurve, UnpackedMasterDampingsArray);
		}
		else
		{
			GATHER_SETTINGS_PARAMETERS(PhysicsSettings.MasterDampingSettings.MasterDamping, PhysicsSettings.MasterDampingSettings.MultiplierCurve, UnpackedMasterDampingsArray);
		}

		// Mass :
		if (ChainSetting.bOverrideMassEachBone)
		{
			const FRichCurve* MultiplierCurve = ChainSetting.MassSettings.MultiplierCurve.GetRichCurveConst();
			if (!MultiplierCurve->IsEmpty())
			{
				UnpackedMassesArray[ChainIndex].Reserve(BoneLengthNum);
				for (int32 BoneIndex = 0; BoneIndex < BoneLengthNum; ++BoneIndex)
				{
					// Preventing division by zero.
					const FHGMReal Value = 1.0 / FHGMMathLibrary::Max<FHGMReal>((ChainSetting.MassSettings.Mass * MultiplierCurve->Eval(UnpackedNormalizedBoneLengths[BoneIndex])), 0.1);
					UnpackedMassesArray[ChainIndex].Emplace(Value);
				}
			}
			else
			{
				UnpackedMassesArray[ChainIndex].Init(1.0 / FHGMMathLibrary::Max<FHGMReal>(ChainSetting.MassSettings.Mass, 0.1), BoneLengthNum);
			}
		}
		else
		{
			const FRichCurve* MultiplierCurve = PhysicsSettings.MassSettings.MultiplierCurve.GetRichCurveConst();
			if (!MultiplierCurve->IsEmpty())
			{
				UnpackedMassesArray[ChainIndex].Reserve(BoneLengthNum);
				for (int32 BoneIndex = 0; BoneIndex < BoneLengthNum; ++BoneIndex)
				{
					const FHGMReal Value = 1.0 / FHGMMathLibrary::Max<FHGMReal>((PhysicsSettings.MassSettings.Mass * MultiplierCurve->Eval(UnpackedNormalizedBoneLengths[BoneIndex])), 0.1);
					UnpackedMassesArray[ChainIndex].Emplace(Value);
				}
			}
			else
			{
				UnpackedMassesArray[ChainIndex].Init(1.0 / FHGMMathLibrary::Max<FHGMReal>(PhysicsSettings.MassSettings.Mass, 0.1), BoneLengthNum);
			}
		}

		// RelativeLimitAngle :
		if (PhysicsSettings.bUseRelativeLimitAngleConstraint)
		{
			if (ChainSetting.bOverrideRelativeLimitAngleEachBone)
			{
				GATHER_SETTINGS_PARAMETERS(ChainSetting.RelativeLimitAngleConstraintSettings.Angle, ChainSetting.RelativeLimitAngleConstraintSettings.MultiplierCurve, UnpackedRelativeLimitAnglesArray);
				GATHER_SETTINGS_PARAMETERS(ChainSetting.RelativeLimitAngleConstraintSettings.Damping, ChainSetting.RelativeLimitAngleConstraintSettings.DampingMultiplierCurve, UnpackedRelativeLimitAngleDampingsArray);
			}
			else
			{
				GATHER_SETTINGS_PARAMETERS(PhysicsSettings.RelativeLimitAngleConstraintSettings.Angle, PhysicsSettings.RelativeLimitAngleConstraintSettings.MultiplierCurve, UnpackedRelativeLimitAnglesArray);
				GATHER_SETTINGS_PARAMETERS(PhysicsSettings.RelativeLimitAngleConstraintSettings.Damping, PhysicsSettings.RelativeLimitAngleConstraintSettings.DampingMultiplierCurve, UnpackedRelativeLimitAngleDampingsArray);
			}
		}

		// AnimPoseConstraintMovableRadius :
		if (bEnableAnimPoseConstraintMovableRadius)
		{
			if (ChainSetting.bOverrideAnimPoseConstraintMovableRadiusEachBone)
			{
				GATHER_SETTINGS_PARAMETERS(ChainSetting.AnimPoseConstraintMovableRadiusSettings.Radius, ChainSetting.AnimPoseConstraintMovableRadiusSettings.MultiplierCurve, UnpackedAnimPoseConstraintRadiusesArray);
				GATHER_SETTINGS_PARAMETERS(ChainSetting.AnimPoseConstraintMovableRadiusSettings.Damping, ChainSetting.AnimPoseConstraintMovableRadiusSettings.DampingMultiplierCurve, UnpackedAnimPoseConstraintRadiusDampingsArray);
			}
			else
			{
				GATHER_SETTINGS_PARAMETERS(PhysicsSettings.AnimPoseConstraintMovableRadiusSettings.Radius, PhysicsSettings.AnimPoseConstraintMovableRadiusSettings.MultiplierCurve, UnpackedAnimPoseConstraintRadiusesArray);
				GATHER_SETTINGS_PARAMETERS(PhysicsSettings.AnimPoseConstraintMovableRadiusSettings.Damping, PhysicsSettings.AnimPoseConstraintMovableRadiusSettings.DampingMultiplierCurve, UnpackedAnimPoseConstraintRadiusDampingsArray);
			}
		}

		// AnimPoseConstraintLimitAngle.
		if (bEnableAnimPoseConstraintLimitAngle)
		{
			if (ChainSetting.bOverrideAnimPoseConstraintLimitAngleEachBone)
			{
				GATHER_SETTINGS_PARAMETERS(ChainSetting.AnimPoseConstraintLimitAngleSettings.Angle, ChainSetting.AnimPoseConstraintLimitAngleSettings.MultiplierCurve, UnpackedAnimPoseConstraintAnglesArray);
				GATHER_SETTINGS_PARAMETERS(ChainSetting.AnimPoseConstraintLimitAngleSettings.Damping, ChainSetting.AnimPoseConstraintLimitAngleSettings.DampingMultiplierCurve, UnpackedAnimPoseConstraintAngleDampingsArray);
			}
			else
			{
				GATHER_SETTINGS_PARAMETERS(PhysicsSettings.AnimPoseConstraintLimitAngleSettings.Angle, PhysicsSettings.AnimPoseConstraintLimitAngleSettings.MultiplierCurve, UnpackedAnimPoseConstraintAnglesArray);
				GATHER_SETTINGS_PARAMETERS(PhysicsSettings.AnimPoseConstraintLimitAngleSettings.Damping, PhysicsSettings.AnimPoseConstraintLimitAngleSettings.DampingMultiplierCurve, UnpackedAnimPoseConstraintAngleDampingsArray);
			}
		}
	}

	// Creates simulation plane that is multiple of 4 x 4.
	// This is to simplify SIMD calculations.
	// Add dummy bones and parameters to vertical chain.
	// Simplifies calculation of SIMD in horizontal direction.
	int32 ChainBoneMaxNum = 0;
	for (const TArray<FHGMVector3>& ChainPositions : UnpackedChainPositions)
	{
		ChainBoneMaxNum = FHGMMathLibrary::Max(ChainPositions.Num(), ChainBoneMaxNum);
	}

	SimulationPlane.UnpackedVerticalBoneNum = FHGMMathLibrary::RoundUpToMultiple(ChainBoneMaxNum, 4);
	for (int32 ChainIndex = 0; ChainIndex < SimulationPlane.UnpackedHorizontalBoneNum; ++ChainIndex)
	{
		TArray<FHGMVector3>& ChainPositions = UnpackedChainPositions[ChainIndex];

		const int32 ChainBoneNum = ChainPositions.Num();
		const int32 ChainBoneNumDifference = SimulationPlane.UnpackedVerticalBoneNum - ChainBoneNum;
		if (ChainBoneNumDifference == 0)
		{
			continue;
		}

		SolverInternal::AddDummyBone(ChainPositions, UnpackedChainBones[ChainIndex], UnpackedDummyChainBoneMasks[ChainIndex], ChainBoneNumDifference);

		TArray<FHGMReal> DummyZeroReals {};
		DummyZeroReals.Init(0.0, ChainBoneNumDifference);

		TArray<FHGMReal> DummyOneReals {};
		DummyOneReals.Init(1.0, ChainBoneNumDifference);

		UnpackedRadiusesArray[ChainIndex] += DummyZeroReals;
		UnpackedFrictionsArray[ChainIndex] += DummyZeroReals;
		UnpackedWorldVelocityDampingsArray[ChainIndex] += DummyZeroReals;
		UnpackedWorldAngularVelocityDampingsArray[ChainIndex] += DummyZeroReals;
		UnpackedSimulationVelocityDampingsArray[ChainIndex] += DummyZeroReals;
		UnpackedSimulationAngularVelocityDampingsArray[ChainIndex] += DummyZeroReals;
		UnpackedMasterDampingsArray[ChainIndex] += DummyZeroReals;

		if (PhysicsSettings.bUseRelativeLimitAngleConstraint)
		{
			UnpackedRelativeLimitAnglesArray[ChainIndex] += DummyZeroReals;
			UnpackedRelativeLimitAngleDampingsArray[ChainIndex] += DummyZeroReals;
		}

		if (bEnableAnimPoseConstraintMovableRadius)
		{
			UnpackedAnimPoseConstraintRadiusesArray[ChainIndex] += DummyZeroReals;
			UnpackedAnimPoseConstraintRadiusDampingsArray[ChainIndex] += DummyZeroReals;
		}

		if (bEnableAnimPoseConstraintLimitAngle)
		{
			UnpackedAnimPoseConstraintAnglesArray[ChainIndex] += DummyZeroReals;
			UnpackedAnimPoseConstraintAngleDampingsArray[ChainIndex] += DummyZeroReals;
		}

		UnpackedMassesArray[ChainIndex] += DummyOneReals;
	}

	// Add dummy chain and parameters.
	// Simplifies calculation of SIMD in vertical direction.
	const int32 SIMDChainNum = FHGMMathLibrary::RoundUpToMultiple(SimulationPlane.UnpackedHorizontalBoneNum, 4);
	const int32 DummyChainNum = SIMDChainNum - SimulationPlane.UnpackedHorizontalBoneNum;
	if (DummyChainNum > 0)
	{
		SolverInternal::AddDummyChain(CopiedChainSettings, UnpackedChainPositions, UnpackedChainBones, UnpackedDummyChainBoneMasks, DummyChainNum);
		SimulationPlane.UnpackedHorizontalBoneNum = CopiedChainSettings.Num();

		TArray<FHGMReal> DummyZeroReals {};
		DummyZeroReals.Init(0.0, UnpackedChainPositions.Last(0).Num());

		TArray<FHGMReal> DummyOneReals {};
		DummyOneReals.Init(1.0, UnpackedChainPositions.Last(0).Num());

		for (int32 DummyChainCount = 0; DummyChainCount < DummyChainNum; ++DummyChainCount)
		{
			UnpackedRadiusesArray.Emplace(DummyZeroReals);
			UnpackedFrictionsArray.Emplace(DummyZeroReals);
			UnpackedWorldVelocityDampingsArray.Emplace(DummyZeroReals);
			UnpackedWorldAngularVelocityDampingsArray.Emplace(DummyZeroReals);
			UnpackedSimulationVelocityDampingsArray.Emplace(DummyZeroReals);
			UnpackedSimulationAngularVelocityDampingsArray.Emplace(DummyZeroReals);
			UnpackedMasterDampingsArray.Emplace(DummyZeroReals);

			if (PhysicsSettings.bUseRelativeLimitAngleConstraint)
			{
				UnpackedRelativeLimitAnglesArray.Emplace(DummyZeroReals);
				UnpackedRelativeLimitAngleDampingsArray.Emplace(DummyZeroReals);
			}

			if (bEnableAnimPoseConstraintMovableRadius)
			{
				UnpackedAnimPoseConstraintRadiusesArray.Emplace(DummyZeroReals);
				UnpackedAnimPoseConstraintRadiusDampingsArray.Emplace(DummyZeroReals);
			}

			if (bEnableAnimPoseConstraintLimitAngle)
			{
				UnpackedAnimPoseConstraintAnglesArray.Emplace(DummyZeroReals);
				UnpackedAnimPoseConstraintAngleDampingsArray.Emplace(DummyZeroReals);
			}

			UnpackedMassesArray.Emplace(DummyOneReals);

			if (bUseAnimPosePlanarConstraint)
			{
				UnpackedPlanarConstraintAxes.Emplace(0);
			}
		}
	}

	// To facilitate calculation with SIMD, chain data is converted to one-dimensional data.
	const int32 UnpackedPositionNum = SimulationPlane.UnpackedHorizontalBoneNum * SimulationPlane.UnpackedVerticalBoneNum;
	const int32 PackedPositionNum = UnpackedPositionNum / 4;
	SimulationPlane.PackedHorizontalBoneNum = SimulationPlane.UnpackedHorizontalBoneNum / 4;
	SimulationPlane.PackedVerticalBoneNum = SimulationPlane.UnpackedVerticalBoneNum / 4;
	SimulationPlane.ActualUnpackedHorizontalBoneNum = ChainSettings.Num();
	SimulationPlane.ActualPackedHorizontalBoneNum = (ChainSettings.Num() / 4) + 1;

	Bones.Reset(UnpackedPositionNum);
	Positions.Reset(PackedPositionNum);
	FixedBlends.Reset(PackedPositionNum);
	DummyBoneMasks.Reset(PackedPositionNum);
	BoneSphereColliderRadiuses.Reset(PackedPositionNum);
	Frictions.Reset(PackedPositionNum);
	InverseMasses.Reset(PackedPositionNum);
	WorldVelocityDampings.Reset(PackedPositionNum);
	WorldAngularVelocityDampings.Reset(PackedPositionNum);
	SimulationVelocityDampings.Reset(PackedPositionNum);
	SimulationAngularVelocityDampings.Reset(PackedPositionNum);
	MasterDampings.Reset(PackedPositionNum);
	RelativeLimitAngles.Reset(PackedPositionNum);
	AnimPoseConstraintMovableRadiuses.Reset(PackedPositionNum);
	AnimPoseConstraintLimitAngles.Reset(PackedPositionNum);
	for (int32 VerticalChainBoneIndex = 0; VerticalChainBoneIndex < SimulationPlane.UnpackedVerticalBoneNum; ++VerticalChainBoneIndex)
	{
		for (int32 HorizontalChainIndexBase = 0; HorizontalChainIndexBase < SimulationPlane.UnpackedHorizontalBoneNum; HorizontalChainIndexBase += 4)
		{
			TStaticArray<FHGMVector3, 4> UnpackedPositions {};
			TStaticArray<FHGMReal, 4> UnpackedDummyBoneMasks {};
			TStaticArray<FHGMReal, 4> UnpackedFixedBlneds {};
			TStaticArray<FHGMReal, 4> UnpackedRadiuses {};
			TStaticArray<FHGMReal, 4> UnpackedFrictions {};
			TStaticArray<FHGMReal, 4> UnpackedWorldVelocityDampings {};
			TStaticArray<FHGMReal, 4> UnpackedWorldAngularVelocityDampings {};
			TStaticArray<FHGMReal, 4> UnpackedSimulationVelocityDampings {};
			TStaticArray<FHGMReal, 4> UnpackedSimulationAngularVelocityDampings {};
			TStaticArray<FHGMReal, 4> UnpackedMasterDampings {};
			TStaticArray<FHGMReal, 4> UnpackedMasses {};
			TStaticArray<FHGMReal, 4> UnpackedRelativeLimitAngles {};
			TStaticArray<FHGMReal, 4> UnpackedRelativeLimitAngleDampings {};
			TStaticArray<FHGMReal, 4> UnpackedAnimPoseConstraintRadiuses {};
			TStaticArray<FHGMReal, 4> UnpackedAnimPoseConstraintRadiusDampings {};
			TStaticArray<FHGMReal, 4> UnpackedAnimPoseConstraintAngles {};
			TStaticArray<FHGMReal, 4> UnpackedAnimPoseConstraintAngleDampings {};
			for (int32 Offset = 0; Offset < 4; ++Offset)
			{
				UnpackedPositions[Offset] = UnpackedChainPositions[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedDummyBoneMasks[Offset] = UnpackedDummyChainBoneMasks[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedRadiuses[Offset] = UnpackedRadiusesArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedFrictions[Offset] = UnpackedFrictionsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedWorldVelocityDampings[Offset] = UnpackedWorldVelocityDampingsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedWorldAngularVelocityDampings[Offset] = UnpackedWorldAngularVelocityDampingsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedSimulationVelocityDampings[Offset] = UnpackedSimulationVelocityDampingsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedSimulationAngularVelocityDampings [Offset] = UnpackedSimulationAngularVelocityDampingsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedMasterDampings[Offset] = UnpackedMasterDampingsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				UnpackedMasses[Offset] = UnpackedMassesArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];

				if (PhysicsSettings.bUseRelativeLimitAngleConstraint)
				{
					UnpackedRelativeLimitAngles[Offset] = UnpackedRelativeLimitAnglesArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
					UnpackedRelativeLimitAngleDampings[Offset] = UnpackedRelativeLimitAngleDampingsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				}

				if (bEnableAnimPoseConstraintMovableRadius)
				{
					UnpackedAnimPoseConstraintRadiuses[Offset] = UnpackedAnimPoseConstraintRadiusesArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
					UnpackedAnimPoseConstraintRadiusDampings[Offset] = UnpackedAnimPoseConstraintRadiusDampingsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				}

				if (bEnableAnimPoseConstraintLimitAngle)
				{
					UnpackedAnimPoseConstraintAngles[Offset] = UnpackedAnimPoseConstraintAnglesArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
					UnpackedAnimPoseConstraintAngleDampings[Offset] = UnpackedAnimPoseConstraintAngleDampingsArray[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];

				}

				if (VerticalChainBoneIndex == 0)
				{
					const FHGMChainSetting& ChainSetting = CopiedChainSettings[HorizontalChainIndexBase + Offset];
					UnpackedFixedBlneds[Offset] = ChainSetting.bBeginBonePositionFixed ? 1.0 : 0.0;
				}
				else
				{
					UnpackedFixedBlneds[Offset] = 0.0f;
				}

				const FBoneReference& ChainBone = UnpackedChainBones[HorizontalChainIndexBase + Offset][VerticalChainBoneIndex];
				Bones.Emplace(ChainBone);
			}

			FHGMSIMDVector3 sPosition {};
			FHGMSIMDLibrary::Load(sPosition, UnpackedPositions);
			Positions.Emplace(sPosition);

			FHGMSIMDReal sDummyBoneMask {};
			FHGMSIMDLibrary::Load(sDummyBoneMask, UnpackedDummyBoneMasks);
			DummyBoneMasks.Emplace(sDummyBoneMask);

			FHGMSIMDReal sFixedBlend {};
			FHGMSIMDLibrary::Load(sFixedBlend, UnpackedFixedBlneds);
			FixedBlends.Emplace(sFixedBlend);

			FHGMSIMDReal sRadius {};
			FHGMSIMDLibrary::Load(sRadius, UnpackedRadiuses);
			BoneSphereColliderRadiuses.Emplace(sRadius);

			FHGMSIMDReal sFriction {};
			FHGMSIMDLibrary::Load(sFriction, UnpackedFrictions);
			Frictions.Emplace(sFriction);

			FHGMSIMDReal sWorldVelocityDamping {};
			FHGMSIMDLibrary::Load(sWorldVelocityDamping, UnpackedWorldVelocityDampings );
			WorldVelocityDampings.Emplace(sWorldVelocityDamping);

			FHGMSIMDReal sWorldAngularVelocityDamping {};
			FHGMSIMDLibrary::Load(sWorldAngularVelocityDamping, UnpackedWorldAngularVelocityDampings );
			WorldAngularVelocityDampings .Emplace(sWorldAngularVelocityDamping);

			FHGMSIMDReal sSimulationVelocityDamping {};
			FHGMSIMDLibrary::Load(sSimulationVelocityDamping, UnpackedSimulationVelocityDampings );
			SimulationVelocityDampings.Emplace(sSimulationVelocityDamping);

			FHGMSIMDReal sSimulationAngularVelocityDamping {};
			FHGMSIMDLibrary::Load(sSimulationAngularVelocityDamping, UnpackedSimulationAngularVelocityDampings );
			SimulationAngularVelocityDampings .Emplace(sSimulationAngularVelocityDamping);

			FHGMSIMDReal sMasterDamping {};
			FHGMSIMDLibrary::Load(sMasterDamping, UnpackedMasterDampings);
			MasterDampings.Emplace(sMasterDamping);

			FHGMSIMDReal sMass {};
			FHGMSIMDLibrary::Load(sMass, UnpackedMasses);
			InverseMasses.Emplace(sMass);

			if (PhysicsSettings.bUseRelativeLimitAngleConstraint)
			{
				FHGMSIMDRelativeLimitAngle sRelativeLimitAngle {};
				FHGMSIMDLibrary::Load(sRelativeLimitAngle.sAngle, UnpackedRelativeLimitAngles);
				FHGMSIMDLibrary::Load(sRelativeLimitAngle.sDamping, UnpackedRelativeLimitAngleDampings);
				RelativeLimitAngles.Emplace(sRelativeLimitAngle);
			}

			if (bEnableAnimPoseConstraintMovableRadius)
			{
				FHGMSIMDAnimPoseConstraintMovableRadius sAnimPoseConstraintMovableRadius {};
				FHGMSIMDLibrary::Load(sAnimPoseConstraintMovableRadius.sRadius, UnpackedAnimPoseConstraintRadiuses);
				FHGMSIMDLibrary::Load(sAnimPoseConstraintMovableRadius.sDamping, UnpackedAnimPoseConstraintRadiusDampings);
				AnimPoseConstraintMovableRadiuses.Emplace(sAnimPoseConstraintMovableRadius);
			}

			if (bEnableAnimPoseConstraintLimitAngle)
			{
				FHGMSIMDAnimPoseConstraintLimitAngle sAnimPoseConstraintLimitAngle {};

				FHGMSIMDReal sAnimPoseConstraintAngles {};
				FHGMSIMDLibrary::Load(sAnimPoseConstraintLimitAngle.sAngle, UnpackedAnimPoseConstraintAngles);
				FHGMSIMDLibrary::Load(sAnimPoseConstraintLimitAngle.sDamping, UnpackedAnimPoseConstraintAngleDampings);
				AnimPoseConstraintLimitAngles.Emplace(sAnimPoseConstraintLimitAngle);
			}
		}
	}

	if (bUseAnimPosePlanarConstraint)
	{
		AnimPosePlanarConstraintAxes.Init(HGMSIMDConstants::ZeroInt, SimulationPlane.PackedHorizontalBoneNum);
		for (int32 HorizontalChainIndexBase = 0; HorizontalChainIndexBase < SimulationPlane.UnpackedHorizontalBoneNum; HorizontalChainIndexBase += 4)
		{
			FHGMSIMDInt sPlanarConstraintAxis {};
			FHGMSIMDLibrary::Load(AnimPosePlanarConstraintAxes[HorizontalChainIndexBase / 4], UnpackedPlanarConstraintAxes[HorizontalChainIndexBase + 0], UnpackedPlanarConstraintAxes[HorizontalChainIndexBase + 1],
							UnpackedPlanarConstraintAxes[HorizontalChainIndexBase + 2], UnpackedPlanarConstraintAxes[HorizontalChainIndexBase + 3]);
		}
	}

	ReferencePositions = AnimPosePositions = PrevPositions = Positions;
	ActualFrictions.Init(HGMSIMDConstants::ZeroReal, Positions.Num());

	// Make structures.
	VerticalStructures.Reset(SimulationPlane.PackedHorizontalBoneNum * SimulationPlane.UnpackedVerticalBoneNum);
	FHGMConstraintLibrary::MakeVerticalStructure(SimulationPlane, Positions, VerticalStructures);

	if (PhysicsSettings.bUseHorizontalStructuralConstraint)
	{
		HorizontalStructures.Reset(SimulationPlane.PackedHorizontalBoneNum * SimulationPlane.UnpackedVerticalBoneNum);
		FHGMConstraintLibrary::MakeHorizontalStructure(SimulationPlane, Positions, PhysicsSettings.bLoopHorizontalStructure, HorizontalStructures);
	}

	if (PhysicsSettings.bUseVerticalBendConstraint)
	{
		FHGMConstraintLibrary::MakeVerticalBendStructure(SimulationPlane, Positions, VerticalBendStructures);
	}

	if (PhysicsSettings.bUseHorizontalBendConstraint)
	{
		FHGMConstraintLibrary::MakeHorizontalBendStructure(SimulationPlane, Positions, HorizontalBendStructures);
	}

	if (PhysicsSettings.bUseShearConstraint)
	{
		FHGMConstraintLibrary::MakeShearStructure(SimulationPlane, Positions, PhysicsSettings.bLoopHorizontalStructure, ShearStructures);
	}

	// Initialize physics context.
	PhysicsContext.PhysicsSettings = PhysicsSettings;
	SolverInternal::ConvertStiffnessesToCompliances(PhysicsSettings, PhysicsContext);

	bHasInitialized = true;

	return true;
}


void FHGMDynamicBoneSolver::PreSimulate(FComponentSpacePoseContext& Output, const FHGMPhysicsSettings& PhysicsSettings, FHGMPhysicsContext& PhysicsContext, int32 AnimationCurveNumber)
{
	SCOPE_CYCLE_COUNTER(STAT_SolverPreSimulate);

	SolverInternal::UpdateDeltaTime(Output, PhysicsContext);

	SolverInternal::UpdateAnimationCurveValues(Output, PhysicsContext, AnimationCurveNumber);

	SolverInternal::UpdateSkeletalMeshComponentTransform(Output, PhysicsContext);

	// Pre-factoring in posture changes of specified bones so that they do not affect physics simulation.
	if (PhysicsContext.PhysicsSettings.bUseSimulationRootBone)
	{
		SolverInternal::UpdateSimulationRootBoneTransform(Output, PhysicsContext);
		SolverInternal::ApplySimulationRootBone(PhysicsContext, Positions, PrevPositions);
	}

	SolverInternal::CopyAnimationPositions(Output, Bones, AnimPosePositions);
}


void FHGMDynamicBoneSolver::Simulate(FComponentSpacePoseContext& Output, FHGMPhysicsContext& PhysicsContext, const FHGMBodyCollider& BodyCollider, const FHGMBodyCollider& PrevBodyCollider, TConstArrayView<FHGMSIMDPlaneCollider> PlaneColliders)
{
	SCOPE_CYCLE_COUNTER(STAT_SolverSimulate);

	//----------------------------------------------------------
	// Add forces
	//----------------------------------------------------------
	FHGMPhysicsLibrary::ApplyForces(Output, PhysicsContext, Positions, PrevPositions,
								WorldVelocityDampings, WorldAngularVelocityDampings, SimulationVelocityDampings, SimulationAngularVelocityDampings, MasterDampings,
								ActualFrictions, FixedBlends, DummyBoneMasks);


	//----------------------------------------------------------
	// Update positions
	//----------------------------------------------------------
	FHGMPhysicsLibrary::VerletIntegrate(PhysicsContext, Positions, PrevPositions, ActualFrictions, MasterDampings, FixedBlends, DummyBoneMasks);


	//----------------------------------------------------------
	// Solve constraints and collision detection
	//----------------------------------------------------------
	FHGMConstraintLibrary::FixedBlendConstraint(Positions, PrevPositions, AnimPosePositions, FixedBlends);

	if (PhysicsContext.PhysicsSettings.bUseRelativeLimitAngleConstraint)
	{
		FHGMConstraintLibrary::RelativeLimitAngleConstraint(SimulationPlane, VerticalStructures, RelativeLimitAngles, AnimPosePositions, Positions, DummyBoneMasks);
	}

	if (PhysicsContext.PhysicsSettings.bUseAnimPoseConstraint)
	{
		if (PhysicsContext.PhysicsSettings.bUseAnimPoseConstraintMovableRadius)
		{
			FHGMConstraintLibrary::AnimPoseMovableRadiusConstraint(AnimPoseConstraintMovableRadiuses, AnimPosePositions, Positions);
		}

		if (PhysicsContext.PhysicsSettings.bUseAnimPoseConstraintLimitAngle)
		{
			FHGMConstraintLibrary::AnimPoseLimitAngleConstraint(VerticalStructures, AnimPoseConstraintLimitAngles, AnimPosePositions, Positions);
		}

		if (PhysicsContext.PhysicsSettings.bUseAnimPoseConstraintPlanar)
		{
			FHGMConstraintLibrary::AnimPosePlanarConstraint(AnimPosePlanarConstraintAxes, VerticalStructures, SimulationPlane, Output, Bones, AnimPosePositions, Positions);
		}
	}

	FHGMConstraintLibrary::ResetLambda<FHGMSIMDStructure>(VerticalStructures);

	if (PhysicsContext.PhysicsSettings.bUseHorizontalStructuralConstraint)
	{
		FHGMConstraintLibrary::ResetLambda<FHGMSIMDStructure>(HorizontalStructures);
	}

	if (PhysicsContext.PhysicsSettings.bUseVerticalBendConstraint)
	{
		FHGMConstraintLibrary::ResetLambda<FHGMSIMDStructure>(VerticalBendStructures);
	}

	if (PhysicsContext.PhysicsSettings.bUseHorizontalBendConstraint)
	{
		FHGMConstraintLibrary::ResetLambda<FHGMSIMDStructure>(HorizontalBendStructures);
	}

	if (PhysicsContext.PhysicsSettings.bUseShearConstraint)
	{
		FHGMConstraintLibrary::ResetLambda<FHGMSIMDShearStructure>(ShearStructures);
	}

	FHGMSIMDReal sCollisionBlend {};
	FHGMSIMDLibrary::Load(sCollisionBlend, PhysicsContext.PhysicsSettings.CollisionBlend);

	FHGMSIMDReal sColliderPenetrationDepth {};
	FHGMSIMDLibrary::Load(sColliderPenetrationDepth, PhysicsContext.PhysicsSettings.ColliderPenetrationDepth);

	for (int32 IterationCount = 0; IterationCount < PhysicsContext.PhysicsSettings.SolverIterations; ++IterationCount)
	{
		// Collision detection.
		// #OPTIMIZE
		//  - Add mechanism to cache contacts to reduce number of calculations. However, caches should be considered carefully as fabric penetration is likely to occur.
		//  - Implement Acceleration Structure. Care should be taken not to increase load by footing.
		BodyColliderContactCache.Reset();
		FHGMCollisionLibrary::CalculateBodyColliderContacts(BoneSphereColliderRadiuses, Positions, PrevPositions, BodyCollider, PrevBodyCollider, BodyColliderContactCache);

		if (PhysicsContext.PhysicsSettings.bUseEdgeCollider)
		{
			VerticalContactCache.Reset();
			FHGMCollisionLibrary::CalculateBodyColliderContactsForVerticalEdge(VerticalStructures, Positions, BodyCollider, VerticalContactCache);

			if (PhysicsContext.PhysicsSettings.bUseHorizontalEdgeCollider && !HorizontalStructures.IsEmpty())
			{
				HorizontalContactCache.Reset();
				FHGMCollisionLibrary::CalculateBodyColliderContactsForHorizontalEdge(SimulationPlane, HorizontalStructures, Positions, BodyCollider, HorizontalContactCache);
			}
		}

		PlaneColliderContactCache.Reset();
		FHGMCollisionLibrary::CalculatePlaneColliderContacts(PlaneColliders, BoneSphereColliderRadiuses, Positions, PlaneColliderContactCache);

		// Applying Constraints.
		if (PhysicsContext.PhysicsSettings.bUseRigidVerticalStructureConstraint)
		{
			FHGMConstraintLibrary::RigidVerticalStructuralConstraint(VerticalStructures, Positions, FixedBlends);
		}
		else
		{
			FHGMConstraintLibrary::VerticalStructuralConstraint(VerticalStructures, PhysicsContext, Positions, InverseMasses, FixedBlends, DummyBoneMasks);
		}

		if (PhysicsContext.PhysicsSettings.bUseHorizontalStructuralConstraint)
		{
			FHGMConstraintLibrary::HorizontalStructuralConstraint(HorizontalStructures, PhysicsContext, SimulationPlane, Positions, InverseMasses, FixedBlends, DummyBoneMasks);
		}

		if (PhysicsContext.PhysicsSettings.bUseVerticalBendConstraint)
		{
			FHGMConstraintLibrary::VerticalBendConstraint(VerticalBendStructures, PhysicsContext, Positions, InverseMasses, FixedBlends, DummyBoneMasks);
		}

		if (PhysicsContext.PhysicsSettings.bUseHorizontalBendConstraint)
		{
			FHGMConstraintLibrary::HorizontalBendConstraint(HorizontalBendStructures, PhysicsContext, SimulationPlane, Positions, InverseMasses, FixedBlends, DummyBoneMasks);
		}

		if (PhysicsContext.PhysicsSettings.bUseShearConstraint)
		{
			FHGMConstraintLibrary::ShearConstraint(ShearStructures, PhysicsContext, SimulationPlane, Positions, InverseMasses, FixedBlends, DummyBoneMasks);
		}

		// Solve contacts.
		FHGMConstraintLibrary::ColliderContactConstraint(BodyColliderContactCache, sCollisionBlend, sColliderPenetrationDepth, Positions, FixedBlends, DummyBoneMasks);

		if (PhysicsContext.PhysicsSettings.bUseEdgeCollider)
		{
			FHGMConstraintLibrary::ColliderContactConstraint(VerticalContactCache, sCollisionBlend, sColliderPenetrationDepth, Positions, FixedBlends, DummyBoneMasks);
			if (PhysicsContext.PhysicsSettings.bUseHorizontalEdgeCollider && !HorizontalStructures.IsEmpty())
			{
				FScopedTranspose ScopedTranspose(&SimulationPlane);
				ScopedTranspose.Add(&Positions);
				ScopedTranspose.Add(&FixedBlends);
				ScopedTranspose.Add(&DummyBoneMasks);
				ScopedTranspose.Execute();
				FHGMConstraintLibrary::ColliderContactConstraint(HorizontalContactCache, sCollisionBlend, sColliderPenetrationDepth, Positions, FixedBlends, DummyBoneMasks);
			}
		}

		FHGMConstraintLibrary::ColliderContactConstraint(PlaneColliderContactCache, sCollisionBlend, sColliderPenetrationDepth, Positions, FixedBlends, DummyBoneMasks);

		// Calculate frictions.
		if (IterationCount == 0)
		{
			FHGMPhysicsLibrary::ResetFriction(ActualFrictions);

			FHGMPhysicsLibrary::CalculateFriction(Frictions, BodyColliderContactCache, ActualFrictions);

			if (PhysicsContext.PhysicsSettings.bUseEdgeCollider)
			{
				FHGMPhysicsLibrary::CalculateFriction(Frictions, VerticalContactCache, ActualFrictions);
			}

			FHGMPhysicsLibrary::CalculateFriction(Frictions, PlaneColliderContactCache, ActualFrictions);
		}
	}
}


void FHGMDynamicBoneSolver::OutputSimulateResult(FHGMPhysicsContext& PhysicsContext, FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_SolverOutputSimulateResult);

	const FBoneContainer& BoneContainer =  Output.Pose.GetPose().GetBoneContainer();

	TArray<TStaticArray<FHGMQuaternion, 4>> PrevBoneQuaternions {};
	PrevBoneQuaternions.SetNum(SimulationPlane.PackedHorizontalBoneNum);

	for (int32 VerticalStructureIndex = 0; VerticalStructureIndex < VerticalStructures.Num(); ++VerticalStructureIndex)
	{
		const FHGMSIMDStructure& VerticalStructure = VerticalStructures[VerticalStructureIndex];

		TStaticArray<int32, 4> FirstBoneIndexes {};
		FHGMSIMDLibrary::Store(VerticalStructure.sFirstBoneUnpackedIndex, FirstBoneIndexes);

		TStaticArray<int32, 4> SecondBoneIndexes {};
		FHGMSIMDLibrary::Store(VerticalStructure.sSecondBoneUnpackedIndex, SecondBoneIndexes);

		const int32 PackedHorizontalIndex = VerticalStructureIndex % SimulationPlane.PackedHorizontalBoneNum;
		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			const int32 FirstBoneIndex = FirstBoneIndexes[ComponentIndex];
			const FBoneReference& FirstBone = Bones[FirstBoneIndex];

			if (!FirstBone.IsValidToEvaluate())
			{
				continue;
			}

			const FCompactPoseBoneIndex FirstBoneCompactIndex = FirstBone.GetCompactPoseIndex(BoneContainer);
			const FHGMTransform& OriginalFirstBoneTransform = Output.Pose.GetComponentSpaceTransform(FirstBoneCompactIndex);

			FHGMVector3 FirstBonePosition {};
			FHGMSIMDLibrary::Store(Positions[VerticalStructure.FirstBonePackedIndex], ComponentIndex, FirstBonePosition);

			const int32 SecondBoneIndex = SecondBoneIndexes[ComponentIndex];
			const FBoneReference& SecondBone = Bones[SecondBoneIndex];
			if (!SecondBone.IsValidToEvaluate())
			{
				const FHGMTransform BoneTransform(PrevBoneQuaternions[PackedHorizontalIndex][ComponentIndex], FirstBonePosition, OriginalFirstBoneTransform.GetScale3D());
				OutBoneTransforms.Add(FBoneTransform(FirstBoneCompactIndex, BoneTransform));
				continue;
			}

			const FCompactPoseBoneIndex SecondBoneCompactIndex = SecondBone.GetCompactPoseIndex(BoneContainer);
			const FHGMTransform& OriginalSecondBoneTransform = Output.Pose.GetComponentSpaceTransform(SecondBoneCompactIndex);

			const FHGMVector3 OriginalPrimaryVector = OriginalSecondBoneTransform.GetTranslation() - OriginalFirstBoneTransform.GetTranslation();

			FHGMVector3 SecondBonePosition {};
			FHGMSIMDLibrary::Store(Positions[VerticalStructure.SecondBonePackedIndex], ComponentIndex, SecondBonePosition);

			const FHGMVector3 BonePrimaryVector = SecondBonePosition - FirstBonePosition;
			const FHGMQuaternion BoneQuaternion = FHGMQuaternion::FindBetweenVectors(OriginalPrimaryVector, BonePrimaryVector) * OriginalFirstBoneTransform.GetRotation();
			FHGMTransform BoneTransform(BoneQuaternion, FirstBonePosition, OriginalFirstBoneTransform.GetScale3D());

			if (PhysicsContext.Alpha < 1.0)
			{
				BoneTransform.BlendWith(OriginalFirstBoneTransform, 1.0 - PhysicsContext.Alpha);
			}

			OutBoneTransforms.Add(FBoneTransform(FirstBoneCompactIndex, BoneTransform));

			PrevBoneQuaternions[PackedHorizontalIndex][ComponentIndex] = BoneQuaternion;
		}
	}

	// Tip bone outputs same posture as parent because no pair exists.
	for (int32 LeafVerticalStructureIndex = VerticalStructures.Num() - SimulationPlane.PackedHorizontalBoneNum; LeafVerticalStructureIndex < VerticalStructures.Num(); ++LeafVerticalStructureIndex)
	{
		const FHGMSIMDStructure& VerticalStructure = VerticalStructures[LeafVerticalStructureIndex];
		const int32 PackedHorizontalIndex = LeafVerticalStructureIndex % SimulationPlane.PackedHorizontalBoneNum;

		TStaticArray<int32, 4> LeafBoneIndexes {};
		FHGMSIMDLibrary::Store(VerticalStructure.sSecondBoneUnpackedIndex, LeafBoneIndexes);
		for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
		{
			const int32 LeafBoneIndex = LeafBoneIndexes[ComponentIndex];
			const FBoneReference& LeafBone = Bones[LeafBoneIndex];
			if (!LeafBone.IsValidToEvaluate())
			{
				continue;
			}

			const FCompactPoseBoneIndex LeafBoneCompactIndex = LeafBone.GetCompactPoseIndex(BoneContainer);
			const FHGMTransform& OriginalLeafBoneTransform = Output.Pose.GetComponentSpaceTransform(LeafBoneCompactIndex);

			FHGMVector3 LeafBonePosition {};
			FHGMSIMDLibrary::Store(Positions[VerticalStructure.SecondBonePackedIndex], ComponentIndex, LeafBonePosition);
			FHGMTransform BoneTransform(PrevBoneQuaternions[PackedHorizontalIndex][ComponentIndex], LeafBonePosition, OriginalLeafBoneTransform.GetScale3D());

			if (PhysicsContext.Alpha < 1.0)
			{
				BoneTransform.BlendWith(OriginalLeafBoneTransform, 1.0 - PhysicsContext.Alpha);
			}

			OutBoneTransforms.Add(FBoneTransform(LeafBoneCompactIndex, BoneTransform));
		}
	}

	OutBoneTransforms.RemoveAll([](const FBoneTransform& BoneTransform)
	{
		return BoneTransform.BoneIndex < 0;
	});

	OutBoneTransforms.Sort(FCompareBoneTransformIndex());
}


// ---------------------------------------------------------------------------------------
// SolverLibrary
// ---------------------------------------------------------------------------------------
void FHGMSolverLibrary::Transpose(FHGMSimulationPlane& SimulationPlane)
{
	const int32 CopiedUnpackedVerticalBoneNum = SimulationPlane.UnpackedVerticalBoneNum;
	const int32 CopiedPackedVerticalBoneNum = SimulationPlane.PackedVerticalBoneNum;

	SimulationPlane.UnpackedVerticalBoneNum = SimulationPlane.UnpackedHorizontalBoneNum;
	SimulationPlane.PackedVerticalBoneNum = SimulationPlane.PackedHorizontalBoneNum;

	SimulationPlane.UnpackedHorizontalBoneNum = CopiedUnpackedVerticalBoneNum;
	SimulationPlane.PackedHorizontalBoneNum = CopiedPackedVerticalBoneNum;
}