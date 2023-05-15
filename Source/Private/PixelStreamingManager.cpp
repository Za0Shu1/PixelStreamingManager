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
#include <Windows.h>
#include <Shellapi.h>
#include <string>

#include "Async/Async.h"
#include "Common/UPSUtils.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableText.h"
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
	// 添加程序退出事件的监听器
	FCoreDelegates::OnExit.AddRaw(this, &FPixelStreamingManager::ShutDown);
}

FPixelStreamingManager::~FPixelStreamingManager()
{
	ShutDown();
}

void FPixelStreamingManager::ShutDown()
{
	FTSTicker::GetCoreTicker().RemoveTicker(GlobalTickHandle);

	if (bMatchMakerRunningInProgress)
	{
		CloseProcess();
	}
}

void FPixelStreamingManager::CloseProcess()
{
	UPSUtils::Get().TerminateProcessByHandle(HND_Matchmaker);
}

void FPixelStreamingManager::TerminateProcessByHandle(DWORD ProcessID)
{
	std::wstring command = L"taskkill /PID " + std::to_wstring(ProcessID) + L" /T /F";
	STARTUPINFOW startupInfo = { sizeof(STARTUPINFOW) };
	PROCESS_INFORMATION processInfo;

	CreateProcessW(NULL, const_cast<LPWSTR>(command.c_str()), NULL, NULL, false, 0, NULL, NULL, &startupInfo, &processInfo);

	WaitForSingleObject(processInfo.hProcess, INFINITE);

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
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
									.Text_Lambda([this]()
									             {
										             FText _Run(LOCTEXT("LaunchMatchmaker", "启动Matchmaker"));
										             FText _Stop(LOCTEXT("ShutdownMatchmaker", "停止Matchmaker"));
										             return bMatchMakerRunningInProgress ? _Stop : _Run;
									             })
									.IsEnabled_Lambda([this]()
									             {
										             return CanDoScan() && FSettingsConfig::Get().GetUseMatchmaker();
									             })
									.OnClicked_Raw(this, &FPixelStreamingManager::ToggleMatchMaker)
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

								+ SHorizontalBox::Slot()
								  .AutoWidth()
								  .Padding(FMargin(2.f, 0.f, 0.f, 0.f))
								[
									SNew(SButton)
									.Text(FText(LOCTEXT("GetAddress", "获取地址")))
									.Visibility_Lambda([this]()
									             {
										             return bMatchMakerRunningInProgress
											                    ? EVisibility::Visible
											                    : EVisibility::Collapsed;
									             })
									.OnClicked_Raw(this, &FPixelStreamingManager::OnGetAddressRequest)
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
					  .HAlign(HAlign_Fill)
					  .VAlign(VAlign_Fill)
					[
						SAssignNew(LoadingWidget, SOverlay)
						.Visibility(EVisibility::Collapsed)

						+ SOverlay::Slot()
						  .HAlign(HAlign_Fill)
						  .VAlign(VAlign_Fill)
						[
							SNew(SImage)
							.RenderOpacity(0.f)
						]

						+ SOverlay::Slot()
						  .HAlign(HAlign_Center)
						  .VAlign(VAlign_Center)
						[
							// Circular Throbber
							SNew(SCircularThrobber)
												                  .Radius(50.f)
												                  .Period(1.f)
												                  .NumPieces(20)
						]

						+ SOverlay::Slot()
						  .HAlign(HAlign_Center)
						  .VAlign(VAlign_Center)
						[
							SAssignNew(LoadingText, STextBlock)
						]
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
	FString Result;
	if (CanDoScan() && FSettingsConfig::Get().GetUseMatchmaker())
	{
		Result = bMatchMakerRunningInProgress ? "Shutdown matchmaker service" : "Launch matchmaker service";
	}
	else
	{
		Result = "Matchmaker not enable.";
	}
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

FReply FPixelStreamingManager::ToggleMatchMaker()
{
	if (bMatchMakerRunningInProgress)
	{
		// 手动关闭进程
		CloseProcess();
	}
	else
	{
		const FString MatchmakerPath = FSettingsConfig::Get().GetLaunchConfig().MatchMakerBatchPath;

		AsyncTask(ENamedThreads::AnyThread, [MatchmakerPath,this]()
		{
			bMatchMakerRunningInProgress = true;
			LaunchMatchmakerBatchServer(MatchmakerPath, [this]()
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Matchmaker service shutdown."));
				bMatchMakerRunningInProgress = false;
			});
		});
	}

	return FReply::Handled();
}

FReply FPixelStreamingManager::OnGetAddressRequest()
{
	AllocateAccessAddressFromMatchmaker([](FString Response)
	{
		if (Response.Contains("No signalling servers available"))
		{
			FText Title = FText::FromString("Failed");
			FMessageDialog::Open(
				EAppMsgType::Ok,
				LOCTEXT("NoAddress", "暂无可预览地址"),
				&Title);
		}
		else
		{
			FString Addr = Response;
			if (UPSUtils::Get().GetJsonValue(Response, "signallingServer", Addr))
			{

				FText Title = FText::FromString("Success");
				FText Message = FText::Format(LOCTEXT("GetAddressFinished", "访问地址:{0},是否复制到剪贴板?"), FText::FromString(Addr));
					
				EAppReturnType::Type UserChoice = FMessageDialog::Open(
					EAppMsgType::OkCancel,
					Message,
					&Title);
				if (UserChoice == EAppReturnType::Ok)
				{
					// 用户点击了 "OK" 按钮
					UPSUtils::Get().CopyToClipBoard(Addr);
				}
				else if (UserChoice == EAppReturnType::Cancel)
				{
					// 用户点击了 "Cancel" 按钮
				}
				else
				{
					// 用户关闭了对话框或点击了其他按钮类型
				}
			}
			else
			{
				FText Title = FText::FromString("ERROR");
				FMessageDialog::Open(
					EAppMsgType::Ok,
					LOCTEXT("AddressError", "地址异常"),
					&Title);
			}
		}
	});
	return FReply::Handled();
}

void FPixelStreamingManager::AllocateAccessAddressFromMatchmaker(TUniqueFunction<void(FString)> Response)
{
	// 获取HTTP模块并创建HTTP请求
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("GET");
	HttpRequest->SetURL("http://localhost:90/signallingserver");

	GetAddressCallback = MoveTemp(Response);
	// 设置响应处理函数
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
		{
			if (bSuccess && HttpResponse.IsValid())
			{
				// 响应状态码为200表示成功
				if (HttpResponse->GetResponseCode() == 200)
				{
					FString ResponseStr = HttpResponse->GetContentAsString();
					// 处理响应内容
					UE_LOG(LogPixelStreamingManager, Log, TEXT("HTTP GET request success with response: %s"),
					       *ResponseStr);

					UE_LOG(LogPixelStreamingManager, Log, TEXT("Callback : %p"), &GetAddressCallback);

					GetAddressCallback(ResponseStr);
				}
			}
			else
			{
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("HTTP GET request failed"));
			}
		});

	// 发送HTTP请求
	HttpRequest->ProcessRequest();
}

