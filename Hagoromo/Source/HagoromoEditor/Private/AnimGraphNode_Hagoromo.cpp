// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "AnimGraphNode_Hagoromo.h"

#include "UnrealWidgetFwd.h"
#include "Editor/AnimGraph/Public/AnimNodeEditModes.h"
#include "AnimNodeEditModes.h"
#include "Kismet2/CompilerResultsLog.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#define LOCTEXT_NAMESPACE "Hagoromo"

namespace HagoromoEditorInternal
{
	static const FSlateColor OffColor = FSlateColor(FColor(145, 215, 255, 255));
	static const FSlateColor OnColor = FSlateColor(FColor(255, 145, 145, 255));

	static void CopyNodeData(FAnimNode_Hagoromo* Dist, FAnimNode_Hagoromo* Src)
	{
		Dist->ChainSettings = Src->ChainSettings;
		Dist->PhysicsSettings = Src->PhysicsSettings;
		Dist->PhysicsAssetForBodyCollider = Src->PhysicsAssetForBodyCollider;
		Dist->AdditionalColliderSettings = Src->AdditionalColliderSettings;
	}
}


UAnimGraphNode_Hagoromo::UAnimGraphNode_Hagoromo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#define SNEW_CONSOLE_COMMAND_BUTTON(ButtonName, Command) \
	SNew(SButton) \
	.HAlign(HAlign_Center) \
	.VAlign(VAlign_Center) \
	.OnClicked_Lambda([this]() \
	{ \
		static IConsoleVariable* CVar##ButtonName = IConsoleManager::Get().FindConsoleVariable(TEXT(#Command)); \
		if (CVar##ButtonName) \
		{ \
			int32 NewValue = 0; \
			if (CVar##ButtonName->GetInt() <= 0) \
			{ \
				NewValue = 2; \
			} \
			CVar##ButtonName->Set(NewValue); \
		} \
		return FReply::Handled(); \
	}) \
	.ButtonColorAndOpacity_Lambda([this]() \
	{ \
		static IConsoleVariable* CVar##ButtonName = IConsoleManager::Get().FindConsoleVariable(TEXT(#Command)); \
		if (!CVar##ButtonName) \
		{ \
			return HagoromoEditorInternal::OffColor; \
		} \
			int32 ButtonName##Value = CVar##ButtonName->GetInt(); \
		return ButtonName##Value > 0 ? HagoromoEditorInternal::OnColor : HagoromoEditorInternal::OffColor; \
	}) \
	.Content() \
	[ \
		SNew(STextBlock) \
		.Text(FText::FromString(#ButtonName)) \
	] \


void UAnimGraphNode_Hagoromo::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	Super::CustomizeDetails(DetailBuilder);

	IDetailCategoryBuilder& ViewportCategory = DetailBuilder.EditCategory(TEXT("Hagoromo Debug Visualizer"));
	FDetailWidgetRow& WidgetRow = ViewportCategory.AddCustomRow(LOCTEXT("HagoromoDebugVisualizer", "HagoromoDebugVisualizer"));

	WidgetRow
	[
		SNew(SUniformGridPanel)
		.SlotPadding(FMargin(2.0f, 0.0f, 2.0f, 0.0f))
		// Line 1
		+ SUniformGridPanel::Slot(0, 0)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowBoneColliders, p.Hagoromo.ShowBoneColliders)
		]
		+ SUniformGridPanel::Slot(1, 0)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowBodyCollider, p.Hagoromo.ShowBodyCollider)
		]
		+ SUniformGridPanel::Slot(2, 0)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowPlaneColliders, p.Hagoromo.ShowPlaneColliders)
		]
		// Line 2
		+ SUniformGridPanel::Slot(0, 1)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowStructures, p.Hagoromo.ShowStructures)
		]
		+ SUniformGridPanel::Slot(1, 1)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowShearStructures, p.Hagoromo.ShowShearStructures)
		]
		+ SUniformGridPanel::Slot(2, 1)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowFixedBledns, p.Hagoromo.ShowFixedBlends)
		]
		// Line 3
		+ SUniformGridPanel::Slot(0, 2)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowAnimPoseMovableRadiuses, p.Hagoromo.ShowAnimPoseMovableRadiusConstraint)
		]
		+ SUniformGridPanel::Slot(1, 2)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowAnimPoseLimitAngles, p.Hagoromo.ShowAnimPoseLimitAngleConstraint)
		]
		+ SUniformGridPanel::Slot(2, 2)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowAnimPosePlanarAxes, p.Hagoromo.ShowAnimPosePlanarConstraint)
		]
		// Line 4
		+ SUniformGridPanel::Slot(0, 3)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowRelativeLimitAngle, p.Hagoromo.ShowRelativeLimitAngleConstraint)
		]
		+ SUniformGridPanel::Slot(1, 3)
		[
			SNEW_CONSOLE_COMMAND_BUTTON(ShowVelocities, p.Hagoromo.ShowVelocities)
		]
	];
}


