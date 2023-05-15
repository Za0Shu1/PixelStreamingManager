#include "SPSServerSingleton.h"

#include "CommonStyle.h"
#include "SlateOptMacros.h"
#include "Async/Async.h"
#include "Common/STextProperty.h"
#include "Common/UPSUtils.h"
#include "Misc/CoreDelegates.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/SBoxPanel.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

DEFINE_LOG_CATEGORY(LogPSServer);

#define LOCTEXT_NAMESPACE "SPSServerSingleton"
#define FromHex(Hex) FLinearColor::FromSRGBColor(FColor::FromHex(Hex))

SPSServerSingleton::SPSServerSingleton()
{
	FCoreDelegates::OnExit.AddLambda([this]()
	{
		CloseServerHandle();
	});
}

SPSServerSingleton::~SPSServerSingleton()
{
}

void SPSServerSingleton::Construct(const FArguments& InArgs)
{
	bIsBackupServer = InArgs._bIsBackupServer;
	Name = InArgs._Name;
	Width = InArgs._Width;
	Height = InArgs._Height;
	State = EServerState::E_Stop;
	Config = InArgs._Config;
	HttpPort = InArgs._Config.Config.HttpPort;
	SFUPort = InArgs._Config.Config.SFUPort;
	StreamerPort = InArgs._Config.Config.StreamerPort;

	OnStateChanged = InArgs._OnStateChanged;
	OnCreateServer = InArgs._OnCreateServer;
	OnDeleteServer = InArgs._OnDeleteServer;
	OnServerRename = InArgs._OnRenameServer;
	OnChangePort = InArgs._OnChangePort;

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(Width)
		.HeightOverride(Height)
		[
			SNew(SOverlay)

			// background
			+ SOverlay::Slot()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FromHex("#3399ff"))
				.Padding(2.f)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("WhiteBrush"))
					.ColorAndOpacity(FSlateColor(FromHex("#313131")))
				]
			]

			+ SOverlay::Slot()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			  .Padding(2.f)
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				  .VAlign(VAlign_Top)
				  .HAlign(HAlign_Left)
				  .Padding(0.f)
				[
					SNew(SButton)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Visibility(bIsBackupServer ? EVisibility::Visible : EVisibility::Collapsed)
					.ToolTipText(FText::FromString("Launch"))
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					// onclick
					.IsEnabled_Lambda([this]()
					             {
						             return GetIsEnabled() || State == EServerState::E_Running;
					             })
					.OnClicked_Raw(this, &SPSServerSingleton::OnButtonClick)
					[
						SNew(SImage)
						// icon
						.Image_Lambda([this]()
						{
							bool bIsRunning = (State == EServerState::E_Running);
							return FPSManagerStyle::Get().GetBrush(bIsRunning ? TEXT("Stop") : TEXT("Run"));
						})
					]
				]

				// Name
				+ SOverlay::Slot()
				  .HAlign(HAlign_Center)
				  .VAlign(VAlign_Top)
				  .Padding(FMargin(0, 10.f, 0, 0))
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FromHex("#6161615B"))
					[
						SAssignNew(ServerName, SEditableText)
						.IsEnabled_Raw(this, &SPSServerSingleton::GetIsEnabled)
						.Text(FText::FromString(Name))
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
						.ColorAndOpacity(FromHex("#3399ff"))
						.SelectAllTextWhenFocused(true)
						.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type Type)
						                                     {
							                                     if (Name != Text.ToString())
							                                     {
								                                     OnServerRename.ExecuteIfBound(
									                                     this, Config, Text.ToString());
								                                     Name = Text.ToString();
							                                     }
						                                     })
						.ToolTipText_Lambda([this]()
						                                     {
							                                     return ServerName->GetText();
						                                     })
					]
				]