void FPixelStreamingManager::RunBatScriptWithOutput(const FString& BatPath)
{
	// 将FString转换为std::wstring
	std::wstring ScriptPathWStr(*BatPath);

	// 创建匿名管道
	HANDLE hReadPipe, hWritePipe;
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = true;
	saAttr.lpSecurityDescriptor = NULL;


	if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
	{
		UE_LOG(LogPixelStreamingManager, Error, TEXT("Failed to create pipe!"));
		return;
	}

	// 设置启动信息
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = NULL;
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;

	// 创建进程并启动bat脚本
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	FString DirectoryPath = FPaths::GetPath(BatPath);

	// 将FString转换为TCHAR*
	TCHAR* BatchPathTChar = new TCHAR[BatPath.Len() + 1];
	_tcscpy_s(BatchPathTChar, BatPath.Len() + 1, *BatPath);

	// 将FString转换为TCHAR*
	TCHAR* DirectoryPathTChar = new TCHAR[DirectoryPath.Len() + 1];
	_tcscpy_s(DirectoryPathTChar, DirectoryPath.Len() + 1, *DirectoryPath);

	if (!CreateProcess(NULL, const_cast<LPWSTR>(ScriptPathWStr.c_str()), NULL, NULL, true, 0, NULL, DirectoryPathTChar,
	                   &si, &pi))
	{
		UE_LOG(LogPixelStreamingManager, Error, TEXT("Failed to start the bat script!"));
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		return;
	}

	// 关闭无用的写管道句柄
	CloseHandle(hWritePipe);

	// 读取管道输出并打印到控制台
	CHAR buffer[4096];
	DWORD bytesRead;
	std::string output;

	while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) != 0 && bytesRead != 0)
	{
		buffer[bytesRead] = '\0';
		output += buffer;

		// 每次读取到换行符或回车符时，打印输出
		size_t pos = output.find_first_of("\r\n");
		while (pos != std::string::npos)
		{
			if (pos > 0) // 跳过空行
			{
				FString OutputLine(output.substr(0, pos).c_str());
				UE_LOG(LogPixelStreamingManager, Warning, TEXT("%s"), *OutputLine);
			}

			output.erase(0, pos + 1);
			pos = output.find_first_of("\r\n");
		}
	}

	// 关闭管道和进程句柄
	CloseHandle(hReadPipe);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// 释放内存
	delete[] BatchPathTChar;
	delete[] DirectoryPathTChar;
}


