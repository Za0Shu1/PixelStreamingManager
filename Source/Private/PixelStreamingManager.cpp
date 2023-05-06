// Copyright Epic Games, Inc. All Rights Reserved.


#include "PixelStreamingManager.h"

#include "CommonStyle.h"
#include "FileHelper.h"
#include "FSettingsConfig.h"
#include "SPSServerSingleton.h"
#include "Common/SBooleanProperty.h"
#include "Common/SPathProperty.h"
#include "Common/STextProperty.h"
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
	: ScanWebServersFolder(WebServersPath)
{
}

void FDoScanTask::DoWork()
{
	UE_LOG(LogPixelStreamingManager, Display, TEXT("Scanning Directory : %s "), *ScanWebServersFolder);

	// Search webservers
	IFileManager::Get().IterateDirectoryRecursively(*ScanWebServersFolder, [this](const TCHAR* Path, bool bIsDirectory)
	{
		FString FilePath(Path);

		if (FilePath.Contains("Backup"))
		{
			// skip server copy
			return true;
		}

		if (!bIsDirectory)
		{
			FPaths::NormalizeFilename(FilePath);
			if (FilePath.EndsWith("run.bat") && FilePath.Contains("Matchmaker"))
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Matchmaker Launch File Found at { %s }"), *FilePath);
				FSettingsConfig::Get().GetLaunchConfig().MatchMakerBatchPath = FilePath;
				FSettingsConfig::Get().ValidServer("run.bat");
			}

			if (FilePath.EndsWith("run_local.bat") && FilePath.Contains("SignallingWebServer"))
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("SignallingWebServer Local Launch File Found at { %s }"),
				       *FilePath);
				FSettingsConfig::Get().GetLaunchConfig().SingnallingServerLocalPath = FilePath;
				FSettingsConfig::Get().ValidServer("run_local.bat");
			}

			if (FilePath.EndsWith("Start_WithTURN_SignallingServer.ps1") && FilePath.Contains("SignallingWebServer"))
			{
				UE_LOG(LogPixelStreamingManager, Display,
				       TEXT("SignallingWebServer Public Launch File Found at { %s }"), *FilePath);
				FSettingsConfig::Get().GetLaunchConfig().SingnallingServerPublicPath = FilePath;
				FSettingsConfig::Get().ValidServer("Start_WithTURN_SignallingServer.ps1");
			}

			if (FilePath.Contains("turnserver.exe"))
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Turn Server APP Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("turnserver.exe");
			}
			else if (FilePath.Contains("node.exe"))
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Node APP Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("node.exe");
			}
		}
		else
		{
			FPaths::NormalizeFilename(FilePath);
			if (FilePath.EndsWith("Matchmaker"))
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Directory Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("Matchmaker");
			}
			else if (FilePath.EndsWith("SFU"))
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Directory Found at { %s }"), *FilePath);
				FSettingsConfig::Get().ValidServer("SFU");
			}
			else if (FilePath.EndsWith("SignallingWebServer"))
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Directory Found at { %s }"), *FilePath);
				FSettingsConfig::Get().GetLaunchConfig().SingnallingServerConfigPath = FilePath;
				FSettingsConfig::Get().ValidServer("SignallingWebServer");
			}
			else if (FilePath.EndsWith("WebServers"))
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Directory Found at { %s }"), *FilePath);
				FSettingsConfig::Get().GetLaunchConfig().ServersRoot = FilePath;
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
			UE_LOG(LogPixelStreamingManager, Display, TEXT("Scan task finished, destroy background thread."));
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

	// build the window
