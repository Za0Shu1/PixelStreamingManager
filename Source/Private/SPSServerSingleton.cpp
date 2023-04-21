#include "SPSServerSingleton.h"

#include "SlateOptMacros.h"
#include "Styling/AppStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

#define LOCTEXT_NAMESPACE "LiveCoding"
#define FromHex(Hex) FLinearColor::FromSRGBColor(FColor::FromHex(Hex))

SPSServerSingleton::SPSServerSingleton()
{
}

SPSServerSingleton::~SPSServerSingleton()
{
}

void SPSServerSingleton::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(300)
		.HeightOverride(300)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			[
				// left background
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
		]
	];
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
