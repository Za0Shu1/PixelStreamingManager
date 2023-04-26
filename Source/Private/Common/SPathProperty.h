#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/UnrealString.h"

class STextBlock;
class SEditableText;

DECLARE_LOG_CATEGORY_EXTERN(LogPathProperty, Log, All);

DECLARE_DELEGATE_OneParam(FOnPathValueChanged, FString);

class SPathProperty : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPathProperty)
			: _Key("Key"),
			  _Value("Value"),
			  _AllowFileTypes(TEXT("JSON files (*.json)|*.json")),
			  _bPickupFile(false),
			  _LeftWidth(100.f),
			  _OnValueChanged()
		{
		}

		SLATE_ARGUMENT(FString, Key)
		SLATE_ARGUMENT(FString, Value)
		SLATE_ARGUMENT(FString, AllowFileTypes)
		SLATE_ARGUMENT(bool, bPickupFile);
		SLATE_ARGUMENT(float, LeftWidth);
		SLATE_EVENT(FOnPathValueChanged, OnValueChanged);

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	FReply PickupFolder();

private:
	bool bPickupFile;
	FString Key;
	FString Value;
	FString AllowFileTypes;
	float LeftWidth;
	FOnPathValueChanged OnValueChanged;
	TSharedPtr<STextBlock> PathText;
};
