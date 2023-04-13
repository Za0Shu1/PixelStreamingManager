// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PixelStreamingManager : ModuleRules
{
	public PixelStreamingManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add("Runtime/Launch/Public");

		PrivateIncludePaths.Add("Runtime/Launch/Private");		// For LaunchEngineLoop.cpp include

		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("Projects");
		//添加Slate依赖
		PrivateDependencyModuleNames.AddRange(
		   new string[]
		   {
				"CoreUObject",
				"Slate",
				"SlateCore",
				"StandaloneRenderer"
		   });
	}
}