void FPixelStreamingManager::LaunchMatchmakerBatchServer(const FString& BatchScriptPath,
                                                         TUniqueFunction<void()> Callback)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Convert FString to LPCWSTR
	const TCHAR* BatPathPtr = *BatchScriptPath;
	LPCWSTR BatPathLPCWSTR = (LPCWSTR)BatPathPtr;

	// Create the process
	if (!::CreateProcessW(NULL, (LPWSTR)BatPathLPCWSTR, NULL, NULL, false, 0, NULL, NULL, &si, &pi))
	{
		UE_LOG(LogPixelStreamingManager, Error, TEXT("Failed to start bat script!"));
		Callback();
		return;
	}

	HND_Matchmaker = pi.hProcess;

	// Wait for the process to finish
	WaitForSingleObject(pi.hProcess, INFINITE);


	// Clean up
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	Callback();
}

void FPixelStreamingManager::StartScan()
{
	ShowLoadingWidget("Scanning...");

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
	HiddenLoadingWidget();

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

	ExistsServer.Empty();
	if (ServersContainer.IsValid())
	{
		ServersContainer->ClearChildren();

		if (!FSettingsConfig::Get().GetLaunchConfig().SingnallingServerConfigPath.IsEmpty())
		{
			SignallingServerConfig config = FileHelper::Get().LoadServerConfigFromJsonFile(
				FSettingsConfig::Get().GetLaunchConfig().SingnallingServerConfigPath / "config.json");
			FBackupServerInfo BaseServerInfo;
			BaseServerInfo.ServerName = AllocServerName(FSettingsConfig::Get().GetProjectName());
			BaseServerInfo.Config = config;

			CreateServerItem(BaseServerInfo, false);
		}

		// load all backup server from json file
		TArray<FBackupServerInfo> Servers = FileHelper::Get().LoadAllBackupServers(
			FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup/Servers.json");
		{
			for (FBackupServerInfo server : Servers)
			{
				UE_LOG(LogPixelStreamingManager, Display, TEXT("Server : %s"), *server.ServerName);
				server.Config = FileHelper::Get().LoadServerConfigFromJsonFile(server.ConfigFilePath);
				CreateServerItem(server, true);
			}
		}
	}
}

void FPixelStreamingManager::CreateServerItem(const FBackupServerInfo& Config, bool bIsBackupServer)
{
	AddExistsServerInformation(Config);

	ServersContainer->AddSlot()
	                .Padding(ItemHorizontalPadding, ItemVerticalPadding)
	                .ForceNewLine(false)
	[
		SNew(SPSServerSingleton)
				.Width(ItemWidth)
				.Height(ItemHeight)
				.Name(Config.ServerName)
				.Config(Config)
				.bIsBackupServer(bIsBackupServer)
				.OnCreateServer_Raw(this, &FPixelStreamingManager::CopyServer)
				.OnDeleteServer_Raw(this, &FPixelStreamingManager::DeleteServer)
				.OnStateChanged_Raw(this, &FPixelStreamingManager::ServerStateChanged)
				.OnRenameServer_Raw(this, &FPixelStreamingManager::RenameServer)
				.OnChangePort_Raw(this, &FPixelStreamingManager::ChangePort)
	];
}

void FPixelStreamingManager::CopyServer(FBackupServerInfo Config, FString Name)
{
	UE_LOG(LogPixelStreamingManager, Display, TEXT("Handle copy server request."));

	const FString CopyFrom = FSettingsConfig::Get().GetLaunchConfig().SingnallingServerConfigPath;
	const FString BackupFolder = FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup";

	ShowLoadingWidget("Copying...");

	Config.ServerName = AllocServerName(Name);
	Config.ConfigFilePath = BackupFolder / Config.ServerName / "config.json";
	Config.SingnallingServerLocalPath = BackupFolder / Config.ServerName / "platform_scripts/cmd/run_local.bat";
	Config.SingnallingServerPublicPath = BackupFolder / Config.ServerName /
		"platform_scripts/cmd/Start_WithTURN_SignallingServer.ps1";
	Config.Config.UseMatchmaker = FSettingsConfig::Get().GetUseMatchmaker();

	AllocPorts(Config.Config);

	FileHelper::Get().CopyFolderRecursively(CopyFrom, BackupFolder, Config.ServerName, true,
	                                        [this,BackupFolder,Config](bool result)
	                                        {
		                                        if (result)
		                                        {
			                                        UE_LOG(LogPixelStreamingManager, Display, TEXT("Server Copied."));

			                                        if (FileHelper::Get().UpdateServerConfigIntoJsonFile(
				                                        Config.ConfigFilePath, Config.Config))
			                                        {
				                                        CreateServerItem(Config, true);
				                                        FileHelper::Get().AddServerIntoConfig(Config);
			                                        }
			                                        else
			                                        {
				                                        FText Title = FText::FromString("UpdateFailed");
				                                        FMessageDialog::Open(
					                                        EAppMsgType::Ok, LOCTEXT("UpdateError", "初始化服务器配置失败."),
					                                        &Title);
				                                        const FString ServerFolder = BackupFolder / Config.ServerName;
				                                        FileHelper::Get().DeleteFolder(ServerFolder, nullptr);
			                                        }
		                                        }
		                                        else
		                                        {
			                                        FText Title = FText::FromString("CopyFailed");

			                                        FMessageDialog::Open(
				                                        EAppMsgType::Ok, LOCTEXT("CopyError", "拷贝信令服务器失败."),
				                                        &Title);
		                                        }
		                                        HiddenLoadingWidget();
	                                        });
}

void FPixelStreamingManager::DeleteServer(FBackupServerInfo Config, FString Name, SPSServerSingleton* Target)
{
	UE_LOG(LogPixelStreamingManager, Display, TEXT("Handle delete server request."));
	FileHelper::Get().DeleteFolder(FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup" / Name, []()
	{
		FText Title = FText::FromString("DeleteFailed");
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("DeleteError", "未彻底删除文件夹，请手动删除"),
		                     &Title);
	});

	FileHelper::Get().DeleteServerFromConfig(Name);
	RemoveExistsServerInformation(Config);

	if (ServersContainer.IsValid())
	{
		ServersContainer->RemoveSlot(StaticCastSharedRef<SPSServerSingleton>(Target->AsShared()));
	}
}

