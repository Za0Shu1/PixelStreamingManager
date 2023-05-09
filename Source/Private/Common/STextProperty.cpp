#include "STextProperty.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"


#define FromHex(Hex) FLinearColor::FromSRGBColor(FColor::FromHex(Hex))

void STextProperty::Construct(const FArguments& InArgs)
{
	Key = InArgs._Key;
	Value = InArgs._Value;
	LeftWidth = InArgs._LeftWidth;
	OnValueChanged = InArgs._OnValueChanged;

	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		  .HAlign(HAlign_Left)
		  .VAlign(VAlign_Center)
		  .AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(LeftWidth)
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Key + " : "))
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.ToolTipText_Lambda([this]()
				{
					return FText::FromString(Key);
				})
			]
		]

		+ SHorizontalBox::Slot()
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Center)
		  .FillWidth(1.f)
		  .Padding(FMargin(10.f, 0.f))
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FromHex("#6161615B"))
			.HAlign(HAlign_Fill)
			.Padding(FMargin(10.f, 0, 0, 0))
			[
				SAssignNew(Textblock,SEditableText)
				.Text(FText::FromString(Value))
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.SelectAllTextWhenFocused(true)

				.ToolTipText_Lambda([this]()
				                   {
					                   return FText::FromString(Value);
				                   })
				.OnTextCommitted_Lambda([=](const FText& NewText, ETextCommit::Type CommitType)
				                   {
					                   if (Value != NewText.ToString())
					                   {
						                   Value = NewText.ToString();
						                   OnValueChanged.ExecuteIfBound(Value);
					                   }
				                   })
			]
		]
	];
}

void STextProperty::SetText(const FString& InText)
{
	if(Textblock.IsValid())
	{
		Textblock->SetText(FText::FromString(InText));
	}
}
