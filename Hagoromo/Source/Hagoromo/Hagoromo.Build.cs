// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

using System.IO;
using UnrealBuildTool;

public class Hagoromo : ModuleRules
{
	public Hagoromo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// double --> float
		// Note: HGMSolvers.h and MathFwd.h may also need to be changed double to float.
		PublicDefinitions.Add("HGM_USE_FLOAT32=0");


		// If AVX is available, VectorRegister4Double may also be good choice.
		// For now, we recommend using VectorRegister4Float as long as there are no accuracy issues from performance perspective.
#if UE_PLATFORM_MATH_USE_AVX && !HGM_USE_FLOAT32
		PublicDefinitions.Add("HGM_USE_SIMD_REGISTER_FLOAT4X64=1");
#else
		PublicDefinitions.Add("HGM_USE_SIMD_REGISTER_FLOAT4X64=0");
#endif


		// Whether inverse square root, which is used primarily to find normal, should be calculated as low-load approximation or not.
		PublicDefinitions.Add("HGM_USE_ESTIMATED_RECIPROCAL_SQUARE_ROOT=0");


		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AnimGraphRuntime",
			"Slate",
			"SlateCore",
			"Settings"
		});
	}
}
