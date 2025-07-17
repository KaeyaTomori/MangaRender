// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class MangaRender : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty")); }
	}
	public MangaRender(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePathModuleNames.AddRange(
			new string[]
			{
				"Launch",
			});
		PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "webp/include"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "webp/lib", "libwebpdecoder.lib"));
		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("Projects");

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"StandaloneRenderer", 
				"DesktopPlatform",
			});
		// to link with CoreUObject module:
		// PrivateDependencyModuleNames.Add("CoreUObject");

		// to enable LLM tracing:
		// GlobalDefinitions.Add("LLM_ENABLED_IN_CONFIG=1");
		// GlobalDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
	}
}
