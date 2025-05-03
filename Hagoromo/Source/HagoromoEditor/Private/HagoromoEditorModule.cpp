// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HagoromoEditorModule.h"

DEFINE_LOG_CATEGORY(LogHagoromoEditor);

#define LOCTEXT_NAMESPACE "FHagoromoEditorModule"


void FHagoromoEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FHagoromoEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHagoromoEditorModule, HagoromoEditor)
