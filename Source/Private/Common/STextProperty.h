#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/UnrealString.h"

class STextBlock;
class SEditableText;

DECLARE_DELEGATE_OneParam(FOnValueChanged, FString);

class STextProperty : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STextProperty)
			: _Key("Key"),
			  _Value("Value"),
			  _bIsEnable(false),
			  _LeftWidth(100.f),
			  _OnValueChanged()
		{
		}

		SLATE_ARGUMENT(FString, Key)
		SLATE_ARGUMENT(FString, Value)
		SLATE_ARGUMENT(bool, bIsEnable);
		SLATE_ARGUMENT(float, LeftWidth);
		SLATE_EVENT(FOnValueChanged, OnValueChanged);

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	bool bIsEnable;
	FString Key;
	FString Value;
	float LeftWidth;
	FOnValueChanged OnValueChanged;
};
