// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "HagoromoEditorModule.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHagoromoEditor, Log, Log);


// ---------------------------------------------------------------------------------------
// Macro
// ---------------------------------------------------------------------------------------
#define HGM_LOG(Verbosity, Format, ...) \
UE_LOG(LogHagoromoEditor, Verbosity, TEXT("[%hs L%d] %s"), (__FUNCTION__), (__LINE__), *FString::Printf(Format, ##__VA_ARGS__)); \

#define HGM_CLOG(Condition, Verbosity, Format, ...) \
UE_CLOG(Condition, LogHagoromoEditor, Verbosity, TEXT("[%hs L%d] %s"), (__FUNCTION__), (__LINE__), *FString::Printf(Format, ##__VA_ARGS__)); \


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
class FHagoromoEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline FHagoromoEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FHagoromoEditorModule>("HagoromoEditor");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("HagoromoEditor");
	}
};
