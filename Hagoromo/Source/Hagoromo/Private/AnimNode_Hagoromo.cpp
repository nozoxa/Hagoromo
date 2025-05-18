// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "AnimNode_Hagoromo.h"
#include "HGMDebug.h"
#include "HagoromoModule.h"
#include "HGMMath.h"
#include "HGMConstraints.h"
#include "HGMPhysics.h"
#include "HGMCollision.h"
#include "HGMAnimation.h"

#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimStats.h"
#include "Animation/AnimTrace.h"
#include "Algo/Reverse.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"


#pragma region Internal
namespace AnimNodeHagoromoInternal
{
	static TAutoConsoleVariable<int32> CVarShowLinks(TEXT("p.Hagoromo.ShowLinks"), 0, TEXT("Show links.\n"));
	static TAutoConsoleVariable<int32> CVarShowBodyCollider(TEXT("p.Hagoromo.ShowBodyCollider"), 0, TEXT("Show body collider.\n"));
	static TAutoConsoleVariable<int32> CVarShowBoneColliders(TEXT("p.Hagoromo.ShowBoneColliders"), 0, TEXT("Show bone colliders.\n"));
	static TAutoConsoleVariable<int32> CVarShowPlaneColliders(TEXT("p.Hagoromo.ShowPlaneColliders"), 0, TEXT("Show plane colliders.\n"));
	static TAutoConsoleVariable<int32> CVarShowFixedBledns(TEXT("p.Hagoromo.ShowFixedBlends"), 0, TEXT("Show fixed bledns.\n"));
	static TAutoConsoleVariable<int32> CVarShowVerticalStructure(TEXT("p.Hagoromo.ShowVerticalStructure"), 0, TEXT("Show vertical structure.\n"));
	static TAutoConsoleVariable<int32> CVarShowHorizontalStructure(TEXT("p.Hagoromo.ShowHorizontalStructure"), 0, TEXT("Show horizontal structure.\n"));
	static TAutoConsoleVariable<int32> CVarShowStructures(TEXT("p.Hagoromo.ShowStructures"), 0, TEXT("Show horizontal and vertical structures.\n"));
	static TAutoConsoleVariable<int32> CVarShowShearStructures(TEXT("p.Hagoromo.ShowShearStructures"), 0, TEXT("Show shear structures.\n"));
	static TAutoConsoleVariable<int32> CVarShowAnimPoseMovableRadiusConstraint(TEXT("p.Hagoromo.ShowAnimPoseMovableRadiusConstraint"), 0, TEXT("Show anim pose movable radius constraint.\n"));
	static TAutoConsoleVariable<int32> CVarShowAnimPoseLimitAngleConstraint(TEXT("p.Hagoromo.ShowAnimPoseLimitAngleConstraint"), 0, TEXT("Show anim pose limit angle constraint.\n"));
	static TAutoConsoleVariable<int32> CVarShowAnimPosePlanarConstraint(TEXT("p.Hagoromo.ShowAnimPosePlanarConstraint"), 0, TEXT("Show anim pose planar constraint.\n"));
	static TAutoConsoleVariable<int32> CVarShowVelocities(TEXT("p.Hagoromo.ShowVelocities"), 0, TEXT("Show velocities.\n"));
	static TAutoConsoleVariable<int32> CVarShowRelativeLimitAngleConstraint(TEXT("p.Hagoromo.ShowRelativeLimitAngleConstraint"), 0, TEXT("Show relative limit angle constraint.\n"));

	static void Initialize(FAnimNode_Hagoromo* AnimNodeHagoromo, const FBoneContainer& BoneContainer)
	{
		AnimNodeHagoromo->PhysicsContext.bIsFirstUpdate = true;

		AnimNodeHagoromo->Solver->Initialize(BoneContainer, AnimNodeHagoromo->ChainSettings, AnimNodeHagoromo->PhysicsSettings, AnimNodeHagoromo->PhysicsContext);

		FHGMCollisionLibrary::InitializeBodyColliderFromPhysicsAsset(BoneContainer, AnimNodeHagoromo->PhysicsAssetForBodyCollider, AnimNodeHagoromo->BodyCollider);

		if (AnimNodeHagoromo->AdditionalColliderSettings.PlaneColliders.Num() > 0)
		{
			FHGMCollisionLibrary::InitializePlaneColliders(BoneContainer, AnimNodeHagoromo->AdditionalColliderSettings.PlaneColliders, AnimNodeHagoromo->PlaneColliders);
		}
	}
}
#pragma endregion


FAnimNode_Hagoromo::~FAnimNode_Hagoromo()
{
	if (!Solver)
	{
		delete Solver;
		Solver = nullptr;
	}
}


void FAnimNode_Hagoromo::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	Super::Initialize_AnyThread(Context);

	if (!Solver)
	{
		Solver = new FHGMDynamicBoneSolver();
		bShouldInitialize = true;
		PhysicsContext.bIsFirstUpdate = true;
	}
}


void FAnimNode_Hagoromo::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	Super::CacheBones_AnyThread(Context);
}


