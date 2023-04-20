#include "PixelStreamingManagerApp.h"
#include "PixelStreamingManager.h"

#include "CommonStyle.h"
#include "LaunchEngineLoop.h"
#include "Framework/Application/SlateApplication.h"
#include "Modules/ModuleManager.h"
#include "StandaloneRenderer.h"
#include "Misc/ConfigCacheIni.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

IMPLEMENT_APPLICATION(PixelStreamingManager, "PixelStreamingManager");


bool PixelStreamingMain(const TCHAR* CmdLine)
{
	// start up the main loop
	GEngineLoop.PreInit(CmdLine);
	check(GConfig && GConfig->IsReadyForUse());

	// Initialize high DPI mode
	FSlateApplication::InitHighDPI(true);
	{
		// Create the platform slate application (what FSlateApplication::Get() returns)
		TSharedRef<FSlateApplication> Slate = FSlateApplication::Create(MakeShareable(FPlatformApplicationMisc::CreateApplication()));
		{
			// Initialize renderer
			TSharedRef<FSlateRenderer> SlateRenderer = GetStandardStandaloneRenderer();

			// Try to initialize the renderer. It's possible that we launched when the driver crashed so try a few times before giving up.
			bool bRendererInitialized = Slate->InitializeRenderer(SlateRenderer, true);
			if (!bRendererInitialized)
			{
				// Close down the Slate application
				FSlateApplication::Shutdown();
				return 0;
			}

			// Set the normal UE IsEngineExitRequested() when outer frame is closed
			Slate->SetExitRequestedHandler(FSimpleDelegate::CreateStatic(&OnRequestExit));

			// Prepare the custom Slate styles
			FPSManagerStyle::Initialize();

			// Set the icon
			//FAppStyle::SetAppStyleSet(FPSManagerStyle::Get());
			//FSlateApplication::Get().SetAppIcon(FCoreStyle::Get().GetBrush("AppIcon"));

			// run the inner application loop
			FPixelStreamingManager App(Slate.Get());
			App.Run();

			// unload style
			FPSManagerStyle::Shutdown();
		}
		
		// close the slate application
		FSlateApplication::Shutdown();
	}
	
	FEngineLoop::AppPreExit();
	FModuleManager::Get().UnloadModulesAtShutdown();
	FEngineLoop::AppExit();
	return true;
}

void OnRequestExit()
{
	RequestEngineExit(TEXT("OnRequestExit"));
}
