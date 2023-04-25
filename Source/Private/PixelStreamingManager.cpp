// Copyright Epic Games, Inc. All Rights Reserved.


#include "PixelStreamingManager.h"

#include "CommonStyle.h"
#include "FSettingsConfig.h"
#include "SPSServerSingleton.h"
#include "HAL/FileManager.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"

DEFINE_LOG_CATEGORY(LogPixelStreamingManager);
#define LOCTEXT_NAMESPACE "PixelStreamingManager"

FDoScanTask::FDoScanTask(FString WebServersPath)
	: ScanFolder(WebServersPath)
{
}

void FDoScanTask::DoWork()
{
	UE_LOG(LogPixelStreamingManager, Warning, TEXT("Scanning Directory : %s "), *ScanFolder);

	IFileManager::Get().IterateDirectoryRecursively(*ScanFolder, [this](const TCHAR* Path, bool bIsDirectory)
	{
		if (!bIsDirectory)
		{
			FString FilePath(Path);
			FPaths::NormalizeFilename(FilePath);
			if (FilePath.EndsWith("run.bat") && FilePath.Contains("Matchmaker"))
			{
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("Matchmaker Launch File Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("run.bat");
				FSettingsConfig::Get().GetLaunchConfig().MatchMakerBatchPath = FilePath;
			}

			if (FilePath.EndsWith("run_local.bat") && FilePath.Contains("SignallingWebServer"))
			{
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("SignallingWebServer Local Launch File Found at { %s }"),
				       *FilePath);
				FSettingsConfig::Get().ValidServer("run_local.bat");
				FSettingsConfig::Get().GetLaunchConfig().SingnallingServerLocalPath = FilePath;
			}

			if (FilePath.EndsWith("Start_WithTURN_SignallingServer.ps1") && FilePath.Contains("SignallingWebServer"))
			{
				UE_LOG(LogPixelStreamingManager, Warning,
				       TEXT("SignallingWebServer Public Launch File Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("Start_WithTURN_SignallingServer.ps1");
				FSettingsConfig::Get().GetLaunchConfig().SingnallingServerPublicPath = FilePath;
			}

			if (FilePath.Contains("turnserver.exe"))
			{
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("Turn Server APP Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("turnserver.exe");
			}
			else if (FilePath.Contains("node.exe"))
			{
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("Node APP Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("node.exe");
			}
		}
		else
		{
			FString FilePath(Path);
			FPaths::NormalizeFilename(FilePath);
			if (FilePath.EndsWith("Matchmaker"))
			{
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("Directory Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("Matchmaker");
			}
			else if (FilePath.EndsWith("SFU"))
			{
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("Directory Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("SFU");
			}
			else if (FilePath.EndsWith("SignallingWebServer"))
			{
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("Directory Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("SignallingWebServer");
			}
		}

		return true; // Continue
	});
}

FPixelStreamingManager::FPixelStreamingManager(FSlateApplication& InSlate): Slate(InSlate)
{
}

FPixelStreamingManager::~FPixelStreamingManager()
{
	FTSTicker::GetCoreTicker().RemoveTicker(GlobalTickHandle);
}

#pragma region Mannully Hand Tick

bool FPixelStreamingManager::ScanTaskTick(float UnusedDeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FPixelStreamingManager_Tick);

	if (ScanTask)
	{
		if (ScanTask->IsWorkDone())
		{
			StopBackgroundThread();
			UE_LOG(LogPixelStreamingManager, Warning, TEXT("Scan task finished, destroy background thread."));
			StopScan();
			check(ScanTask == nullptr); // Expected after StopBackgroundThread() call.
		}
	}

	return true;
}

bool FPixelStreamingManager::Tick(float UnusedDeltaTime)
{
	if (RightPanel.IsValid())
	{
		float Temp = RightPanel->GetCachedGeometry().GetLocalSize().X;

		if (RightPanelWidth != Temp)
		{
			RightPanelWidth = Temp;
			OnWindowResized();
		}
	}
	return true;
}

void FPixelStreamingManager::StartScanTicker()
{
	if (!ScanTickHandle.IsValid())
	{
		ScanTickHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateRaw(this, &FPixelStreamingManager::ScanTaskTick), 0.1f);
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
			+ SOverlay::Slot()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			[
				SNew(SBorder)
						.BorderImage(FPSManagerStyle::Get().GetBrush(TEXT("Background")))
						.BorderBackgroundColor(FLinearColor(1, 1, 1, 0.5))
			]

			+ SOverlay::Slot()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)
				// left panel
				+ SHorizontalBox::Slot()
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
						+ SOverlay::Slot()
						  .HAlign(HAlign_Fill)
						  .VAlign(VAlign_Fill)
						[
							// left background
							SNew(SBorder)
									.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FromHex("45326666"))
						]

						+ SOverlay::Slot()
						  .HAlign(HAlign_Fill)
						  .VAlign(VAlign_Fill)
						[
							SNew(SVerticalBox)
							// icon
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Center)
							  .VAlign(VAlign_Top)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(SImage)
								.Image(icon)
								.DesiredSizeOverride(FVector2d(48, 48))
							]

							//name
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Center)
							  .VAlign(VAlign_Top)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 5, 0, 0)))
							[
								SNew(STextBlock)
								.Text(FText(LOCTEXT("PixelStreamingManager", "像素流管理平台")))
								.ColorAndOpacity(FromHex("#3399ff"))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
							]

							//properties
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Center)
							  .VAlign(VAlign_Fill)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								  .VAlign(VAlign_Center)
								  .HAlign(HAlign_Right)
								  .Padding(FMargin(0, 0, 5, 0))
								[
									SNew(SBox)
									.WidthOverride(150)
									.HAlign(HAlign_Right)
									[
										SNew(STextBlock)
										.Text(FText(LOCTEXT("WebServersPath", "服务器路径：")))
									]
								]

								+ SHorizontalBox::Slot()
								.Padding(FMargin(5, 0, 0, 0))
								[
									SNew(SButton)
									.VAlign(VAlign_Center)
									.HAlign(HAlign_Left)
									.OnClicked_Raw(this, &FPixelStreamingManager::PickupFolder)
									[
										SAssignNew(PathText, STextBlock)
										.Text(FText::FromString(
											                                FSettingsConfig::Get().GetWebServersPath().
											                                IsEmpty()
												                                ? "..."
												                                : FSettingsConfig::Get().
												                                GetWebServersPath()))
										.ToolTipText(FText::FromString(
											                                FSettingsConfig::Get().GetWebServersPath().
											                                IsEmpty()
												                                ? "Select your webservers folder."
												                                : FSettingsConfig::Get().
												                                GetWebServersPath()))
									]
								]
							]

							// commit
							+ SVerticalBox::Slot()
							  .FillHeight(1.f)
							  .VAlign(VAlign_Bottom)
							  .HAlign(HAlign_Right)
							[
								SNew(SHorizontalBox)
								
								+ SHorizontalBox::Slot()
								[
									SNew(SButton)
									.Text(FText(LOCTEXT("LaunchMatchmaker", "启动Matchmaker")))
									.IsEnabled_Lambda([this]()
									{
										return CanDoScan() && bUseMatchmaker;
									})
									.OnClicked_Raw(this, &FPixelStreamingManager::LaunchMatchMaker)
									.ToolTipText_Raw(this, &FPixelStreamingManager::GenerateLaunchMatchmakerToolTip)
								]

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(FMargin(2.f,0.f,0.f,0.f))
								[
									SNew(SButton)
									.Text(FText(LOCTEXT("ScanFolder", "扫描")))
									.IsEnabled_Raw(this, &FPixelStreamingManager::CanDoScan)
									.OnClicked_Raw(this, &FPixelStreamingManager::DoScan)
									.ToolTipText_Raw(this, &FPixelStreamingManager::GenerateScanToolTip)
								]
							]
						]
					]
				]

				// right panel
				+ SHorizontalBox::Slot()
				  .SizeParam(FStretch(1.f))
				  .HAlign(HAlign_Fill)
				  .VAlign(VAlign_Fill)
				  .Padding(FMargin(0, 4, 4, 4))
				[
					SAssignNew(RightPanel, SOverlay)
					+ SOverlay::Slot()
					  .HAlign(HAlign_Fill)
					  .VAlign(VAlign_Fill)
					[
						// right background
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FromHex("665B6666"))
					]

					+ SOverlay::Slot()
					  .HAlign(HAlign_Fill)
					  .VAlign(VAlign_Fill)
					[
						SNew(SScrollBox)
						.Orientation(EOrientation::Orient_Vertical)
						.ScrollBarVisibility(EVisibility::Visible)
						.ScrollBarPadding(ScrollBarPadding)
						.ScrollBarThickness(FVector2d(ScrollBarThickness))

						+ SScrollBox::Slot()
						  .Padding(FMargin(WrapBoxPadding))
						  .HAlign(HAlign_Center)
						[
							SAssignNew(ContainerArea, SBox)
							.HAlign(HAlign_Fill)
							[
								// SNew(SBorder)
								// 		.VAlign(VAlign_Fill)
								// 		.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
								// 		.BorderBackgroundColor(FromHex("0000ff"))
								SAssignNew(ServersContainer, SWrapBox)
														.Orientation(EOrientation::Orient_Horizontal)
														.UseAllottedSize(true) //自适应尺寸，避免每个item都独占一行或一列
														.HAlign(HAlign_Left)
							]
						]
					]

					+ SOverlay::Slot()
					  .HAlign(HAlign_Center)
					  .VAlign(VAlign_Center)
					[
						// Circular Throbber
						SAssignNew(ScanningThrobber, SCircularThrobber)
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
	if (!GlobalTickHandle.IsValid())
	{
		GlobalTickHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateRaw(this, &FPixelStreamingManager::Tick), 1 / 60.0f);
	}
	while (!IsEngineExitRequested())
	{
		// Tick app logic
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		FTSTicker::GetCoreTicker().Tick(1 / 60.0f);

		FSlateApplication::Get().Tick();
		FSlateApplication::Get().PumpMessages();
	}
}

