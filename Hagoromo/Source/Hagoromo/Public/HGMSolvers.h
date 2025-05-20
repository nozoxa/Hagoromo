// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"
#include "HGMConstraints.h"
#include "HGMCollision.h"

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "BonePose.h"

#include "HGMSolvers.generated.h"

struct FHGMBodyCollider;
struct FHGMSIMDStructure;

struct FComponentSpacePoseContext;


// Plane of multiple of 4 x 4.
// This is to simplify SIMD calculations.
struct FHGMSimulationPlane
{
	// Number of vertical bones.
	// Note: Dummy bones are included.
	int32 UnpackedVerticalBoneNum = 0;
	int32 PackedVerticalBoneNum = 0;

	// Number of horizontal bones.
	// Note: Dummy bones are included.
	int32 UnpackedHorizontalBoneNum = 0;
	int32 PackedHorizontalBoneNum = 0;

	// FAnimNode_Hagoromo::ChainSettings.Num().
	// Note: Dummy bones are not included.
	int32 ActualUnpackedHorizontalBoneNum = 0;
};


USTRUCT()
struct FHGMAdditionalColliderSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "")
	TArray<FHGMPlaneCollider> PlaneColliders {};
};


USTRUCT()
struct FHGMBoneSphereColliderSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0))
	double Radius = 2.0;

	/**
	* ボーン単位で Radius を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は Radius に乗算する値です。
	*
	* This curve is used to adjust Radius on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by Radius.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};
};


USTRUCT()
struct FHGMFrictionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double Friction = 0.15;

	/**
	* ボーン単位で Friction を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は Friction に乗算する値です。
	*
	* This curve is used to adjust Friction on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by Friction.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};
};


USTRUCT()
struct FHGMWorldVelocityDampingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double Damping = 0.6;

	/**
	* ボーン単位で WorldVelocityDamping を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は WorldVelocityDamping に乗算する値です。
	*
	* This curve is used to adjust WorldVelocityDamping on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by WorldVelocityDamping.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};
};


USTRUCT()
struct FHGMWorldAngularVelocityDampingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double Damping = 0.65;

	/**
	* ボーン単位で WorldAngularVelocityDamping を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は WorldAngularVelocityDamping に乗算する値です。
	*
	* This curve is used to adjust WorldAngularVelocityDamping on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by WorldAngularVelocityDamping.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};
};


USTRUCT()
struct FHGMSimulationVelocityDampingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double Damping = 1.0;

	/**
	* ボーン単位で SimulationVelocityDamping を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は SimulationVelocityDamping に乗算する値です。
	*
	* This curve is used to adjust SimulationVelocityDamping on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by SimulationVelocityDamping.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};
};


USTRUCT()
struct FHGMSimulationAngularVelocityDampingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double Damping = 1.0;

	/**
	* ボーン単位で SimulationAngularVelocityDamping を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は SimulationAngularVelocityDamping に乗算する値です。
	*
	* This curve is used to adjust SimulationAngularVelocityDamping on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by SimulationAngularVelocityDamping.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};
};


USTRUCT()
struct FHGMMasterDampingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double MasterDamping = 0.0;

	/**
	* ボーン単位で MasterDamping を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は MasterDamping に乗算する値です。
	*
	* Curves for adjusting MasterDamping on per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by MasterDamping.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};
};


USTRUCT()
struct FHGMAnimPoseConstraintMovableRadiusSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0))
	double Radius = 0.0;

	/**
	* ボーン単位で Radius を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は Radius に乗算する値です。
	*
	* Curves for adjusting Radius on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by Radius.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};

	/**
	* 値が 0.0 に近づくほど可動域にすぐに戻ります。
	*
	* The closer value is to 0.0, sooner it returns to limiting range.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double Damping = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve DampingMultiplierCurve {};
};

struct FHGMSIMDAnimPoseConstraintMovableRadius
{
	FHGMSIMDReal sRadius {};
	FHGMSIMDReal sDamping {};
};


USTRUCT()
struct FHGMAnimPoseConstraintLimitAngleSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (ForceUnits = "Degrees", UIMin = 0.0, ClampMin = 0.0))
	double Angle = 0.0;

	/**
	* ボーン単位で Angle を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は Angle に乗算する値です。
	*
	* Curves for adjusting Angle on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by Angle.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};

	/**
	* 値が 0.0 に近づくほど制限角度にすぐに戻ります。
	*
	* The closer value is to 0.0, sooner it returns to limiting angle.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double Damping = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve DampingMultiplierCurve {};
};

struct FHGMSIMDAnimPoseConstraintLimitAngle
{
	FHGMSIMDReal sAngle {};
	FHGMSIMDReal sDamping {};
};


USTRUCT()
struct FHGMRelativeLimitAngleConstraintSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (ForceUnits = "Degrees", UIMin = 0.0, ClampMin = 0.0))
	double Angle = 0.0;

	/**
	* ボーン単位で Angle を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は Angle に乗算する値です。
	*
	* Curves for adjusting Angle on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by Angle.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};

	/**
	* 値が 0.0 に近づくほど制限角度にすぐに戻ります。
	*
	* The closer value is to 0.0, sooner it returns to limiting angle.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double Damping = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "")
	FRuntimeFloatCurve DampingMultiplierCurve {};
};

struct FHGMSIMDRelativeLimitAngle
{
	FHGMSIMDReal sAngle {};
	FHGMSIMDReal sDamping {};
};


USTRUCT()
struct FHGMMassSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.1, ClampMin = 0.1))
	double Mass = 1.0;

	/**
	* ボーン単位で Mass を調整するためのカーブです。
	* 横軸はボーンの長さの比率 0.0 ～ 1.0 に対応しています。
	* 縦軸は Mass に乗算する値です。
	*
	* Curves for adjusting Mass on a per-bone basis.
	* Horizontal axis corresponds to bone length ratio 0.0 to 1.0.
	* Vertical axis is value to be multiplied by Mass.
	*/
	UPROPERTY(EditAnywhere, Category = "")
	FRuntimeFloatCurve MultiplierCurve {};
};