#pragma region operations
				// Operations
				+ SOverlay::Slot()
				  .HAlign(HAlign_Right)
				  .VAlign(VAlign_Top)
				  .Padding(FMargin(0.f))
				[
					// copy button
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.MaxDesiredHeight(32.f)
						.MaxDesiredWidth(32.f)
						.Padding(FMargin(2.f, 5.f, 2.f, 0.f))
						[
							SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.ButtonStyle(FPSManagerStyle::Get(),TEXT("CopyButton"))
							.Visibility(EVisibility::Visible)
							.ToolTipText(FText::FromString("Copy"))
							.IsEnabled_Lambda([this]()
							             {
								             return !bIsBackupServer || State != EServerState::E_Running;
							             })
							 .OnClicked_Lambda([this]()
							             {
								             OnCreateServer.ExecuteIfBound(Config, Name);
								             return FReply::Handled();
							             })
						]
					]

					// delete button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.MaxDesiredHeight(32.f)
						.MaxDesiredWidth(32.f)
						.Padding(FMargin(2.f, 5.f, 2.f, 0.f))
						[
							SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.ButtonStyle(FPSManagerStyle::Get(),TEXT("DeleteButton"))
							.Visibility(bIsBackupServer ? EVisibility::Visible : EVisibility::Collapsed)
							.ToolTipText(FText::FromString("Delete"))
							.IsEnabled_Lambda([this]()
							             {
								             return State != EServerState::E_Running;
							             })
				             .OnClicked_Lambda([this]()
							             {
								             OnDeleteServer.ExecuteIfBound(Config, Name, this);
								             return FReply::Handled();
							             })
						]
					]

					// preview button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.MaxDesiredHeight(32.f)
						.MaxDesiredWidth(32.f)
						.Padding(FMargin(2.f, 5.f, 2.f, 0.f))
						[
							SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.ButtonStyle(FPSManagerStyle::Get(),TEXT("PreviewButton"))
							.ToolTipText(FText::FromString("Preview"))
							.Visibility_Lambda([this]()
							             {
								             return bIsBackupServer && State == EServerState::E_Running
									                    ? EVisibility::Visible
									                    : EVisibility::Collapsed;
							             })

							 .OnClicked_Lambda([this]()
							             {
								             const FString URL = GetPreviewURL();
								             UE_LOG(LogPSServer, Display, TEXT("Launch URL : %s"), *URL);
								             FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
								             return FReply::Handled();
							             })
						]

					]

				]
#pragma endregion
				// Ports
#pragma region Ports
				+ SOverlay::Slot()
				  .HAlign(HAlign_Fill)
				  .VAlign(VAlign_Fill)
				  .Padding(FMargin(40.F, 40.f, 40.f, 10.f))
				[
					SNew(SVerticalBox)

					// http port
					+ SVerticalBox::Slot()
					  .AutoHeight()
					  .Padding(FMargin(2.f))
					[
						SAssignNew(HttpPortText, STextProperty)
						.Key("Http Port")
						.Value(FString::FromInt(HttpPort))
						.IsEnabled_Raw(this, &SPSServerSingleton::GetIsEnabled)
						.LeftWidth(100.f)
						.OnValueChanged_Lambda([this](FString& NewPort)
						{
							OnChangePort.ExecuteIfBound(Config, EPortType::E_Http, NewPort, HttpPort);
						})
					]

					// sfu port
					+ SVerticalBox::Slot()
					  .AutoHeight()
					  .Padding(FMargin(2.f))
					[
						SAssignNew(SFUPortText, STextProperty)
						.Key("SFU Port")
						.Value(FString::FromInt(SFUPort))
						.IsEnabled_Raw(this, &SPSServerSingleton::GetIsEnabled)
						.LeftWidth(100.f)
						.OnValueChanged_Lambda([this](FString& NewPort)
						{
							OnChangePort.ExecuteIfBound(Config, EPortType::E_SFU, NewPort, SFUPort);
						})
					]

					// streamer port
					+ SVerticalBox::Slot()
					  .AutoHeight()
					  .Padding(FMargin(2.f))
					[
						SAssignNew(StreamerPortText, STextProperty)
						.Key("Streamer Port")
						.Value(FString::FromInt(StreamerPort))
						.IsEnabled_Raw(this, &SPSServerSingleton::GetIsEnabled)
						.LeftWidth(100.f)
						.OnValueChanged_Lambda([this](FString& NewPort)
						{
							OnChangePort.ExecuteIfBound(Config, EPortType::E_Streamer, NewPort, StreamerPort);
						})
					]

				]
