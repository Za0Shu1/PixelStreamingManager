// Copyright Epic Games, Inc. All Rights Reserved.


#include "PixelStreamingManager.h"

#include "RequiredProgramMainCPPInclude.h"
#include "Framework/Application/SlateApplication.h"
#include "StandaloneRenderer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"
#include "Internationalization/Internationalization.h"

#include "CommonStyle.h"
#include "SServerConfigWidget.h"
#include "Widgets/SCanvas.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SWidget.h"


#define FromHex(Hex) FLinearColor::FromSRGBColor(FColor::FromHex(Hex))

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

			const TSharedPtr<SServerConfigWidget> ServerConfigWidget = SNew(SServerConfigWidget);
			
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
						SNew(SBorder)
						.BorderImage(FPSManagerStyle::Get().GetBrush(TEXT("Background")))
						.BorderBackgroundColor(FLinearColor(1,1,1,0.5))
					]

					+SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						
						// left panel
						+SHorizontalBox::Slot()
						.SizeParam(FAuto())
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Fill)
						.Padding(FMargin(4))
						[
							SNew(SBox)
							.WidthOverride(300.f)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(SOverlay)
								+SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									// left background
									SNew(SBorder)
									.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FromHex("45326666"))
								]

								// icon
								+SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Top)
								[
									SNew(SVerticalBox)
									// icon
									+SVerticalBox::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Top)
									.AutoHeight()
									.Padding(FMargin(FVector4f(0,20,0,0)))
									[
										SNew(SImage)
										.Image(icon)
										.DesiredSizeOverride(FVector2d(48,48))
									]
									//name
									+SVerticalBox::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Fill)
									.Padding(FMargin(FVector4f(0,5,0,0)))
									[
										SNew(STextBlock)
										.Text(FText(LOCTEXT("PixelStreamingManager", "像素流管理平台")))
										.ColorAndOpacity(FromHex("ffffff66"))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
									]

									//properties
									+SVerticalBox::Slot()
									.FillHeight(1.f)
									.Padding(FMargin(FVector4f(0,5,0,0)))
									[
										ServerConfigWidget.ToSharedRef()
									]
								]
								
							]
						]

						// right panel
						+SHorizontalBox::Slot()
						.SizeParam(FStretch(1.f))
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(FMargin(0,4,4,4))
						[
							SNew(SOverlay)
							+SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								// right background
								SNew(SBorder)
								.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FromHex("665B6666"))
							]
						]
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
