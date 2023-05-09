﻿#include "SPSServerSingleton.h"

#include "CommonStyle.h"
#include "SlateOptMacros.h"
#include "Common/STextProperty.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/SBoxPanel.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

#define LOCTEXT_NAMESPACE "SPSServerSingleton"
#define FromHex(Hex) FLinearColor::FromSRGBColor(FColor::FromHex(Hex))

SPSServerSingleton::SPSServerSingleton()
{
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
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					// onclick
					.IsEnabled_Lambda([this]()
					             {
						             return GetIsEnabled() || State == EServerState::E_Running;
					             })
					.OnClicked_Lambda([this]()
					             {
						             State = State == EServerState::E_Running
							                     ? EServerState::E_Stop
							                     : EServerState::E_Running;
						             return FReply::Handled();
					             })
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

				]
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
						.OnValueChanged_Lambda([this](FString NewPort)
						{
							auto func = [=](bool bAvailable)
							{
								if (bAvailable)
								{
									Config.Config.HttpPort = FCString::Atoi(*NewPort);
									if (FileHelper::Get().UpdateServerConfigIntoJsonFile(
										Config.ConfigFilePath, Config.Config))
									{
										HttpPort = Config.Config.HttpPort;
									}
									else
									{
										Config.Config.HttpPort = HttpPort;
									}
								}
								else
								{
									HttpPortText->SetText(FString::FromInt(HttpPort));
								}
							};

							
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
						.OnValueChanged_Lambda([this](FString NewPort)
						{
							Config.Config.SFUPort = FCString::Atoi(*NewPort);
							if (!FileHelper::Get().UpdateServerConfigIntoJsonFile(Config.ConfigFilePath, Config.Config))
							{
								SFUPortText->SetText(FString::FromInt(SFUPort));
							}
							else
							{
								SFUPort = Config.Config.HttpPort;
							}
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
						.OnValueChanged_Lambda([this](FString NewPort)
						{
							Config.Config.StreamerPort = FCString::Atoi(*NewPort);
							if (!FileHelper::Get().UpdateServerConfigIntoJsonFile(Config.ConfigFilePath, Config.Config))
							{
								StreamerPortText->SetText(FString::FromInt(StreamerPort));
							}
							else
							{
								StreamerPort = Config.Config.StreamerPort;
							}
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

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