void UAnimGraphNode_Hagoromo::ValidateAnimNodePostCompile(FCompilerResultsLog& MessageLog, UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex)
{
	Super::ValidateAnimNodePostCompile(MessageLog, CompiledClass, CompiledNodeIndex);

	// The skeleton of the preview mesh is used to accurately determine if bones are included.
	const USkeleton* TargetSkeleton = nullptr;
	if (CompiledClass)
	{
		TargetSkeleton = CompiledClass->GetTargetSkeleton();
		if (TargetSkeleton)
		{
			if (const USkeletalMesh* const PreviewMesh = TargetSkeleton->GetPreviewMesh())
			{
				TargetSkeleton = PreviewMesh->GetSkeleton();
			}
		}
	}

	if (!TargetSkeleton)
	{
		return;
	}

	// Check root bone of chain :
	for (int32 Index = 0; Index < Node.ChainSettings.Num(); ++Index)
	{
		FHGMChainSetting& ChainSetting = Node.ChainSettings[Index];

		if (!ChainSetting.RootBone.Initialize(TargetSkeleton))
		{
			MessageLog.Error(*FString::Printf(TEXT("@@ ChainSetting[%d].RootBone is not included in skeletal mesh."), Index), this);
		}

		if (ChainSetting.bUseExcludeBones)
		{
			for (int32 ExcludeBoneIndex = 0; ExcludeBoneIndex < ChainSetting.ExcludeBones.Num(); ++ExcludeBoneIndex)
			{
				FBoneReference& ExcludeBone = ChainSetting.ExcludeBones[ExcludeBoneIndex];
				if (!ExcludeBone.Initialize(TargetSkeleton))
				{
					MessageLog.Error(*FString::Printf(TEXT("@@ ChainSetting[%d].ExcludeBones[%d] is not included in skeletal mesh."), Index, ExcludeBoneIndex), this);
				}
			}
		}
	}

	// Check simulation root bone :
	if (Node.PhysicsSettings.bUseSimulationRootBone)
	{
		if (!Node.PhysicsSettings.SimulationRootBone.Initialize(TargetSkeleton))
		{
			MessageLog.Error(*FString::Printf(TEXT("@@ SimulationRootBone is not included in skeletal mesh.")), this);
		}
	}

	// Check driving bone of gravity :
	if (Node.PhysicsSettings.GravitySettings.bUseBoneSpaceGravity)
	{
		if (!Node.PhysicsSettings.GravitySettings.DrivingBone.Initialize(TargetSkeleton))
		{
			MessageLog.Error(*FString::Printf(TEXT("@@ GravitySettings.DrivingBone is not included in skeletal mesh.")), this);
		}
	}

	Node.Initialize();
}


void UAnimGraphNode_Hagoromo::CopyNodeDataToPreviewNode(FAnimNode_Base* AnimNode)
{
	FAnimNode_Hagoromo* Dist = StaticCast<FAnimNode_Hagoromo*>(AnimNode);
	if (!Dist)
	{
		return;
	}

	HagoromoEditorInternal::CopyNodeData(Dist, &Node);
	Dist->Initialize();
}


FText UAnimGraphNode_Hagoromo::GetControllerDescription() const
{
	return LOCTEXT("UAnimGraphNode_Hagoromo_Title", "Hagoromo");
}


FText UAnimGraphNode_Hagoromo::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return GetControllerDescription();
}


#undef LOCTEXT_NAMESPACE