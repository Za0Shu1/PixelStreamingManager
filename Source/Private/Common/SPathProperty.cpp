#include "SPathProperty.h"
#include "DesktopPlatformModule.h"
#include "SlateOptMacros.h"
#include "Windows/WindowsApplication.h"


#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

DEFINE_LOG_CATEGORY(LogPathProperty);
#define LOCTEXT_NAMESPACE "SPathProperty"
#define FromHex(Hex) FLinearColor::FromSRGBColor(FColor::FromHex(Hex))

void SPathProperty::Construct(const FArguments& InArgs)
{
	Key = InArgs._Key;
	Value = InArgs._Value;
	AllowFileTypes = InArgs._AllowFileTypes;
	bIsEnable = InArgs._bIsEnable;
	bPickupFile = InArgs._bPickupFile;
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
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Center)
		  .FillWidth(1.f)
		  .Padding(FMargin(2.f, 0.f))
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.OnClicked_Raw(this, &SPathProperty::PickupFolder)
			[
				SAssignNew(PathText, STextBlock)
				.Text(FText::FromString(Value.IsEmpty() ? "..." : Value))
				.ToolTipText(FText::FromString(Value.IsEmpty()
					                               ? FString::Printf(TEXT("Click to select your %s path"), *Key)
					                               : Value))
			]
		]
	];
}

FReply SPathProperty::PickupFolder()
{
	FString NewPath;
	const FString Title = FString::Printf(TEXT("选择%s"), *Key);
	
	if (bPickupFile)
	{
		// Prompt the user for the directory
		TArray<FString> OutFiles;

		if (FDesktopPlatformModule::Get()->OpenFileDialog(GetActiveWindow(),
															   Title,
															   Value, NewPath,AllowFileTypes, EFileDialogFlags::None, OutFiles))
		{
			if(OutFiles.Num() == 0)
			{
				return FReply::Handled();
			}
			
			NewPath = OutFiles[0];
			if (Value != NewPath)
			{
				Value = NewPath;
				UE_LOG(LogPathProperty, Display, TEXT("%s selected at : %s"), *Key, *Value);
				if (PathText.IsValid())
				{
					PathText->SetText(FText::FromString(Value));
					PathText->SetToolTipText(FText::FromString(Value));
					OnValueChanged.ExecuteIfBound(Value);
				}
			}
		}
	}
	else
	{
		// Prompt the user for the directory
		if (FDesktopPlatformModule::Get()->OpenDirectoryDialog(GetActiveWindow(),
		                                                       Title,
		                                                       Value, NewPath))
		{
			if (Value != NewPath)
			{
				Value = NewPath;
				UE_LOG(LogPathProperty, Display, TEXT("%s selected at : %s"), *Key, *Value);
				if (PathText.IsValid())
				{
					PathText->SetText(FText::FromString(Value));
					PathText->SetToolTipText(FText::FromString(Value));
					OnValueChanged.ExecuteIfBound(Value);
				}
			}
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