#pragma region Build Window
	const TSharedPtr<SWindow> MainWindow =
		SNew(SWindow)
				.Title(TitleText)
				.ClientSize(FVector2D(1280, 720))
				.IsInitiallyMaximized(false)
		[
			SNew(SOverlay)


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
						  .Padding(FMargin(10.f, 5.f))
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

							// 像素流服务器路径
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Fill)
							  .VAlign(VAlign_Fill)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(SPathProperty)
								.Key(TEXT("Servers Path"))
								.Value(FSettingsConfig::Get().GetWebServersPath())
								.LeftWidth(100)
								.IsEnabled_Lambda([this]()
								                   {
									                   return true;
								                   })
								.OnValueChanged_Lambda([this](FString NewPath)
								                   {
									                   FSettingsConfig::Get().SetWebServersPath(NewPath);
									                   UE_LOG(LogPixelStreamingManager, Display,
									                          TEXT("WebServers path changed to %s ."), *NewPath);
								                   })
							]
							// 客户端路径
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Fill)
							  .VAlign(VAlign_Fill)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(SPathProperty)
								.Key(TEXT("Client Path"))
								.Value(FSettingsConfig::Get().GetClientPath())
								.LeftWidth(100)
								.IsEnabled_Lambda([this]()
								                   {
									                   return true;
								                   })
								.bPickupFile(true)
								.AllowFileTypes(TEXT("Exe file (*.exe)|*.exe"))
								.OnValueChanged_Lambda([this](FString NewPath)
								                   {
									                   FSettingsConfig::Get().SetClientPath(NewPath);
									                   UE_LOG(LogPixelStreamingManager, Display,
									                          TEXT("Client path changed to %s ."), *NewPath);
								                   })
							]
							// 公网IP
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Fill)
							  .VAlign(VAlign_Fill)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(STextProperty)
								.Key(TEXT("Public IP"))
								.Value(FSettingsConfig::Get().GetPublicIP())
								.LeftWidth(100)
								.IsEnabled_Lambda([this]()
								                   {
									                   return FSettingsConfig::Get().GetIsPublic();
								                   })
								.OnValueChanged_Lambda([this](FString NewValue)
								                   {
									                   FSettingsConfig::Get().SetPublicIP(NewValue);
									                   UE_LOG(LogPixelStreamingManager, Display,
									                          TEXT("Public changed to %s ."), *NewValue);
								                   })
							]

							// 项目名称
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Fill)
							  .VAlign(VAlign_Fill)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(STextProperty)
								.Key(TEXT("Project Name"))
								.Value(FSettingsConfig::Get().GetProjectName())
								.LeftWidth(100)
								.IsEnabled_Lambda([this]()
								                   {
									                   return true;
								                   })
								.OnValueChanged_Lambda([this](FString NewValue)
								                   {
									                   FSettingsConfig::Get().SetProjectName(NewValue);
									                   UE_LOG(LogPixelStreamingManager, Display,
									                          TEXT("Project name changed to %s ."), *NewValue);
								                   })
							]

							// 客户端启动附加参数
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Fill)
							  .VAlign(VAlign_Fill)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(STextProperty)
								.Key(TEXT("Extra commands"))
								.Value(FSettingsConfig::Get().GetExtraCommands())
								.LeftWidth(100)
								.IsEnabled_Lambda([this]()
								                   {
									                   return true;
								                   })
								.OnValueChanged_Lambda([this](FString NewValue)
								                   {
									                   FSettingsConfig::Get().SetExtraCommands(NewValue);
									                   UE_LOG(LogPixelStreamingManager, Display,
									                          TEXT("Extra commands changed to %s ."), *NewValue);
								                   })
							]

							// 是否公网部署
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Fill)
							  .VAlign(VAlign_Fill)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(SBooleanProperty)
								.Key(TEXT("bIsPublic"))
								.Value(FSettingsConfig::Get().GetIsPublic())
								.LeftWidth(100)
								.IsEnabled_Lambda([this]()
								                      {
									                      return true;
								                      })
								.OnValueChanged_Lambda([this](bool NewValue)
								                      {
									                      FSettingsConfig::Get().SetIsPublic(NewValue);
									                      FString OutInfo = NewValue ? "true" : "false";
									                      UE_LOG(LogPixelStreamingManager, Display,
									                             TEXT("bIsPublic changed to %s ."), *OutInfo);
								                      })
							]

							// 是否启用Matchmaker
							+ SVerticalBox::Slot()
							  .HAlign(HAlign_Fill)
							  .VAlign(VAlign_Fill)
							  .AutoHeight()
							  .Padding(FMargin(FVector4f(0, 20, 0, 0)))
							[
								SNew(SBooleanProperty)
								.Key(TEXT("bUseMatchmaker"))
								.Value(FSettingsConfig::Get().GetUseMatchmaker())
								.LeftWidth(100)
								.IsEnabled_Lambda([this]()
								                      {
									                      return true;
								                      })
								.OnValueChanged_Lambda([this](bool NewValue)
								                      {
									                      FSettingsConfig::Get().SetUseMatchmaker(NewValue);
									                      FString OutInfo = NewValue ? "true" : "false";
									                      UE_LOG(LogPixelStreamingManager, Display,
									                             TEXT("bUseMatchmaker changed to %s ."), *OutInfo);
								                      })
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
										             return CanDoScan() && FSettingsConfig::Get().GetUseMatchmaker();
									             })
									.OnClicked_Raw(this, &FPixelStreamingManager::LaunchMatchMaker)
									.ToolTipText_Raw(this, &FPixelStreamingManager::GenerateLaunchMatchmakerToolTip)
								]

								+ SHorizontalBox::Slot()
								  .AutoWidth()
								  .Padding(FMargin(2.f, 0.f, 0.f, 0.f))
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
						.BorderImage(FPSManagerStyle::Get().GetBrush(TEXT("Background")))
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

