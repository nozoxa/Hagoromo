// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HagoromoModule.h"

#include "Misc/ConfigContext.h"

DEFINE_LOG_CATEGORY(LogHagoromoRuntime);


FString HGMGlobal::IniFileName {};
FHGMReal HGMGlobal::TargetFrameRate = 60.0;


#define LOCTEXT_NAMESPACE "FHagoromoModule"
void FHagoromoModule::StartupModule()
{
	FString HagoromoIni {};
	if (GConfig)
	{
		FConfigContext::ReadIntoGConfig().Load(TEXT("Hagoromo"), HGMGlobal::IniFileName);
#if HGM_USE_FLOAT32
		GConfig->GetFloat(TEXT("/Script/Hagoromo.HagoromoSettings"), TEXT("TargetFrameRate"), HGMGlobal::TargetFrameRate, HGMGlobal::IniFileName);
#else
		GConfig->GetDouble(TEXT("/Script/Hagoromo.HagoromoSettings"), TEXT("TargetFrameRate"), HGMGlobal::TargetFrameRate, HGMGlobal::IniFileName);
#endif
	}
}


void FHagoromoModule::ShutdownModule()
{
}
#undef LOCTEXT_NAMESPACE


IMPLEMENT_MODULE(FHagoromoModule, Hagoromo)