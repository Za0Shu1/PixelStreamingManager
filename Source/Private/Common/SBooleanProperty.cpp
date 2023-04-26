#include "SBooleanProperty.h"
#include "DesktopPlatformModule.h"
#include "SlateOptMacros.h"
#include "Windows/WindowsApplication.h"


#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

DEFINE_LOG_CATEGORY(LogBooleanProperty);
#define LOCTEXT_NAMESPACE "SBooleanProperty"

void SBooleanProperty::Construct(const FArguments& InArgs)
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
				.Text(FText::Format(LOCTEXT("KEY", "{0} : "), FText::FromString(Key)))
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.ToolTipText_Lambda([this]()
				{
					return FText::FromString(Key);
				})
			]
		]

		+ SHorizontalBox::Slot()
		  .HAlign(HAlign_Left)
		  .VAlign(VAlign_Center)
		  .FillWidth(1.f)
		  .Padding(FMargin(10.f, 0.f))
		[
			SNew(SCheckBox)
			.IsChecked(Value)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
			{
				bool bChecked = NewState == ECheckBoxState::Checked;
				if (bChecked != Value)
				{
					Value = bChecked;
					OnValueChanged.ExecuteIfBound(Value);
				}
			})
		]
	];
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
