// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"

#include "AnimationRuntime.h"


namespace HGMAnimationConstants
{
	// Note: BoneIndex is not set. It defaults to INDEX_NONE(-1).
	const FName DummyBoneName { TEXT("HagoromoDummyBone") };
	const FBoneReference DummyBone { DummyBoneName };

	const FName VirtualBoneName { TEXT("HagoromoVirtualBone") };
	const FBoneReference VirtualBone { VirtualBoneName };

	const FName LeafBoneName { TEXT("HagoromoLeafBone") };
	const FBoneReference LeafBone { LeafBoneName };
}


struct FHGMAnimationLibrary
{
	// Returns Bind Pose of component space.
	FORCEINLINE static FHGMTransform GetComponentSpaceRefTransform(const FBoneContainer& BoneContainer, const FCompactPoseBoneIndex& CompactPoseBoneIndex)
	{
		return FAnimationRuntime::GetComponentSpaceRefPose(CompactPoseBoneIndex, BoneContainer);
	}

	FORCEINLINE static FHGMTransform GetComponentSpaceRefTransform(const FBoneContainer& BoneContainer, int32 BoneIndex)
	{
		return FHGMAnimationLibrary::GetComponentSpaceRefTransform(BoneContainer, FCompactPoseBoneIndex(BoneIndex));
	}


	// Get parent index from index of bone.
	FORCEINLINE static int32 GetParentBoneIndex(const FBoneContainer& BoneContainer, int32 BoneIndex)
	{
		return  BoneContainer.GetParentBoneIndex(BoneIndex);
	}

	FORCEINLINE static FCompactPoseBoneIndex GetParentBoneIndex(const FBoneContainer& BoneContainer, const FCompactPoseBoneIndex& CompactPoseBoneIndex)
	{
		return  BoneContainer.GetParentBoneIndex(CompactPoseBoneIndex);
	}


	// Get name from index of bone.
	FORCEINLINE static FName GetBoneName(const FBoneContainer& BoneContainer, int32 BoneIndex)
	{
		const FReferenceSkeleton& ReferenceSkeleton = BoneContainer.GetReferenceSkeleton();
		return ReferenceSkeleton.GetBoneName(BoneIndex);
	}

	FORCEINLINE static FName GetBoneName(const FBoneContainer& BoneContainer, const FCompactPoseBoneIndex& CompactPoseBoneIndex)
	{
		return FHGMAnimationLibrary::GetBoneName(BoneContainer, CompactPoseBoneIndex.GetInt());
	}


	// These bones do not affect collision judgments or animation poses.
	// Basically, it is inserted to simplify calculations with SIMD.
	FORCEINLINE static bool IsDummyBone(const FBoneReference& BoneReference)
	{
		return BoneReference.BoneName == HGMAnimationConstants::DummyBoneName;
	}

	// Bone used for divided function.
	// This affects collision determination and output of animation poses.
	FORCEINLINE static bool IsVirtualBone(const FBoneReference& BoneReference)
	{
		return BoneReference.BoneName == HGMAnimationConstants::VirtualBoneName;
	}

	// It's basically just dummy bone at end of chain.
	FORCEINLINE static bool IsDummyLeafBone(const FBoneReference& BoneReference)
	{
		return BoneReference.BoneName == HGMAnimationConstants::LeafBoneName;
	}

	FORCEINLINE static bool IsValidBone(const FBoneReference& BoneReference)
	{
		return BoneReference.IsValidToEvaluate();
	}
};