#pragma endregion

			]
		]
	];
}

bool SPSServerSingleton::GetIsEnabled() const
{
	return bIsBackupServer && State != EServerState::E_Running;
}

FString SPSServerSingleton::GetPreviewURL()
{
	FString URL;
	FString Protocol = Config.Config.UseHTTPS ? "https://" : "http://";
	uint16 Port = Config.Config.UseHTTPS ? Config.Config.HttpsPort : Config.Config.HttpPort;

	FString IP = Config.Config.PublicIp;
	if (Config.Config.PublicIp == "localhost")
	{
		IP = "127.0.0.1";
	}
	
	if(!UPSUtils::Get().IsIPAddress(IP))
	{
		IP = "";
		UE_LOG(LogPSServer,Error,TEXT("Invalid IPV4 address for public IP."))
	}

	return Protocol + IP + ":" + FString::FromInt(Port);
}

void SPSServerSingleton::CloseServerHandle()
{
	if (HND_SingallingServer != NULL && HND_SingallingServer != INVALID_HANDLE_VALUE)
	{
		UPSUtils::Get().TerminateProcessByHandle(HND_SingallingServer);
	}
	else if (HND_UnrealClient != NULL && HND_UnrealClient != INVALID_HANDLE_VALUE)
	{
		UPSUtils::Get().TerminateProcessByHandle(HND_UnrealClient);
	}
	
	State = EServerState::E_Stop;
}

FReply SPSServerSingleton::OnButtonClick()
{
	if (State == EServerState::E_Stop)
	{
		// RUN SIGNALLING SERVER
		const FString RunSingallingServerPath = FSettingsConfig::Get().GetIsPublic()
			                                        ? Config.SingnallingServerPublicPath
			                                        : Config.SingnallingServerLocalPath;

		AsyncTask(ENamedThreads::AnyThread, [&,RunSingallingServerPath,this]()
		{
			RunServerScript(RunSingallingServerPath, [&,this](int RetCode)
			{
				switch (RetCode)
				{
				case -1:
					UE_LOG(LogPSServer, Error, TEXT("Can not launch signalling server."));
					break;
				case 0:
					UE_LOG(LogPSServer, Display, TEXT("Singalling server shutdown."));
					
					if (HND_UnrealClient != NULL && HND_UnrealClient != INVALID_HANDLE_VALUE)
					{
						UPSUtils::Get().TerminateProcessByHandle(HND_UnrealClient);
					}
					State = EServerState::E_Stop;
					break;
				case 1:
					UE_LOG(LogPSServer, Display, TEXT("Singalling server is running..."));
					if (GetIsValidHandle(HND_UnrealClient))
					{
						State = EServerState::E_Running;
					}
					break;
				}
			});
		});

		// RUN UNREAL CLIENT
		const FString ExtraCommands = FSettingsConfig::Get().GetExtraCommands() + " -PixelStreamingIP=" + Config.Config.
			PublicIp + " -PixelStreamingPort=" + FString::FromInt(Config.Config.StreamerPort);
		UE_LOG(LogPSServer, Display, TEXT("commands: ===%s"), *ExtraCommands);

		AsyncTask(ENamedThreads::AnyThread, [&,ExtraCommands,this]()
		{
			RunUnrealClient(FSettingsConfig::Get().GetClientPath(), ExtraCommands, [&,this](int RetCode)
			{
				switch (RetCode)
				{
				case -1:
					UE_LOG(LogPSServer, Error, TEXT("Can not launch unreal client."));
					break;

				case 0:
					UE_LOG(LogPSServer, Display, TEXT("Unreal client shutdown."));
					if (HND_SingallingServer != NULL && HND_SingallingServer != INVALID_HANDLE_VALUE)
					{
						UPSUtils::Get().TerminateProcessByHandle(HND_SingallingServer);
					}
					State = EServerState::E_Stop;
					break;

				case 1:
					UE_LOG(LogPSServer, Display, TEXT("Unreal client is running..."));
					if (GetIsValidHandle(HND_SingallingServer))
					{
						State = EServerState::E_Running;
					}
					break;
				}
			});
		});
	}
	else
	{
		if (State == EServerState::E_Running)
		{
			CloseServerHandle();
		}
	}
	OnStateChanged.ExecuteIfBound(Config, State);
	return FReply::Handled();
}

