// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "HagoromoModule.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHagoromoRuntime, Log, Log);


// ---------------------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------------------
namespace HGMGlobal
{
	extern FString IniFileName;
	extern FHGMReal TargetFrameRate;
}


// ---------------------------------------------------------------------------------------
// Macros
// ---------------------------------------------------------------------------------------
#define HGM_LOG(Verbosity, Format, ...) \
UE_LOG(LogHagoromoRuntime, Verbosity, TEXT("[%hs L%d] %s"), (__FUNCTION__), (__LINE__), *FString::Printf(Format, ##__VA_ARGS__))

#define HGM_CLOG(Condition, Verbosity, Format, ...) \
UE_CLOG(Condition, LogHagoromoRuntime, Verbosity, TEXT("[%hs L%d] %s"), (__FUNCTION__), (__LINE__), *FString::Printf(Format, ##__VA_ARGS__))


DECLARE_STATS_GROUP(TEXT("Hagoromo"), STATGROUP_Hagoromo, STATCAT_Advanced);

// ---------------------------------------------------------------------------------------
// Plugin Settings
// ---------------------------------------------------------------------------------------
UCLASS(config=Hagoromo)
class UHagoromoSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "Hagoromo Plugin Settings")
	float TargetFrameRate = 60.0f;
};


// ---------------------------------------------------------------------------------------
// Module
// ---------------------------------------------------------------------------------------
class FHagoromoModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
