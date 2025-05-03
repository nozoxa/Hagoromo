// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"

#include "Animation/AnimNodeBase.h"
#include "Animation/AnimInstanceProxy.h"

struct FHGMDynamicBoneSolver;
struct FHGMBodyCollider;


// ---------------------------------------------------------------------------------------
// DebugLibrary
// ---------------------------------------------------------------------------------------
struct FHGMDebugLibrary
{
	FORCEINLINE static FString ToString(const FHGMReal& A)
	{
#if HGM_USE_FLOAT32
		return FString::Printf(TEXT("%3.3f"), A);
#else
		return FString::Printf(TEXT("%3.3lf"), A);
#endif
	}

	FORCEINLINE static FString ToString(const FHGMReal& A, const FHGMReal& B, const FHGMReal& C, const FHGMReal& D)
	{
#if HGM_USE_FLOAT32
		return FString::Printf(TEXT("A=%3.3f B=%3.3f C=%3.3f D=%3.3f"), A, B, C, D);
#else
		return FString::Printf(TEXT("A=%3.3lf B=%3.3lf C=%3.3lf D=%3.3lf"), A, B, C, D);
#endif
	}

	FORCEINLINE static FString ToString(const TStaticArray<FHGMReal, 4> Values)
	{
		return FHGMDebugLibrary::ToString(Values[0], Values[1], Values[2], Values[3]);
	}

	FORCEINLINE static FString ToString(const FHGMVector3& V)
	{
		return V.ToString();
	}

	FORCEINLINE static FString ToString(const FHGMVector3& V0, const FHGMVector3& V1, const FHGMVector3& V2, const FHGMVector3& V3)
	{
		return TEXT("\nV0: ") + V0.ToString() + TEXT("\nV1: ") + V1.ToString() + TEXT("\nV2: ") + V2.ToString() + TEXT("\nV3: ") + V3.ToString();
	}

	FORCEINLINE static FString ToString(const TStaticArray<FHGMVector3, 4> Vectors)
	{
		return FHGMDebugLibrary::ToString(Vectors[0], Vectors[1], Vectors[2], Vectors[3]);
	}

	static FString ToString(const FHGMSIMDReal& A);

	static FString ToString(const FHGMSIMDVector3& V);

#if ENABLE_ANIM_DRAW_DEBUG
	static void DrawSphere(FComponentSpacePoseContext& PoseContext, const FHGMSIMDVector3& sCenter, const FHGMSIMDReal& sRadius, int32 Segments, FColor Color, ESceneDepthPriorityGroup DepthPriority = SDPG_World, FHGMReal Thickness = 0.0f);

	static void DrawSegment(FComponentSpacePoseContext& PoseContext, const FHGMSIMDVector3& sStart, const FHGMSIMDVector3& sEnd, const FColor& Color);

	static void DrawBodyCollider(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsContext& PhysicsContext, const FHGMBodyCollider& BodyCollider, ESceneDepthPriorityGroup DepthPriority);

	static void DrawBoneColliders(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);

	static void DrawPlaneColliders(FComponentSpacePoseContext& PoseContext, TConstArrayView<FHGMPlaneCollider> PlaneColliders);

	static void DrawFixedBlends(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);

	static void DrawVerticalStructure(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);

	static void DrawHorizontalStructure(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);

	static void DrawShear(FComponentSpacePoseContext& PoseContext, FHGMDynamicBoneSolver* Solver, bool bLoopHorizontalStructure, ESceneDepthPriorityGroup DepthPriority);

	static void DrawAnimPoseMovableRadiusConstraint(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsSettings& PhysicsSettings, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);

	static void DrawAnimPoseLimitAngleConstraint(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsSettings& PhysicsSettings, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);

	static void DrawAnimPosePlanarConstraint(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsSettings& PhysicsSettings, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);

	static void DrawRelativeLimitAngleConstraint(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsSettings& PhysicsSettings, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);

	static void DrawVelocities(FComponentSpacePoseContext& PoseContext, const FHGMPhysicsContext& PhysicsContext, FHGMDynamicBoneSolver* Solver, ESceneDepthPriorityGroup DepthPriority);
#endif
};
