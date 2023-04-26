#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/UnrealString.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBooleanProperty, Log, All);

DECLARE_DELEGATE_OneParam(FOnBoolValueChanged, bool);

class SBooleanProperty : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBooleanProperty)
			: _Key("Key"),
			  _Value(false),
			  _LeftWidth(100.f),
			 _OnValueChanged()
		{
		}

		SLATE_ARGUMENT(FString, Key)
		SLATE_ARGUMENT(bool, Value)
		SLATE_ARGUMENT(float, LeftWidth);
		SLATE_EVENT(FOnBoolValueChanged, OnValueChanged);

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	FReply PickupFolder();

private:
	bool bPickupFile;
	FString Key;
	bool Value;
	float LeftWidth;
	FOnBoolValueChanged OnValueChanged;
};
