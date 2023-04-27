#pragma once

#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/UnrealString.h"

class SEditableText;

enum class EServerState : uint8
{
	E_Running,
	E_Stop,
};

DECLARE_DELEGATE_OneParam(FOnServerClick, EServerState);

class SPSServerSingleton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPSServerSingleton)
			: _bIsEnable(false),
			  _Width(300.f),
			  _Height(300.f),
			  _Name(FString()),
			  _HttpPort(-1),
			  _SFUPort(-1),
			  _StreamerPort(-1),
			  _OnServerClick(nullptr)
		{
		}

		SLATE_ARGUMENT(bool, bIsEnable)
		SLATE_ARGUMENT(float, Width)
		SLATE_ARGUMENT(float, Height)
		SLATE_ARGUMENT(class FString, Name)
		SLATE_ARGUMENT(int32, HttpPort)
		SLATE_ARGUMENT(int32, SFUPort)
		SLATE_ARGUMENT(int32, StreamerPort)

		SLATE_EVENT(FOnServerClick, OnServerClick)

	SLATE_END_ARGS()

	SPSServerSingleton();
	~SPSServerSingleton();

	bool bIsEnable;
	float Width;
	float Height;
	int32 HttpPort;
	int32 SFUPort;
	int32 StreamerPort;
	EServerState State;
	FString Name;
	FOnServerClick OnServerClick;

	void Construct(const FArguments& InArgs);

public:
	TSharedPtr<SEditableText> ServerName;
	TSharedPtr<SEditableText> HttpPortText;
	TSharedPtr<SEditableText> SFUPortText;
	TSharedPtr<SEditableText> StreamerPortText;
protected:
	bool GetIsEnabled() const;
private:
};