void FPixelStreamingManager::ServerStateChanged(FBackupServerInfo Config, EServerState State)
{
}

void FPixelStreamingManager::RenameServer(SPSServerSingleton* Target, FBackupServerInfo Config, FString NewName)
{
	UE_LOG(LogPixelStreamingManager, Display, TEXT("Rename server from %s to %s."), *Config.ServerName, *NewName);

	FString AllocatedName = AllocServerName(NewName);
	FBackupServerInfo NewConfig = Config;
	NewConfig.ServerName = AllocatedName;

	// update folder name
	FileHelper::Get().RenameFolder(FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup" / Config.ServerName,
	                               FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup" / AllocatedName,
	                               [=](bool bRenameResult)
	                               {
		                               if (bRenameResult)
		                               {
			                               // update json config server name
			                               FileHelper::Get().ModifyServerFromConfig(Config.ServerName, NewName);

			                               // update server display text,it could be another name instead of given name
			                               if (Target && Target->ServerName.IsValid())
			                               {
				                               Target->ServerName.Get()->SetText(FText::FromString(AllocatedName));
			                               }

			                               // update exists server name container
			                               UpdateExistsServerInformation(Config, NewConfig);
		                               }
		                               else
		                               {
			                               UE_LOG(LogPixelStreamingManager, Display, TEXT("Rename Folder Failed..."));
			                               if (Target && Target->ServerName.IsValid())
			                               {
				                               Target->ServerName.Get()->SetText(FText::FromString(Config.ServerName));
			                               }
		                               }
	                               });
}

void FPixelStreamingManager::ChangePort(FBackupServerInfo& Config, EPortType PortType, FString& NewPort,
                                        uint16& OldPort)
{
	if (IsPortAvailable(NewPort))
	{
		int TempPort = FCString::Atoi(*NewPort);

		switch (PortType)
		{
		case EPortType::E_Http:
			Config.Config.HttpPort = TempPort;
			break;
		case EPortType::E_SFU:
			Config.Config.SFUPort = TempPort;
			break;
		case EPortType::E_Streamer:
			Config.Config.StreamerPort = TempPort;
			break;
		}

		if (!FileHelper::Get().UpdateServerConfigIntoJsonFile(Config.ConfigFilePath, Config.Config))
		{
			NewPort = FString::FromInt(OldPort);
			return;
		}
		OldPort = TempPort;
		ExistsServer.Add(Config.ServerName, Config);
	}
	else
	{
		NewPort = FString::FromInt(OldPort);
	}
}