FText FPixelStreamingManager::GenerateScanToolTip() const
{
	const FString Result = CanDoScan() ? "Execute Scan" : "WebServers Path is invalid,Please check.";
	return FText::FromString(Result);
}

FText FPixelStreamingManager::GenerateLaunchMatchmakerToolTip() const
{
	const FString Result = CanDoScan() && FSettingsConfig::Get().GetUseMatchmaker()
		                       ? "Launch Matchmaker service"
		                       : "Matchmaker not enable.";
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
	UE_LOG(LogPixelStreamingManager, Display, TEXT("Going to launch matchmaker service..."));
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

	bool bClientVaild = FSettingsConfig::Get().IsClientValid();
	if (!bClientVaild)
	{
		FText Title = FText::FromString("ValidationError");
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ClientValidError", "像素流Unreal客户端验证失败，请重新指定"), &Title);
		return;
	}

	bool bServerValid = FSettingsConfig::Get().IsServerValid();
	if (!bServerValid)
	{
		FText Title = FText::FromString("ValidationError");
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ServerValidError", "像素流服务器验证失败，请重新指定"), &Title);
		return;
	}

	if (ServersContainer.IsValid())
	{
		ServersContainer->ClearChildren();

		if (!FSettingsConfig::Get().GetLaunchConfig().SingnallingServerConfigPath.IsEmpty())
		{
			SignallingServerConfig config = FileHelper::Get().LoadServerConfigFromJsonFile(
				FSettingsConfig::Get().GetLaunchConfig().SingnallingServerConfigPath / "config.json");

			CreateServerItem(FSettingsConfig::Get().GetProjectName(), config, false);
		}
		// @todo: search other copy server
	}
}

void FPixelStreamingManager::CreateServerItem(FString Name, const SignallingServerConfig& Config, bool bIsBackupServer)
{
	ServersContainer->AddSlot()
	                .Padding(ItemHorizontalPadding, ItemVerticalPadding)
	                .ForceNewLine(false)
	[
		SNew(SPSServerSingleton)
				.Width(ItemWidth)
				.Height(ItemHeight)
				.Name(Name)
				.Config(Config)
				.bIsBackupServer(bIsBackupServer)
				.OnCreateServer_Raw(this, &FPixelStreamingManager::CopyServer)
				.OnDeleteServer_Raw(this, &FPixelStreamingManager::DeleteServer)
				.OnStateChanged_Raw(this, &FPixelStreamingManager::ServerStateChanged)
	];
}

void FPixelStreamingManager::CopyServer(SignallingServerConfig Config, FString Name)
{
	FileHelper::Get().CopyFolderRecursively(
		FSettingsConfig::Get().GetLaunchConfig().SingnallingServerConfigPath,
		FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup",TEXT("Copy1"), true, [this,Config](bool result)
		{
			if (result)
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Server File Copied."));
				CreateServerItem(FSettingsConfig::Get().GetProjectName() + "1", Config, true);
			}
			else
			{
				FText Title = FText::FromString("CopyTaskResult");
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CopyError", "拷贝信令服务器失败."),
				                     &Title);
			}
		});
}

void FPixelStreamingManager::DeleteServer(SignallingServerConfig Config, FString Name,SPSServerSingleton* Target)
{
	if(ServersContainer.IsValid())
	{
		ServersContainer->RemoveSlot(StaticCastSharedRef<SPSServerSingleton>(Target->AsShared()));
	}
}

void FPixelStreamingManager::ServerStateChanged(SignallingServerConfig Config, EServerState State)
{
	
}

#undef LOCTEXT_NAMESPACE