void FAnimNode_Hagoromo::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(EvaluateSkeletalControl_AnyThread)
	ANIM_MT_SCOPE_CYCLE_COUNTER_VERBOSE(Hagoromo, !IsInGameThread());

	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	if (!Solver)
	{
		HGM_LOG(Error, TEXT("Solver was nullptr."));
		return;
	}

	if (bShouldInitialize)
	{
		AnimNodeHagoromoInternal::Initialize(this, BoneContainer);
		bShouldInitialize = false;
	}

	if (!Solver->HasInitialized())
	{
		HGM_LOG(Warning, TEXT("Solver has not initialized."));
		return;
	}

	FHGMCollisionLibrary::UpdateBodyCollider(Output, PhysicsContext, BodyCollider, PrevBodyCollider);

	if (AdditionalColliderSettings.PlaneColliders.Num() > 0)
	{
		FHGMCollisionLibrary::UpdatePlaneColliders(Output, AdditionalColliderSettings.PlaneColliders, PlaneColliders);
	}

	Solver->PreSimulate(Output, PhysicsSettings, PhysicsContext, AnimationCurveNumber);

	Solver->Simulate(Output, PhysicsContext, BodyCollider, PrevBodyCollider, PlaneColliders);

	Solver->OutputSimulateResult(PhysicsContext, Output, OutBoneTransforms);

#if ENABLE_ANIM_DRAW_DEBUG
	AnimDrawDebugHagoromo(Output);
#endif

	PhysicsContext.bIsFirstUpdate = false;
}


bool FAnimNode_Hagoromo::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return Solver != nullptr;
}


#if ENABLE_ANIM_DRAW_DEBUG
void FAnimNode_Hagoromo::AnimDrawDebugHagoromo(FComponentSpacePoseContext& Output)
{
	if (!Solver)
	{
		return;
	}

	// Show body collider :
	if (const int32 ShowBodyCollider = AnimNodeHagoromoInternal::CVarShowBodyCollider.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowBodyCollider == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawBodyCollider(Output, PhysicsContext, BodyCollider, DepthPriority);
	}

	// Show bone colliders :
	if (const int32 ShowBoneColliders = AnimNodeHagoromoInternal::CVarShowBoneColliders.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowBoneColliders == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawBoneColliders(Output, Solver, DepthPriority);
	}

	// Show plane colliders :
	if (const int32 ShowPlaneColliders = AnimNodeHagoromoInternal::CVarShowPlaneColliders.GetValueOnAnyThread())
	{
		FHGMDebugLibrary::DrawPlaneColliders(Output, AdditionalColliderSettings.PlaneColliders);
	}

	// Show fixed blends :
	if (const int32 ShowFixedBledns = AnimNodeHagoromoInternal::CVarShowFixedBledns.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowFixedBledns == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawFixedBlends(Output, Solver, DepthPriority);
	}

	// Show vertical structure :
	if (const int32 ShowVerticalStructure = AnimNodeHagoromoInternal::CVarShowVerticalStructure.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowVerticalStructure == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawVerticalStructure(Output, Solver, DepthPriority);
	}

	// Show horizontal structure :
	if (const int32 ShowHorizontalStructure = AnimNodeHagoromoInternal::CVarShowHorizontalStructure.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowHorizontalStructure == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawHorizontalStructure(Output, Solver, DepthPriority);
	}

	// Show vertical and horizontal structures :
	if (const int32 ShowStructures = AnimNodeHagoromoInternal::CVarShowStructures.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowStructures == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawVerticalStructure(Output, Solver, DepthPriority);
		FHGMDebugLibrary::DrawHorizontalStructure(Output, Solver, DepthPriority);
	}

	// Show shear structures :
	if (const int32 ShowShearStructures = AnimNodeHagoromoInternal::CVarShowShearStructures.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowShearStructures == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawShear(Output, Solver, PhysicsContext.PhysicsSettings.bLoopHorizontalStructure, DepthPriority);
	}

	// Show animation pose movable radius constraint :
	if (const int32 ShowAnimPoseMovableRadiusConstrain = AnimNodeHagoromoInternal::CVarShowAnimPoseMovableRadiusConstraint.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowAnimPoseMovableRadiusConstrain == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawAnimPoseMovableRadiusConstraint(Output, PhysicsContext.PhysicsSettings, Solver, DepthPriority);
	}

	// Show animation pose limit angle constraint :
	if (const int32 ShowAnimPoseLimitAngleConstraint = AnimNodeHagoromoInternal::CVarShowAnimPoseLimitAngleConstraint.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowAnimPoseLimitAngleConstraint == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawAnimPoseLimitAngleConstraint(Output, PhysicsContext.PhysicsSettings, Solver, DepthPriority);
	}

	// Show animation pose planar constraint :
	if (const int32 ShowAnimPoseLimitAngleConstraint = AnimNodeHagoromoInternal::CVarShowAnimPosePlanarConstraint.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowAnimPoseLimitAngleConstraint == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawAnimPosePlanarConstraint(Output, PhysicsContext.PhysicsSettings, Solver, DepthPriority);
	}

	// Show velocities :
	if (const int32 ShowVelocities = AnimNodeHagoromoInternal::CVarShowVelocities.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowVelocities == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawVelocities(Output, PhysicsContext, Solver, DepthPriority);
	}

	// Show relative limit angle constraint :
	if (const int32 ShowRelativeLimitAngleConstraint = AnimNodeHagoromoInternal::CVarShowRelativeLimitAngleConstraint.GetValueOnAnyThread())
	{
		const ESceneDepthPriorityGroup DepthPriority = ShowRelativeLimitAngleConstraint == 1 ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
		FHGMDebugLibrary::DrawRelativeLimitAngleConstraint(Output, PhysicsContext.PhysicsSettings, Solver, DepthPriority);
	}
}
#endif
