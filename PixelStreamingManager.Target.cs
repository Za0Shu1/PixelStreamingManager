// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
public class PixelStreamingManagerTarget : TargetRules
{
	public PixelStreamingManagerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Program;// 类型为Program
		LinkType = TargetLinkType.Monolithic;// 模块的链接方式，Monolithic：1个，Modular：多个dll
		LaunchModuleName = "PixelStreamingManager";// 启动模块

		// Lean and mean
		//其还会决定bCompileLeanAndMeanUE
		bBuildDeveloperTools = false;

		// Never use malloc profiling in Unreal Header Tool.  We set this because often UHT is compiled right before the engine
		// automatically by Unreal Build Tool, but if bUseMallocProfiler is defined, UHT can operate incorrectly.
		// 是否启用内存分析
		bUseMallocProfiler = false;

		// Editor-only data, however, is needed
		bBuildWithEditorOnlyData = true;

		// Currently this app is not linking against the engine, so we'll compile out references from Core to the rest of the engine
		bCompileAgainstEngine = false;// 是否链接所有引擎程序生成的项目
		//bCompileAgainstCoreUObject = false;
		bCompileAgainstCoreUObject = true;// 需要连接到CoreUObject模块
		//bCompileAgainstApplicationCore = false;
		bCompileAgainstApplicationCore = true;
		// UnrealHeaderTool is a console application, not a Windows app (sets entry point to main(), instead of WinMain())
		//bIsBuildingConsoleApplication = true;
		bIsBuildingConsoleApplication = false;// true:控制台app;false:窗体app
	}
}

