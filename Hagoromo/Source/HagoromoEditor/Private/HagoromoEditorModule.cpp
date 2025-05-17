// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HagoromoEditorModule.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"

DEFINE_LOG_CATEGORY(LogHagoromoEditor);

#define LOCTEXT_NAMESPACE "FHagoromoEditorModule"


void FHagoromoEditorModule::StartupModule()
{
	if (ISettingsModule* SettingsModule { FModuleManager::GetModulePtr<ISettingsModule>("Settings") })
	{
		ISettingsSectionPtr SettingsSection { SettingsModule->RegisterSettings("Project", "Plugins", "Hagoromo",
			LOCTEXT("RuntimeSettingsName", "Hagoromo"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the Hagoromo plugin"),
			GetMutableDefault<UHagoromoSettings>()) };
	}
}

void FHagoromoEditorModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule { FModuleManager::GetModulePtr<ISettingsModule>("Settings") })
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Hagoromo");
	}
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHagoromoEditorModule, HagoromoEditor)
