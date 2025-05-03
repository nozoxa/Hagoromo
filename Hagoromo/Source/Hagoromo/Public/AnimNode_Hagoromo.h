// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"
#include "HGMSolvers.h"
#include "HGMCollision.h"
#include "HGMDebug.h"

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BoneContainer.h"
#include "BonePose.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Animation/AnimInstanceProxy.h"

#include "AnimNode_Hagoromo.generated.h"


USTRUCT()
struct HAGOROMO_API FAnimNode_Hagoromo : public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY()

public:
	FAnimNode_Hagoromo() = default;
	~FAnimNode_Hagoromo();

	// FAnimNode_SkeletalControlBase interface
	void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

	FORCENOINLINE void Initialize()
	{
		bShouldInitialize = true;
	}

	/**
	* 物理シミュレーションの対象にするチェーンの設定です。
	*
	* This is setting of chain to be included in physics simulation.
	*/
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Hagoromo Chain Settings", TitleProperty = "RootBone", DisplayPriority = "1"))
	TArray<FHGMChainSetting> ChainSettings {};

	/**
	* 物理挙動を調整するためのパラメータです。
	*
	* Parameters to adjust physical behavior.
	*/
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Hagoromo General Settings", DisplayPriority = "2"))
	FHGMPhysicsSettings PhysicsSettings {};

	/**
	* 物理シミュレーション対象のボーンと衝突判定を行うコライダを物理アセットで指定します。
	* 使用できるコライダの形状はスフィアとカプセルです。
	* 当該物理アセットによって髪や衣服が身体に貫通することを防ぎます。
	*
	* Specify bones to be simulated and the collider to determine collision in physics asset.
	* Available forms of colliders are spheres and capsules.
	* Prevents hair and clothing from penetrating body due to physical asset in question.
	*/
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Hagoromo Body Collider Settings", DisplayPriority="3"))
	TObjectPtr<UPhysicsAsset> PhysicsAssetForBodyCollider = nullptr;

	/**
	* ボディコライダ以外の追加のコライダ設定です。
	*
	* Additional collider settings other than body collider.
	*/
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Hagoromo Additional Collider Settings", DisplayPriority="4"))
	FHGMAdditionalColliderSettings AdditionalColliderSettings {};

	/**
	* ノードに適用するアニメーションカーブを識別するための番号です。
	* 例えば Hagoromo_Alpha_AnimationCurveNumber のようなカーブが当該ノードに適用されるようになります。
	*
	* Number to identify animation curve to be applied to node.
	* For example, curve like Hagoromo_Alpha_AnimationCurveNumber will be applied to node in question.
	*/
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Hagoromo Animation Curve Number", UIMin = 0, ClampMin = 0, DisplayPriority = "0"))
	int32 AnimationCurveNumber = 0;

	FHGMPhysicsContext PhysicsContext {};

	FHGMDynamicBoneSolver* Solver = nullptr;

	FHGMBodyCollider BodyCollider {};
	FHGMBodyCollider PrevBodyCollider {};

	TArray<FHGMSIMDPlaneCollider> PlaneColliders {};

private:
	bool bShouldInitialize = true;

#if ENABLE_ANIM_DRAW_DEBUG
	void AnimDrawDebugHagoromo(FComponentSpacePoseContext& Output);
#endif
};