void FPixelStreamingManager::InitializeConfig()
{
}

FReply FPixelStreamingManager::PickupFolder()
{
	FString WebServerPath;
	// Prompt the user for the directory
	if (FDesktopPlatformModule::Get()->OpenDirectoryDialog(GetActiveWindow(),
	                                                       LOCTEXT("SelectServerPath",
	                                                               "Please select your web server path.").ToString(),
	                                                       FSettingsConfig::Get().GetWebServersPath(), WebServerPath))
	{
		UE_LOG(LogPixelStreamingManager, Display, TEXT("Web servers path selected at : %s"), *WebServerPath);
		if (PathText.IsValid())
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
	const FString Result = CanDoScan() ? "Execute Scan" : "WebServers Path is invalid,Please check.";
	return FText::FromString(Result);
}

FText FPixelStreamingManager::GenerateLaunchMatchmakerToolTip() const
{
	const FString Result = CanDoScan() && bUseMatchmaker ? "Launch Matchmaker service" : "Matchmaker not enable.";
	return FText::FromString(Result);
}

void FPixelStreamingManager::OnWindowResized()
{
	if (ContainerArea.IsValid() && RightPanelWidth > 0.f)
	{
		float CurWidth = RightPanelWidth - 2 * WrapBoxPadding - ScrollBarThickness - ScrollBarPadding * 2;
		const float DesireWidth = CurWidth - FMath::Fmod(
			CurWidth, ItemWidth + ItemHorizontalPadding * 2);
		UE_LOG(LogPixelStreamingManager, Display, TEXT("Window size changed,wrapbox width resize to :%f ~ %f ."),
		       RightPanelWidth,
		       DesireWidth);
		ContainerArea->SetWidthOverride(DesireWidth);
	}
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

FReply FPixelStreamingManager::LaunchMatchMaker()
{
	UE_LOG(LogPixelStreamingManager, Warning, TEXT("Going to launch matchmaker service..."));
	return FReply::Handled();
}

void FPixelStreamingManager::StartScan()
{
	if (ScanningThrobber.IsValid())
	{
		ScanningThrobber->SetVisibility(EVisibility::SelfHitTestInvisible);
	}

	ScanTask = new FAsyncTask<FDoScanTask>(FSettingsConfig::Get().GetWebServersPath());
	ScanTask->StartBackgroundTask();
	//ScanTask->StartSynchronousTask();
	StartScanTicker();
}

void FPixelStreamingManager::StopBackgroundThread()
{
	if (ScanTask)
	{
		ScanTask->EnsureCompletion();
		delete ScanTask;
		ScanTask = nullptr;
	}
}

void FPixelStreamingManager::StopScan()
{
	// should call in game thread
	if (ScanningThrobber.IsValid())
	{
		ScanningThrobber->SetVisibility(EVisibility::Collapsed);
	}

	bool bServerValid = FSettingsConfig::Get().IsServerValid();
	if (!bServerValid)
	{
		FText Title = FText::FromString("ValidationError");
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ServerValidError", "像素流服务器验证失败，请手动验证"), &Title);
		return;
	}

	if (ServersContainer.IsValid())
	{
		ServersContainer->ClearChildren();

		// base 
		CreateServerItem("Base", 80, false);

		int Count = 14;

		for (int index = 1; index <= Count; ++index)
		{
			CreateServerItem("Copy" + FString::FromInt(index), 80 + index, true);
		}
	}
}

void FPixelStreamingManager::CreateServerItem(FString Name, int32 HttpPort, bool bIsEnable)
{
	ServersContainer->AddSlot()
	                .Padding(ItemHorizontalPadding, ItemVerticalPadding)
	                .ForceNewLine(false)
	[
		SNew(SPSServerSingleton)
				.Width(ItemWidth)
				.Height(ItemHeight)
				.Name(Name)
				.HttpPort(HttpPort)
				.bIsEnable(bIsEnable)
	];
}
