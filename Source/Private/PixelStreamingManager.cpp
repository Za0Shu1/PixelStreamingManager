// Copyright Epic Games, Inc. All Rights Reserved.


#include "PixelStreamingManager.h"
#include "CommonStyle.h"
#include "FSettingsConfig.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

DEFINE_LOG_CATEGORY(LogPixelStreamingManager);

FDoScanTask::FDoScanTask(FOnScanFinished Callback):OnScanFinished(Callback)
{
		
}

void FDoScanTask::DoWork() const
{
	//@todo Scan this server folder
	int Index = 0;
	while (Index++ < 1)
	{
		UE_LOG(LogPixelStreamingManager,Warning,TEXT("Do thread work in background task..."));
		FPlatformProcess::Sleep(2.f);
	}
	UE_LOG(LogPixelStreamingManager,Warning,TEXT("Do Scan Finished."));
	OnScanFinished.ExecuteIfBound();
}

FPixelStreamingManager::FPixelStreamingManager(FSlateApplication& InSlate):Slate(InSlate)
{
		
}

FPixelStreamingManager::~FPixelStreamingManager()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
}

#pragma region Mannully Hand Tick
bool FPixelStreamingManager::Tick(float UnusedDeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FPixelStreamingManager_Tick);

	if(bIsScanning)
	{
		if(bScanFinished)
		{
			UE_LOG(LogPixelStreamingManager,Warning,TEXT("Scan finished in handled tick function."));
			StopScan();
		}
	}
	
	return true;
}

void FPixelStreamingManager::StartTicker()
{
	if (!TickHandle.IsValid())
	{
		TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FPixelStreamingManager::Tick),0.1f);
	}
}

#pragma endregion

#pragma region Draw Window
void FPixelStreamingManager::Run()
{
	const FSlateBrush* icon = FPSManagerStyle::Get().GetBrush(TEXT("CustomAppIcon"));
	const FText TitleText = FText(LOCTEXT("PixelStreamingManager", "Pixel Streaming Global Manager"));
	InitializeConfig();
	
	// build the window
#pragma region Build Window
	const TSharedPtr<SWindow> MainWindow =
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

						+SOverlay::Slot()
						 .HAlign(HAlign_Fill)
						 .VAlign(VAlign_Fill)
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
							 .VAlign(VAlign_Top)
							 .AutoHeight()
							 .Padding(FMargin(FVector4f(0,5,0,0)))
							[
								SNew(STextBlock)
										.Text(FText(LOCTEXT("PixelStreamingManager", "像素流管理平台")))
										.ColorAndOpacity(FromHex("ffffff66"))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
							]
									
							//properties
							+SVerticalBox::Slot()
							 .HAlign(HAlign_Center)
							 .VAlign(VAlign_Fill)
							 .AutoHeight()
							 .Padding(FMargin(FVector4f(0,20,0,0)))
							[
								SNew(SHorizontalBox)
										
								+SHorizontalBox::Slot()
								 .VAlign(VAlign_Center)
								 .HAlign(HAlign_Right)
								 .Padding(FMargin(0,0,5,0))
								[
									SNew(SBox)
											.WidthOverride(150)
											.HAlign(HAlign_Right)
									[
										SNew(STextBlock)
										.Text(FText(LOCTEXT("WebServersPath", "服务器路径：")))
									]
								]
										
								+SHorizontalBox::Slot()
								.Padding(FMargin(5,0,0,0))
								[
									SNew(SButton)
									.VAlign(VAlign_Center)
									.HAlign(HAlign_Left)
									.OnClicked_Raw(this,&FPixelStreamingManager::PickupFolder)
									[
										SAssignNew(PathText,STextBlock)
										.Text(FText::FromString(FSettingsConfig::Get().GetWebServersPath().IsEmpty() ? "..." : FSettingsConfig::Get().GetWebServersPath()))
										.ToolTipText(FText::FromString("Select your webservers folder."))
									]
								]
							]

							// commit
							+SVerticalBox::Slot()
							 .FillHeight(1.f)
							 .VAlign(VAlign_Bottom)
							 .HAlign(HAlign_Right)
							[
								SNew(SButton)
										.Text(FText(LOCTEXT("ScanFolder", "扫描")))
										.IsEnabled_Raw(this,&FPixelStreamingManager::CanDoScan)
										.OnClicked_Raw(this,&FPixelStreamingManager::DoScan)
										.ToolTipText_Raw(this,&FPixelStreamingManager::GenerateScanToolTip)
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

					+SOverlay::Slot()
					 .HAlign(HAlign_Center)
					 .VAlign(VAlign_Center)
					[
						// Circular Throbber
						SAssignNew(ScanningThrobber,SCircularThrobber)
								.Radius(50.f)
								.Period(1.f)
								.NumPieces(20)
								.Visibility(EVisibility::Collapsed)
					]
				]
			]
		];
#pragma endregion
	Slate.AddWindow(MainWindow.ToSharedRef());
	// tick
	while (!IsEngineExitRequested())
	{
		// Tick app logic
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		FTSTicker::GetCoreTicker().Tick(0.02);
			
		FSlateApplication::Get().Tick();
		FSlateApplication::Get().PumpMessages();
	}
}

void FPixelStreamingManager::InitializeConfig()
{
	
}

FReply FPixelStreamingManager::PickupFolder()
{
	//@todo: save this folder to config, next time we launch will use it.
	

	FString WebServerPath;
	// Prompt the user for the directory
	if (FDesktopPlatformModule::Get()->OpenDirectoryDialog(GetActiveWindow(), LOCTEXT("SelectServerPath", "Please select your web server path.").ToString(), "", WebServerPath))
	{
		UE_LOG(LogPixelStreamingManager,Display,TEXT("Web servers path selected at : %s"),*WebServerPath);
		if(PathText.IsValid())
		{
			PathText->SetText(FText::FromString(WebServerPath));
			PathText->SetToolTipText(FText::FromString(WebServerPath));
			FSettingsConfig::Get().SetWebServersPath(WebServerPath);
		}
	}
	return FReply::Handled();
}

FText FPixelStreamingManager::GenerateScanToolTip() const
{
	const FString Result =  CanDoScan() ? "Execute Scan" : "WebServers Path is invalid,Please check.";
	return FText::FromString(Result);
}

#pragma endregion 

#pragma region Scan Task
bool FPixelStreamingManager::CanDoScan() const
{
	return !FSettingsConfig::Get().GetWebServersPath().IsEmpty();
}

FReply FPixelStreamingManager::DoScan()
{
	StartScan();
	return FReply::Handled();
}

void FPixelStreamingManager::StartScan()
{
	if(ScanningThrobber.IsValid())
	{
		ScanningThrobber->SetVisibility(EVisibility::SelfHitTestInvisible);
	}
	FOnScanFinished Callback;
	Callback.BindRaw(this,&FPixelStreamingManager::OnScanTaskFinished);

	const auto ScanTask = new FAsyncTask<FDoScanTask>(Callback);
	bIsScanning = true;
	bScanFinished = false;
	ScanTask->StartBackgroundTask();
	StartTicker();
}

void FPixelStreamingManager::StopScan()
{
	bIsScanning = false;
	bScanFinished = false;

	// should call in game thread
	if(ScanningThrobber.IsValid())
	{
		ScanningThrobber->SetVisibility(EVisibility::Collapsed);
	}
}
#pragma endregion 