// only support .bat or .ps1 file
// callback code -1:launch failed; 0:process closed; 1:process running
void SPSServerSingleton::RunServerScript(const FString& ScriptPath,
                                         TUniqueFunction<void(int)> Callback)
{
	// Check if the script file has a valid extension
	if (!(ScriptPath.EndsWith(".bat") || ScriptPath.EndsWith(".ps1")))
	{
		UE_LOG(LogPSServer, Warning, TEXT("Only support .bat or .ps1 script file."));
		Callback(-1);
		return;
	}

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	FString ScriptArgument;

	// Determine the command interpreter and script argument based on the file extension
	if (ScriptPath.EndsWith(".bat"))
	{
		ScriptArgument = FString::Printf(TEXT("cmd.exe /c \"%s\""), *ScriptPath);
	}
	else if (ScriptPath.EndsWith(".ps1"))
	{
		ScriptArgument = FString::Printf(TEXT("powershell.exe -ExecutionPolicy Bypass -File \"%s\""), *ScriptPath);
	}

	// Convert the strings to LPCWSTR
	LPCWSTR ScriptArgumentLPCWSTR = (LPCWSTR)*ScriptArgument;

	UE_LOG(LogPSServer, Warning, TEXT("====  %s"), *ScriptArgument);

	// Create the process
	if (!::CreateProcessW(NULL, (LPWSTR)ScriptArgumentLPCWSTR, NULL, NULL, false, 0, NULL, NULL,
	                      &si, &pi))
	{
		UE_LOG(LogPSServer, Error, TEXT("Failed to start script!"));
		Callback(-1);
		return;
	}

	HND_SingallingServer = pi.hProcess;
	Callback(1);

	// Wait for the process to finish
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Clean up
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	HND_SingallingServer = INVALID_HANDLE_VALUE;
	Callback(0);
}

// callback code -1:launch failed; 0:process closed; 1:process running
void SPSServerSingleton::RunUnrealClient(const FString& ExePath, const FString& ExtraCommands,
                                         TUniqueFunction<void(int)> Callback)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	FString ScriptArgument = FString::Printf(TEXT("%s %s"), *ExePath, *ExtraCommands);

	// Convert the strings to LPCWSTR
	LPCWSTR ScriptArgumentLPCWSTR = (LPCWSTR)*ScriptArgument;

	UE_LOG(LogPSServer, Display, TEXT("====  %s"), *ScriptArgument);

	// Create the process
	if (!::CreateProcessW(NULL, (LPWSTR)ScriptArgumentLPCWSTR, NULL, NULL, false, 0, NULL, NULL,
	                      &si, &pi))
	{
		UE_LOG(LogPSServer, Error, TEXT("Failed to start script!"));
		Callback(-1);
		return;
	}

	HND_UnrealClient = pi.hProcess;

	Callback(1);

	// Wait for the process to finish
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Clean up
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	HND_UnrealClient = INVALID_HANDLE_VALUE;
	Callback(0);
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
