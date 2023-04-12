// Copyright Epic Games, Inc. All Rights Reserved.


#include "../Public/PixelStreamingManager.h"

#include "RequiredProgramMainCPPInclude.h"
#include "Framework/Application/SlateApplication.h"
#include "StandaloneRenderer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"
#include "Widgets/Images/SImage.h"
#include "Internationalization/Internationalization.h"

#include "../Public/CommonStyle.h"
#include "Widgets/Layout/SBorder.h"

DEFINE_LOG_CATEGORY_STATIC(LogPixelStreamingManager, Log, All);

#define LOCTEXT_NAMESPACE "PixelStreamingManager"

IMPLEMENT_APPLICATION(PixelStreamingManager, "PixelStreamingManager");

static void OnRequestExit()
{
	RequestEngineExit(TEXT("Pixel streaming manager closed"));
}


int WINAPI WinMain(_In_ HINSTANCE hInInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int nCmdShow)
{
	// start up the main loop
	GEngineLoop.PreInit(GetCommandLineW());
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

			// build the window
			const FSlateBrush* icon = FPSManagerStyle::Get().GetBrush(TEXT("CustomAppIcon"));

			FText TitleText = FText(LOCTEXT("PixelStreamingManager", "Pixel Streaming Global Manager"));
			
			TSharedPtr<SWindow> MainWindow =
				SNew(SWindow)
				.Title(TitleText)
				.ClientSize(FVector2D(1280, 720))
				.IsInitiallyMaximized(false)
				[
					SNew(SOverlay)
					// background 
					+SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					]

					// icon
					+SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.Padding(FMargin(FVector4f(10,10,0,0)))
					[
						SNew(SImage).Image(icon)
					]
					
				];
			Slate->AddWindow(MainWindow.ToSharedRef());
			
			// tick
			while (!IsEngineExitRequested())
			{
				FSlateApplication::Get().Tick();
				FSlateApplication::Get().PumpMessages();
			}
			
			// Clean up the custom styles
			FPSManagerStyle::Shutdown();

			// Close down the Slate application
			FSlateApplication::Shutdown();
		}
		
		FEngineLoop::AppPreExit();
		FModuleManager::Get().UnloadModulesAtShutdown();
		FEngineLoop::AppExit();
		return 1;
	}
}
