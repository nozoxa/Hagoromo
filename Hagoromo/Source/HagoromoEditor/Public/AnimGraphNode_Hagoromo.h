// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimNode_Hagoromo.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimGraphNode_Hagoromo.generated.h"

class FCompilerResultsLog;


UCLASS(meta=(Keywords = "Hagoromo"))
class UAnimGraphNode_Hagoromo final : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere)
	FAnimNode_Hagoromo Node {};

public:
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	void ValidateAnimNodePostCompile(FCompilerResultsLog& MessageLog, UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex) override;

	void CopyNodeDataToPreviewNode(FAnimNode_Base* AnimNode) override;

	FText GetControllerDescription() const override;

	FORCEINLINE const FAnimNode_SkeletalControlBase* GetNode() const override
	{
		return &Node;
	}

private:
	FNodeTitleTextTable CachedNodeTitles {};
};