void FPixelStreamingManager::ShowLoadingWidget(FString DisplayName)
{
	if (LoadingWidget.IsValid())
	{
		if (LoadingText.IsValid())
		{
			LoadingText->SetText(FText::FromString(DisplayName));
		}

		LoadingWidget->SetVisibility(EVisibility::SelfHitTestInvisible);
	}
}

void FPixelStreamingManager::HiddenLoadingWidget()
{
	if (LoadingWidget.IsValid())
	{
		LoadingWidget->SetVisibility(EVisibility::Collapsed);
	}
}

FString FPixelStreamingManager::AllocServerName(FString InPreferName, int SuffixIndex)
{
	FString Result = InPreferName;
	if (ExistsServer.Contains(Result))
	{
		FString TargetName;
		if (SuffixIndex <= 0)
		{
			// base -> base_1
			SuffixIndex = 1;
			TargetName = Result + "_" + FString::FromInt(SuffixIndex);
		}
		else
		{
			//base_1 -> base_2
			FString leftS, rightS;
			Result.Split("_", &leftS, &rightS, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			TargetName = leftS + "_" + FString::FromInt(SuffixIndex);
		}

		Result = AllocServerName(TargetName, ++SuffixIndex);
	}
	return Result;
}

void FPixelStreamingManager::AllocPorts(SignallingServerConfig& Config)
{
	TArray<int> ExistsHttpPorts, ExistsSFUPorts, ExistsStreamerPorts, AllExistsPorts;
	for (auto a : ExistsServer)
	{
		ExistsHttpPorts.Add(a.Value.Config.HttpPort);
		ExistsSFUPorts.Add(a.Value.Config.SFUPort);
		ExistsStreamerPorts.Add(a.Value.Config.StreamerPort);

		AllExistsPorts.Add(a.Value.Config.HttpPort);
		AllExistsPorts.Add(a.Value.Config.SFUPort);
		AllExistsPorts.Add(a.Value.Config.StreamerPort);
	}

	ExistsHttpPorts.StableSort();
	ExistsSFUPorts.StableSort();
	ExistsStreamerPorts.StableSort();
	int _http = ExistsHttpPorts.Last();
	int _sfu = ExistsSFUPorts.Last();
	int _streamer = ExistsStreamerPorts.Last();


	do
	{
		_http++;
	}
	while (AllExistsPorts.Contains(_http));
	Config.HttpPort = _http;
	AllExistsPorts.Add(_http);

	do
	{
		_sfu++;
	}
	while (AllExistsPorts.Contains(_sfu));
	Config.SFUPort = _sfu;
	AllExistsPorts.Add(_sfu);

	do
	{
		_streamer++;
	}
	while (AllExistsPorts.Contains(_streamer));
	Config.StreamerPort = _streamer;
	AllExistsPorts.Add(_streamer);
	Config.PublicIp = FSettingsConfig::Get().GetIsPublic() ? FSettingsConfig::Get().GetPublicIP() : "localhost";
}

bool FPixelStreamingManager::IsPortAvailable(FString& Port)
{
	if (!Port.IsNumeric())
	{
		UE_LOG(LogPixelStreamingManager, Error, TEXT("Invalid Port!"));
		return false;
	}

	int NewPort = FCString::Atoi(*Port);
	if (NewPort > 65535 || NewPort < 0)
	{
		NewPort = 65535;
	}

	TArray<int> AllExistsPorts;
	for (auto a : ExistsServer)
	{
		AllExistsPorts.Add(a.Value.Config.HttpPort);
		AllExistsPorts.Add(a.Value.Config.SFUPort);
		AllExistsPorts.Add(a.Value.Config.StreamerPort);
	}
	if (!AllExistsPorts.Contains(NewPort))
	{
		Port = FString::FromInt(NewPort);
		return true;
	}

	return false;
}

void FPixelStreamingManager::AddExistsServerInformation(FBackupServerInfo Info)
{
	ExistsServer.Add(Info.ServerName, Info);
}

void FPixelStreamingManager::RemoveExistsServerInformation(FBackupServerInfo Info)
{
	ExistsServer.Remove(Info.ServerName);
}

void FPixelStreamingManager::UpdateExistsServerInformation(const FBackupServerInfo& From, const FBackupServerInfo& To)
{
	for (auto pair : ExistsServer)
	{
		if (*pair.Key == From.ServerName)
		{
			ExistsServer.Add(*pair.Key, To);
			break;
		}
	}
}


#undef LOCTEXT_NAMESPACE