USTRUCT()
struct FHGMGravitySettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "")
	double Gravity = 9.8;

	/**
	* 任意のボーンを基準とした重力に切り替えます。
	*
	* Switch to gravity with respect to arbitrary bone.
	*/
	UPROPERTY(EditAnywhere, Category = "")
	bool bUseBoneSpaceGravity = false;

	/**
	* 重力の方向を決めるボーンを指定します。
	*
	* Specifies bones that determine direction of gravity.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseBoneSpaceGravity", EditConditionHides))
	FBoneReference DrivingBone {};

	/**
	* ボーンの軸を指定します。これが重力の方向になります。
	* 負の向きに重力をかけたい場合は Gravity をマイナスにしてください。
	*
	* Specifies axis of bone.　This is direction of gravity.
	* If you want to apply gravity in negative direction, set Gravity to minus.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseBoneSpaceGravity", EditConditionHides))
	TEnumAsByte<EAxis::Type> Axis = EAxis::X;
};


USTRUCT()
struct FHGMChainSetting
{
	GENERATED_BODY()

	/**
	* 物理シミュレーションを適用するチェーンのルートボーンを選択します。
	*
	* Select root bone of chain to which physics simulation will be applied.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	FBoneReference RootBone {};

	/**
	* 物理シミュレーションの対象から除外したいボーンを指定します。
	* 選択されたボーンとそれ以下のボーンに物理シミュレーションが適用されなくなります。
	*
	* If there are bones in chain that you want to exclude from simulation, specify here.
	* Physics simulation is no longer applied to selected bone and bones below it.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (TitleProperty = "BoneName", EditCondition = "bUseExcludeBones"))
	TArray<FBoneReference> ExcludeBones {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bUseExcludeBones = false;

	/**
	* 根元のボーンをアニメーションポーズに固定するかどうかを選択します。
	* 固定しない場合は重力によって落下し続ける挙動になります。
	*
	* Select whether to anchor root bone to animation pose.
	* If not fixed, it will continue to fall due to gravity.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	bool bBeginBonePositionFixed = true;

	/**
	* ボーンの動きを拘束する平面の法線を指定します。
	* 注意点 : Hagoromo General Settings の bUseAnimPoseConstraint と bUseAnimPoseConstraintPlanar を ON にしないと動作しません。
	*
	* Specifies normal of plane that constrains movement of bone.
	* Note : It will not work unless bUsePlanarConstraint in Hagoromo General Settings is ON.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	TEnumAsByte<EAxis::Type> AnimPoseConstraintPlanarAxis = EAxis::None;

	/**
	* チェーンに含まれるボーン単位で親ボーンからの相対角度拘束を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の RelativeLimitAngleConstraintSettings を上書きします。
	* 注意点: Hagoromo General Settings の bUseRelativeLimitAngleConstraint を有効しないと機能しません。
	*
	* Parameter for adjusting angular constraint relative to parent bone for each bone in chain.
	* This parameter overrides RelativeLimitAngleConstraintSettings in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override Relative Limit Angle Each Bone", EditCondition = "bOverrideRelativeLimitAngleEachBone"))
	FHGMRelativeLimitAngleConstraintSettings RelativeLimitAngleConstraintSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideRelativeLimitAngleEachBone = false;

	/**
	* チェーンに含まれるボーン単位でアニメーションポーズへの距離拘束を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の AnimPoseConstraintMovableRadius を上書きします。
	* 注意点: Hagoromo General Settings の bUseAnimPoseConstraint と bUseAnimPoseConstraintMovableRadius を有効しないと機能しません。
	*
	* Parameters for adjusting distance constraints on animation pose for each bone in chain.
	* This parameter overrides AnimPoseConstraintMovableRadius in Hagoromo General Settings.
	* Note: bUseAnimPoseConstraint and bUseAnimPoseConstraintMovableRadius in Hagoromo General Settings must be enabled for this to work.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override Anim Pose Constraint Movable Radius Each Bone", EditCondition = "bOverrideAnimPoseConstraintMovableRadiusEachBone"))
	FHGMAnimPoseConstraintMovableRadiusSettings AnimPoseConstraintMovableRadiusSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideAnimPoseConstraintMovableRadiusEachBone = false;

	/**
	* チェーンに含まれるボーン単位でアニメーションポーズへの角度拘束を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の AnimPoseConstraintMovableRadius を上書きします。
	* 注意点: Hagoromo General Settings の bUseAnimPoseConstraint と bUseAnimPoseConstraintMovableRadius を有効しないと機能しません。
	*
	* Parameters for adjusting angle constraints on animation pose for each bone in chain.
	* This parameter overrides AnimPoseConstraintMovableRadius in Hagoromo General Settings.
	* Note: bUseAnimPoseConstraint and bUseAnimPoseConstraintMovableRadius in Hagoromo General Settings must be enabled for this to work.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override Anim Pose Constraint Limit Angle Each Bone", EditCondition = "bOverrideAnimPoseConstraintLimitAngleEachBone"))
	FHGMAnimPoseConstraintLimitAngleSettings AnimPoseConstraintLimitAngleSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideAnimPoseConstraintLimitAngleEachBone = false;

	/**
	* チェーンに含まれるボーン単位でスフィアコライダの半径を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の BoneSphereColliderRadius を上書きします。
	*
	* This parameter is used to adjust radius of the sphere collider in units of bones in chain.
	* This parameter overrides BoneSphereColliderRadius in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override BoneSphereColliderRadius Each Bone", EditCondition = "bOverrideBoneSphereColliderRadiusEachBone"))
	FHGMBoneSphereColliderSettings BoneSphereColliderSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideBoneSphereColliderRadiusEachBone = false;

	/**
	* チェーンに含まれるボーン単位で摩擦を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の Friction を上書きします。
	*
	* This parameter is used to adjust radius of the sphere collider in units of bones in chain.
	* This parameter overrides Friction in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override Friction Each Bone", EditCondition = "bOverrideFrictionEachBone"))
	FHGMFrictionSettings FrictionSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideFrictionEachBone = false;

	/**
	* チェーンに含まれるボーン単位でアクターの移動によって発生する力を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の WorldVelocityDamping を上書きします。
	*
	* This parameter is used to adjust Force generated by movement of actor in units of bones in chain.
	* This parameter overrides WorldVelocityDamping in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override World Velocity Damping Each Bone", EditCondition = "bOverrideWorldVelocityDampingEachBone"))
	FHGMWorldVelocityDampingSettings WorldVelocityDampingSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideWorldVelocityDampingEachBone = false;

	/**
	* チェーンに含まれるボーン単位でアクターの回転によって発生する力を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の WorldAngularVelocity を上書きします。
	*
	* This parameter is used to adjust Force generated by rotation of actor in units of bones in chain.
	* This parameter overrides WorldAngularVelocity in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override World Angular Velocity Damping Each Bone", EditCondition = "bOverrideWorldAngularVelocityDampingEachBone"))
	FHGMWorldAngularVelocityDampingSettings WorldAngularVelocityDampingSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideWorldAngularVelocityDampingEachBone = false;

	/**
	* チェーンに含まれるボーン単位で SimulationRootBone の移動によって発生する力を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の SimulationVelocityDamping を上書きします。
	*
	* This parameter is used to adjust Force generated by movement of SimulationRootBone in units of bones in chain.
	* This parameter overrides SimulationVelocityDamping in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override Simulation Velocity Damping Each Bone", EditCondition = "bOverrideSimulationVelocityDampingEachBone"))
	FHGMSimulationVelocityDampingSettings SimulationVelocityDampingSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideSimulationVelocityDampingEachBone = false;

	/**
	* チェーンに含まれるボーン単位で SimulationRootBone の回転によって発生する力を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の SimulationAngularVelocity を上書きします。
	*
	* This parameter is used to adjust Force generated by rotation of SimulationRootBone in units of bones in chain.
	* This parameter overrides SimulationAngularVelocity in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override Simulation Angular Velocity Damping Each Bone", EditCondition = "bOverrideSimulationAngularVelocityDampingEachBone"))
	FHGMSimulationAngularVelocityDampingSettings SimulationAngularVelocityDampingSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideSimulationAngularVelocityDampingEachBone = false;

	/**
	* チェーンに含まれるボーン単位で慣性を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の MasterDamping を上書きします。
	*
	* This parameter is used to adjust MasterDamping of each bone unit in chain.
	* This parameter overrides MasterDamping in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override MasterDamping Each Bone", EditCondition = "bOverrideMasterDampingEachBone"))
	FHGMMasterDampingSettings MasterDampingSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideMasterDampingEachBone = false;

	/**
	* チェーンに含まれるボーン単位で質量を調整するためのパラメータです。
	* 当該パラメータは Hagoromo General Settings の Mass を上書きします。
	*
	* This parameter is used to adjust Mass of each bone unit in chain.
	* This parameter overrides Mass in Hagoromo General Settings.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Override Mass Each Bone", EditCondition = "bOverrideMassEachBone"))
	FHGMMassSettings MassSettings {};
	UPROPERTY(EditAnywhere, Category = "", meta = (InlineEditConditionToggle))
	bool bOverrideMassEachBone = false;
};


USTRUCT()
struct FHGMPhysicsSettings
{
	GENERATED_BODY()

	/**
	* シミュレーション対象の剛性を 0.0 ～ 1.0 で指定します。
	* あくまでも目安であるため、最終的に見た目に満足できれば値はいくつでも問題ありません。
	*   - [コンクリート] 1.0
	*   - [木材] 0.24
	*   - [レザー] 0.004
	*   - [腱] 0.002
	*   - [ゴム] 0.00004
	*   - [筋] 0.0000002
	*   - [脂肪] 0.00000004
	*
	* 参考: https://blog.mmacklin.com/?page_id=8450
	*
	* Specify stiffness from 0.0 to 1.0.
	* Since this is only guide, any number of values is acceptable as long as you are satisfied with final appearance.
	*   - [Concrete] 1.0
	*   - [Wood] 0.24
	*   - [Leather] 0.004
	*   - [Tendon] 0.002
	*   - [Rubber] 0.00004
	*   - [Muscle] 0.0000002
	*   - [Fat] 0.00000004
	*
	* See https://blog.mmacklin.com/?page_id=8450 .
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double StructureStiffness = 0.004;

	/**
	* チェーンの長さが変化しないように強制する制約を有効にします。
	*
	* Enable constraints that force chain length to remain unchanged.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	bool bUseRigidVerticalStructureConstraint = false;

	/**
	* チェーンの横方向の距離制約を有効にします。
	* スカートやマントなどに利用すると布が横に伸びすぎることがなくなり、見映えが良くなります。
	*
	* Enable horizontal distance constraints on chain.
	* When used for skirts and cloaks, it prevents cloth from stretching too much and improves appearance.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	bool bUseHorizontalStructuralConstraint = false;

	/**
	* 最初と最後の2つのチェーンの間に水平方向の距離制約を追加します。
	* スカートのようにチェーンが水平方向にループしている場合は有効にしてください。
	*
	* Add horizontal distance constraints between first and last two chains.
	* Enable if chain loops horizontally, as in skirt.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseHorizontalStructuralConstraint", EditConditionHides))
	bool bLoopHorizontalStructure = false;

	/**
	* 布の縦方向の曲がり具合を調節する制約を有効にします。
	*
	* Enables constraints to adjust vertical bend of the cloth.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "!bUseRigidVerticalStructureConstraint", EditConditionHides))
	bool bUseVerticalBendConstraint = false;

	/**
	* 縦方向の曲げ制約の剛性を 0.0 ～ 1.0 で指定します。
	* 値が大きいほど布が縦方向に曲がりにくくなります。
	*
	* Specify stiffness of vertical bending constraint from 0.0 to 1.0.
	* Larger value, less likely fabric is to bend lengthwise.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0, EditCondition = "bUseVerticalBendConstraint", EditConditionHides))
	double VerticalBendStiffness = 0.004;

	/**
	* 布の横方向の曲がり具合を調節する制約を有効にします。
	*
	* Enables constraints to adjust horizontal bend of the cloth.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	bool bUseHorizontalBendConstraint = false;

	/**
	* 横方向の曲げ制約の剛性を 0.0 ～ 1.0 で指定します。
	* 値が大きいほど布が横方向に曲がりにくくなります。
	*
	* Specify stiffness of horizontal bending constraint from 0.0 to 1.0.
	* Larger value, less likely the fabric is to bend laterally.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0, EditCondition = "bUseHorizontalBendConstraint", EditConditionHides))
	double HorizontalBendStiffness = 0.004;

	/**
	* 布のねじれ具合を調節する制約を有効にします。
	*
	* Enable constraints that adjust twist of cloth.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	bool bUseShearConstraint = false;

	/**
	* せん断制約の剛性を 0.0 ～ 1.0 で指定します。
	* 値が大きいほど布が歪みにくくなります。
	*
	* Specify stiffness of the shear constraint from 0.0 to 1.0.
	* Larger value, less likely cloth is to twist.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0, EditCondition = "bUseShearConstraint", EditConditionHides))
	double ShearStiffness = 0.004;

	/**
	* アニメーションポーズへの距離制約を有効にします。
	* アニメーションポーズ( 元の形状 )を維持しながら物理効果を乗せたい場合に利用してください。
	*
	* Enable distance constraints on animation poses.
	* Use this function when you want to add physics effects while maintaining animation pose (original shape).
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	bool bUseAnimPoseConstraint = false;

	/**
	* アニメーションポーズへの距離拘束を有効にします。
	* これはアニメーションポーズを維持しながら物理を適用したい場合に役立ちます。
	* Radius はアニメーションポーズを原点とした可動域です。
	* MultiplierCurve は Radius に乗算するカーブです。
	* カーブの横軸はボーンの長さの比率 0.0 ～ 1.0 を表しています。
	*
	* Enable distance constraints on animation poses.
	* This is useful if you want to apply physics while maintaining animated poses.
	* Radius is range of motion that originates from animated pose.
	* MultiplierCurve is curve that multiplies Radius.
	* Horizontal axis of curve represents ratio 0.0 to 1.0 of length of bone.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseAnimPoseConstraint", EditConditionHides))
	bool bUseAnimPoseConstraintMovableRadius = false;
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Anim Pose Constraint Movable Radius", EditCondition = "bUseAnimPoseConstraintMovableRadius", EditConditionHides))
	FHGMAnimPoseConstraintMovableRadiusSettings AnimPoseConstraintMovableRadiusSettings {};

	/**
	* アニメーションポーズへの角度拘束を有効にします。
	* これはアニメーションポーズを維持しながら物理を適用したい場合に役立ちます。
	* Angle はアニメーションポーズから相対的に曲げられる最大角度です。
	* MultiplierCurve は Angle に乗算するカーブです。
	* カーブの横軸はボーンの長さの比率 0.0 ～ 1.0 を表しています。
	*
	* Enable angle constraints on animation poses.
	* This is useful if you want to apply physics while maintaining animated poses.
	* Angle is maximum angle that can be bent relative to animation pose
	* MultiplierCurve is curve that multiplies Angle.
	* Horizontal axis of curve represents ratio 0.0 to 1.0 of length of bone.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseAnimPoseConstraint", EditConditionHides))
	bool bUseAnimPoseConstraintLimitAngle = false;
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Anim Pose Constraint Limit Angle", EditCondition = "bUseAnimPoseConstraintLimitAngle", EditConditionHides))
	FHGMAnimPoseConstraintLimitAngleSettings AnimPoseConstraintLimitAngleSettings {};

	/**
	* ボーンの動きをボーンの任意の軸を法線とする平面へ固定する制約を有効にします。
	* 軸の設定については Hagoromo Chain Settings で可能です。
	*
	* Enables constraints that lock motion of bone to plane normal to arbitrary axis of bone.
	* This is useful for adjusting tail and band simulations.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseAnimPoseConstraint", EditConditionHides))
	bool bUseAnimPoseConstraintPlanar = false;

	/**
	* 親ボーンからの相対的な角度制約を有効にします。
	* これは尻尾や帯のシミュレーションの調整に有効です。
	*
	* Enable angular constraints relative to the parent bone.
	* This is useful for adjusting tail and band simulations.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	bool bUseRelativeLimitAngleConstraint = false;

	/**
	* 親ボーンを基準とした相対角度拘束を指定します。
	* 尻尾や帯などのように親ボーンから徐々に角度が付いてほしいケースで利用します。
	*
	* Specifies an angular constraint relative to parent bone.
	* Used in cases where you want gradual angle from parent bone, such as tail or band.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Relative Limit Angle", EditCondition = "bUseRelativeLimitAngleConstraint", EditConditionHides))
	FHGMRelativeLimitAngleConstraintSettings RelativeLimitAngleConstraintSettings {};

	/**
	* 物理シミュレーション対象のボーンにアタッチされるスフィアコライダの半径です。
	*
	* Sphere collider settings attached to the bones to be simulated.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Bone Sphere Collider Radius"))
	FHGMBoneSphereColliderSettings BoneSphereColliderSettings {};

	/**
	* コライダの衝突解決の結果をどれだけ反映するかをコントロールします。
	* 値が 1.0 の場合は衝突による貫通が直ぐに解消されますが、貫通 → 解消を高速に繰り返すことによって物理の暴れに繋がることがあります。
	* 値を 1.0 より小さくすることで貫通が複数フレームに渡って解決されるようになり、コリジョン由来の振動が軽減されます。
	*
	* Controls how much of collider collision resolution results are reflected.
	* If value is 1.0, penetration due to collision is resolved immediately, but repeating penetration → resolution at high speed may lead to physical violence.
	* Value less than 1.0 allows penetration to be resolved over multiple frames, reducing collision-induced vibration.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0, UIMax = 1.0, ClampMax = 1.0))
	double CollisionBlend = 0.8;

	/**
	* コライダの衝突時の許容深度です。
	* 値の分だけコライダの押し出し由来の力が減衰します。
	* 適切に調節することで振動が改善される可能性があります。
	*
	* Allowable depth at collider impact.
	* Force originating from collider extrusion is attenuated by amount of value.
	* Proper adjustment may improve vibration.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 0.0, ClampMin = 0.0))
	double ColliderPenetrationDepth = 0.5;

	/**
	* シミュレーション対象のチェーンに沿って線分の衝突判定を生成します。
	* これにより布がより一層、貫通しなくなります。
	* SIMDで4つの線分を一度に処理して高速化していますが、それでも処理負荷が大きく上がる可能性があることに注意してください。
	*
	* Generates collision determination for line segment along chain to be simulated.
	* This will prevent more cloth penetration.
	* Note that although SIMD handles 4 line segments at once, processing load can still increase significantly!
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	bool bUseEdgeCollider = false;

	/**
	* 水平方向のチェーンにも線分の衝突判定を生成します。
	*
	* Generates line segment collision determination for horizontal chains as well.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseHorizontalStructuralConstraint && bUseEdgeCollider", EditConditionHides))
	bool bUseHorizontalEdgeCollider = false;

	/**
	* 摩擦を 0.0 ～ 1.0 で指定します。
	* 摩擦はシミュレーション対象のボーンがキャラクター等のボディコライダに衝突したときに発生します。
	* 例えば布の場合は値が小さいほど、さらさらとした繊維のような印象を与えます。
	*
	* Specify friction from 0.0 to 1.0.
	* Friction occurs when bones to be simulated collide with body collider such as character.
	* For example, in case of cloth, smaller value gives impression of lighter fiber.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Friction"))
	FHGMFrictionSettings FrictionSettings {};

	/**
	* 物理シミュレーション対象のボーンの質量です。
	* Structure, Shear, Bend 拘束においてペアとなるボーンよりも質量が大きい場合、そのボーンは拘束の影響を受けにくくなります。
	*
	* Mass of the bone to be physically simulated.
	* Structure, Shear, Bend If mass is greater than paired bone in constraint, Its bones are less susceptible to constraint.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Mass"))
	FHGMMassSettings MassSettings {};

	/**
	* アクターの移動によって発生する力の減衰を 0.0 ～ 1.0 で指定します。
	* 1.0 の場合はアクターが移動しても全く物理的な反応は起きません。
	* 0.0 の場合は少しでもアクターが移動すると物理的な反応が起きます。
	*
	* Specifies attenuation of force generated by the actor's movement from 0.0 to 1.0.
	* 1.0, no physical reaction occurs at all when actor moves.
	* In case of 0.0, if actor moves even slightly, physical reaction occurs.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "World Velocity Damping"))
	FHGMWorldVelocityDampingSettings WorldVelocityDampingSettings {};

	/**
	* アクターの移動によって発生する力を無視する閾値です。
	* 1フレームの移動量 > IgnoreWorldVelocityThreshold のとき、アクターの移動は物理シミュレーションの結果に影響を与えないようになります。
	*
	* Threshold to ignore force generated by actor's movement.
	* If amount of movement in one frame > IgnoreWorldVelocityThreshold, actor's movement will not affect results of physics simulation.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "")
	double IgnoreWorldVelocityThreshold = 100.0f;

	/**
	* アクターの回転によって発生する力の減衰を 0.0 ～ 1.0 で指定します。
	* 0.0 の場合はアクターが回転しても全く物理的な反応は起きません。
	* 1.0 の場合は少しでもアクターが回転すると物理的な反応が起きます。
	*
	* Specifies attenuation of force generated by the actor's rotation from 0.0 to 1.0.
	* 1.0, no physical reaction occurs at all when actor rotates.
	* In case of 0.0, if actor rotates even slightly, physical reaction occurs.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "World Angular Velocity Damping"))
	FHGMWorldAngularVelocityDampingSettings WorldAngularVelocityDampingSettings {};

	/**
	* アクターの回転によって発生する力を無視する閾値です。
	* 1フレームの回転量 > IgnoreWorldAngularVelocityThreshold のとき、アクターの移動は物理シミュレーションの結果に影響を与えないようになります。
	*
	* Threshold to ignore force generated by actor's rotation.
	* If amount of rotation in one frame > IgnoreWorldVelocityThreshold, actor's rotation will not affect results of physics simulation.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (ForceUnits = "Degrees"))
	double IgnoreWorldAngularVelocityThreshold = 20.0f;

	/**
	* 物理計算を行う原点となるボーンを指定します。
	* 例えば Hip の姿勢が大きく変化することにより物理が暴れる場合は Hip を指定します。
	* Hip の姿勢の変化が与える影響は SimulationVelocityDamping, SimulationAngularVelocityDamping で細かく調整できます。
	*
	* Specifies bone that is the origin of physics calculation.
	* For example, if large change in Hip's posture causes physics to go haywire, specify Hip.
	* Effect of the Hip's posture change can be fine-tuned with SimulationVelocityDamping and SimulationAngularVelocityDamping.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseSimulationRootBone"))
	FBoneReference SimulationRootBone {};
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (InlineEditConditionToggle))
	bool bUseSimulationRootBone = false;

	/**
	* SimulationRootBone の移動によって発生する力の減衰を 0.0 ～ 1.0 で指定します。
	* 1.0 の場合は SimulationRootBone が移動しても全く物理的な反応は起きません。
	* 0.0 の場合は少しでも SimulationRootBone が移動すると物理的な反応が起きます。
	*
	* Specifies attenuation of force generated by the SimulationRootBone's movement from 0.0 to 1.0.
	* 1.0, no physical reaction occurs at all when SimulationRootBone moves.
	* In case of 0.0, if SimulationRootBone moves even slightly, physical reaction occurs.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Simulation Velocity Damping", EditCondition = "bUseSimulationRootBone", EditConditionHides))
	FHGMSimulationVelocityDampingSettings SimulationVelocityDampingSettings {};

	/**
	* SimulationRootBone の移動によって発生する力を無視する閾値です。
	* 1フレームの移動量 > IgnoreSimulationVelocityThreshold のとき、アクターの移動は物理シミュレーションの結果に影響を与えないようになります。
	*
	* Threshold to ignore force generated by SimulationRootBone's movement.
	* If amount of movement in one frame > IgnoreSimulationVelocityThreshold, SimulationRootBone's movement will not affect results of physics simulation.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (EditCondition = "bUseSimulationRootBone", EditConditionHides))
	double IgnoreSimulationVelocityThreshold = 100.0f;

	/**
	* SimulationRootBone の回転によって発生する力の減衰を 0.0 ～ 1.0 で指定します。
	* 0.0 の場合は SimulationRootBone が回転しても全く物理的な反応は起きません。
	* 1.0 の場合は少しでも SimulationRootBone が回転すると物理的な反応が起きます。
	*
	* Specifies attenuation of force generated by the SimulationRootBone's rotation from 0.0 to 1.0.
	* 1.0, no physical reaction occurs at all when SimulationRootBone rotates.
	* In case of 0.0, if SimulationRootBone rotates even slightly, physical reaction occurs.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Simulation Angular Velocity Damping", EditCondition = "bUseSimulationRootBone", EditConditionHides))
	FHGMSimulationAngularVelocityDampingSettings SimulationAngularVelocityDampingSettings {};

	/**
	* SimulationRootBone の回転によって発生する力を無視する閾値です。
	* 1フレームの回転量 > IgnoreSimulationAngularVelocityThreshold のとき、アクターの移動は物理シミュレーションの結果に影響を与えないようになります。
	*
	* Threshold to ignore force generated by SimulationRootBone's rotation.
	* If amount of rotation in one frame > IgnoreSimulationVelocityThreshold, SimulationRootBone's rotation will not affect results of physics simulation.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (ForceUnits = "Degrees", EditCondition = "bUseSimulationRootBone", EditConditionHides))
	double IgnoreSimulationAngularVelocityThreshold = 20.0f;

	/**
	* 全ての力の減衰をまとめて調整するパラメータです。( ≒ 慣性 )
	* 当該パラメータの調整で全体的な物理挙動の緩急をコントロールできます。
	* 例えばフワフワとした動きにしたい場合は値を 0 より大きくなるように調整してください。
	*
	* This parameter adjusts damping of all forces together.( ≈ Inertia )
	* The overall physical behavior can be controlled by adjusting relevant parameters.
	* For example, if you want fluffy movement, adjust value so that it is greater than 0.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Master Damping"))
	FHGMMasterDampingSettings MasterDampingSettings {};

	/**
	* 物理シミュレーション対象のボーンにかかる重力です。
	* 注意点: 重力は摩擦や各種減衰パラメータの影響を受けません。
	*
	* Gravity applied to the bone of the physics simulation target.
	* Note: Gravity is not affected by friction or various damping parameters.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (DisplayName = "Gravity"))
	FHGMGravitySettings GravitySettings {};

	/**
	* 物理シミュレーションの反復回数です。
	* 数値を上げるほどシミュレーション結果の品質が向上します。
	* 例えば Stiffness を十分に大きな値にしたにもかかわらず、布が伸びて柔らかいと感じる場合、反復回数を上げることで改善するかもしれません。
	* ただし、トレードオフとして処理負荷が上昇することに注意してください。
	*
	* Number of iterations of physics simulation.
	* Higher value, better quality of simulation results.
	* For example, if cloth feels stretched and soft despite a sufficiently large value for Stiffness, increasing number of iterations may help.
	* Note, however, that trade-off is increase in processing load.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "", meta = (UIMin = 1.0, ClampMin = 1.0))
	int32 SolverIterations = 8;
};


struct FHGMPhysicsContext
{
	FHGMPhysicsSettings PhysicsSettings {};

	FHGMTransform SkeletalMeshComponentTransform {};
	FHGMTransform PrevSkeletalMeshComponentTransform {};

	FHGMTransform SimulationRootBoneTransform {};
	FHGMTransform PrevSimulationRootBoneTransform {};

	FHGMSIMDReal sDeltaTime = FHGMSIMDLibrary::LoadConstant(0.016);
	FHGMSIMDReal sPrevDeltaTime = FHGMSIMDLibrary::LoadConstant(0.016);
	FHGMSIMDReal sDeltaTimeExponent = HGMSIMDConstants::OneReal;

	FHGMReal Alpha = 1.0;

	bool bIsFirstUpdate = true;
};


struct FHGMDynamicBoneSolver
{
public:
	bool Initialize(const FBoneContainer& RequiredBones, const TArray<FHGMChainSetting>& ChainSettings, FHGMPhysicsSettings& PhysicsSettings, FHGMPhysicsContext& PhysicsContext);

	FORCEINLINE bool HasInitialized() const
	{
		return bHasInitialized;
	}

	void PreSimulate(FComponentSpacePoseContext& Output, const FHGMPhysicsSettings& PhysicsSettings, FHGMPhysicsContext& PhysicsContext, int32 AnimationCurveNumber);

	void Simulate(FComponentSpacePoseContext& Output, FHGMPhysicsContext& PhysicsContext, const FHGMBodyCollider& BodyCollider, const FHGMBodyCollider& PrevBodyCollider, TConstArrayView<FHGMSIMDPlaneCollider> PlaneColliders);

	void OutputSimulateResult(FHGMPhysicsContext& PhysicsContext, FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);

	FHGMSimulationPlane SimulationPlane {};

	// Note: Index is not packed for SIMD.
	TArray<FBoneReference> Bones {};

	// Note: Index is packed for SIMD.
	TArray<FHGMSIMDVector3> Positions {};
	TArray<FHGMSIMDVector3> PrevPositions {};
	TArray<FHGMSIMDVector3> ReferencePositions {};
	TArray<FHGMSIMDVector3> AnimPosePositions {};
	TArray<FHGMSIMDReal> BoneLengthRates {};
	TArray<FHGMSIMDReal> DummyBoneMasks {};
	TArray<FHGMSIMDReal> FixedBlends {};
	TArray<FHGMSIMDReal> Frictions {};
	TArray<FHGMSIMDReal> ActualFrictions {};
	TArray<FHGMSIMDReal> WorldVelocityDampings {};
	TArray<FHGMSIMDReal> WorldAngularVelocityDampings {};
	TArray<FHGMSIMDReal> SimulationVelocityDampings {};
	TArray<FHGMSIMDReal> SimulationAngularVelocityDampings {};
	TArray<FHGMSIMDReal> MasterDampings {};
	TArray<FHGMSIMDReal> BoneSphereColliderRadiuses {};
	TArray<FHGMSIMDReal> InverseMasses {};

	// Contact cache.
	TArray<FHGMSIMDColliderContact> BodyColliderContactCache {};
	TArray<FHGMSIMDColliderContact> VerticalContactCache {};
	TArray<FHGMSIMDColliderContact> HorizontalContactCache {};
	TArray<FHGMSIMDColliderContact> PlaneColliderContactCache {};

	// Constraints.
	TArray<FHGMSIMDStructure> VerticalStructures {};
	TArray<FHGMSIMDStructure> HorizontalStructures {};
	TArray<FHGMSIMDStructure> VerticalBendStructures {};
	TArray<FHGMSIMDStructure> HorizontalBendStructures {};
	TArray<FHGMSIMDShearStructure> ShearStructures {};
	TArray<FHGMSIMDRelativeLimitAngle> RelativeLimitAngles {};
	TArray<FHGMSIMDAnimPoseConstraintMovableRadius> AnimPoseConstraintMovableRadiuses {};
	TArray<FHGMSIMDAnimPoseConstraintLimitAngle> AnimPoseConstraintLimitAngles {};
	TArray<FHGMSIMDInt> AnimPosePlanarConstraintAxes {};

private:
	bool bHasInitialized = false;
};


// ---------------------------------------------------------------------------------------
// SolverLibrary
// ---------------------------------------------------------------------------------------
struct FHGMSolverLibrary
{
	static void Transpose(FHGMSimulationPlane& SimulationPlane);

	template<typename SIMDType, typename ComponentType>
	static void Transpose(const FHGMSimulationPlane& SimulationPlane, TArray<SIMDType>& Values)
	{
		auto Gather = [](SIMDType& S0, SIMDType& S1, SIMDType& S2, SIMDType& S3, int32 Index) -> SIMDType
		{
			TStaticArray<ComponentType, 4> Values {};
			FHGMSIMDLibrary::Store(S0, Index, Values[0]);
			FHGMSIMDLibrary::Store(S1, Index, Values[1]);
			FHGMSIMDLibrary::Store(S2, Index, Values[2]);
			FHGMSIMDLibrary::Store(S3, Index, Values[3]);

			SIMDType sResult {};
			FHGMSIMDLibrary::Load(sResult, Values);

			return sResult;
		};

		TArray<SIMDType> TransposedValues {};
		TransposedValues.SetNum(Values.Num());
		for (int32 PackedX = 0; PackedX < SimulationPlane.PackedHorizontalBoneNum; ++PackedX)
		{
			for (int32 UnpackedYBase = 0; UnpackedYBase < SimulationPlane.UnpackedVerticalBoneNum; UnpackedYBase += 4)
			{
				const int32 UnpackedY0 = UnpackedYBase + 0;
				const int32 UnpackedY1 = UnpackedYBase + 1;
				const int32 UnpackedY2 = UnpackedYBase + 2;
				const int32 UnpackedY3 = UnpackedYBase + 3;

				const int32 PackedIndex0 = UnpackedY0 * SimulationPlane.PackedHorizontalBoneNum + PackedX;
				const int32 PackedIndex1 = UnpackedY1 * SimulationPlane.PackedHorizontalBoneNum + PackedX;
				const int32 PackedIndex2 = UnpackedY2 * SimulationPlane.PackedHorizontalBoneNum + PackedX;
				const int32 PackedIndex3 = UnpackedY3 * SimulationPlane.PackedHorizontalBoneNum + PackedX;

				const SIMDType sValue0 = Gather(Values[PackedIndex0], Values[PackedIndex1], Values[PackedIndex2], Values[PackedIndex3], 0);
				const SIMDType sValue1 = Gather(Values[PackedIndex0], Values[PackedIndex1], Values[PackedIndex2], Values[PackedIndex3], 1);
				const SIMDType sValue2 = Gather(Values[PackedIndex0], Values[PackedIndex1], Values[PackedIndex2], Values[PackedIndex3], 2);
				const SIMDType sValue3 = Gather(Values[PackedIndex0], Values[PackedIndex1], Values[PackedIndex2], Values[PackedIndex3], 3);

				const int32 TransposedVPackedIndexBase = SimulationPlane.UnpackedVerticalBoneNum * PackedX;
				const int32 TransposedVPackedIndex0 = TransposedVPackedIndexBase + (SimulationPlane.PackedVerticalBoneNum * (UnpackedY0 % 4)) + (UnpackedY0 / 4);
				const int32 TransposedVPackedIndex1 = TransposedVPackedIndexBase + (SimulationPlane.PackedVerticalBoneNum * (UnpackedY1 % 4)) + (UnpackedY1 / 4);
				const int32 TransposedVPackedIndex2 = TransposedVPackedIndexBase + (SimulationPlane.PackedVerticalBoneNum * (UnpackedY2 % 4)) + (UnpackedY2 / 4);
				const int32 TransposedVPackedIndex3 = TransposedVPackedIndexBase + (SimulationPlane.PackedVerticalBoneNum * (UnpackedY3 % 4)) + (UnpackedY3 / 4);

				TransposedValues[TransposedVPackedIndex0] = sValue0;
				TransposedValues[TransposedVPackedIndex1] = sValue1;
				TransposedValues[TransposedVPackedIndex2] = sValue2;
				TransposedValues[TransposedVPackedIndex3] = sValue3;
			}
		}

		Values = MoveTemp(TransposedValues);
	}
};


// Hagoromo manages various parameters based on vertical chain.
// FScopedTranspose transposes those parameters and supports easy execution of horizontal processing.
struct FScopedTranspose
{
public:
	FScopedTranspose(FHGMSimulationPlane* Plane)
		: SimulationPlane(Plane)
	{
	}

	~FScopedTranspose()
	{
		Execute();
	}

	FORCEINLINE void Add(TArray<FHGMSIMDReal>* SIMDRealArray)
	{
		SIMDRealsArray.Emplace(SIMDRealArray);
	}

	FORCEINLINE void Add(TArray<FHGMSIMDVector3>* SIMDVectorArray)
	{
		SIMDVectorsArray.Emplace(SIMDVectorArray);
	}

	void Execute()
	{
		if (!SimulationPlane)
		{
			return;
		}

		for (TArray<FHGMSIMDReal>* SIMDRealArray : SIMDRealsArray)
		{
			if (SIMDRealArray)
			{
				FHGMSolverLibrary::Transpose<FHGMSIMDReal, FHGMReal>(*SimulationPlane, *SIMDRealArray);
			}
		}

		for (TArray<FHGMSIMDVector3>* SIMDVectorArray : SIMDVectorsArray)
		{
			if (SIMDVectorArray)
			{
				FHGMSolverLibrary::Transpose<FHGMSIMDVector3, FHGMVector3>(*SimulationPlane, *SIMDVectorArray);
			}
		}

		FHGMSolverLibrary::Transpose(*SimulationPlane);
	}

private:
	FHGMSimulationPlane* SimulationPlane = nullptr;
	TArray<TArray<FHGMSIMDReal>*> SIMDRealsArray {};
	TArray<TArray<FHGMSIMDVector3>*> SIMDVectorsArray {};
};
