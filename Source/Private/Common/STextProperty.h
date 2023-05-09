#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/UnrealString.h"

class SEditableText;

DECLARE_DELEGATE_OneParam(FOnTextValueChanged, FString);

class STextProperty : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STextProperty)
			: _Key("Key"),
			  _Value("Value"),
			  _LeftWidth(100.f),
			  _OnValueChanged()
		{
		}

		SLATE_ARGUMENT(FString, Key)
		SLATE_ARGUMENT(FString, Value)
		SLATE_ARGUMENT(float, LeftWidth);
		SLATE_EVENT(FOnTextValueChanged, OnValueChanged);

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetText(const FString& InText);

private:
	FString Key;
	FString Value;
	float LeftWidth;
	FOnTextValueChanged OnValueChanged;
	TSharedPtr<SEditableText> Textblock;
};
