#include "SPSServerSingleton.h"

#include "CommonStyle.h"
#include "SlateOptMacros.h"
#include "Common/STextProperty.h"
#include "Styling/AppStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"


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
	bIsEnable = InArgs._bIsEnable;
	Name = InArgs._Name;
	Width = InArgs._Width;
	Height = InArgs._Height;
	OnServerClick = InArgs._OnServerClick;
	State = EServerState::E_Stop;

	HttpPort = InArgs._HttpPort;
	SFUPort = InArgs._SFUPort;
	StreamerPort = InArgs._StreamerPort;

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
						             bIsEnable = bIsEnable ? false : true;
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
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
				]

				// Ports
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
						SNew(STextProperty)
						.Key("Http Port")
						.Value(FString::FromInt(HttpPort))
						.IsEnabled_Raw(this, &SPSServerSingleton::GetIsEnabled)
						.LeftWidth(100.f)
					]

					// sfu port
					+ SVerticalBox::Slot()
					  .AutoHeight()
					  .Padding(FMargin(2.f))
					[
						SNew(STextProperty)
						.Key("SFU Port")
						.Value(FString::FromInt(SFUPort))
						.IsEnabled_Raw(this, &SPSServerSingleton::GetIsEnabled)
						.LeftWidth(100.f)
					]

					// streamer port
					+ SVerticalBox::Slot()
					  .AutoHeight()
					  .Padding(FMargin(2.f))
					[
						SNew(STextProperty)
						.Key("Streamer Port")
						.Value(FString::FromInt(StreamerPort))
						.IsEnabled_Raw(this, &SPSServerSingleton::GetIsEnabled)
						.LeftWidth(100.f)
						.OnValueChanged_Lambda([](FString NewValue)
						{
							UE_LOG(LogTemp, Warning, TEXT("Change Value to : %s"), *NewValue);
						})
					]

				]
			]

		]
	];
}

bool SPSServerSingleton::GetIsEnabled() const
{
	return bIsEnable;
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
