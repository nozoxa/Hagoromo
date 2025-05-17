// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

using UnrealBuildTool;

public class HagoromoEditor : ModuleRules
{
	public HagoromoEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"AnimGraph",
			"AnimGraphRuntime",
			"BlueprintGraph",
			"UnrealEd",
			"Persona",
			"Settings",
			"Hagoromo",
		});
	}
}
