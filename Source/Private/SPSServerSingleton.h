#pragma once

#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/UnrealString.h"
#include "FileHelper.h"

class SEditableText;

enum class EServerState : uint8
{
	E_Running,
	E_Stop,
};

DECLARE_DELEGATE_TwoParams(FOnStateChanged, SignallingServerConfig, EServerState);
DECLARE_DELEGATE_TwoParams(FOnCreateServer, SignallingServerConfig, FString);
DECLARE_DELEGATE_ThreeParams(FOnDeleteServer, SignallingServerConfig, FString,class SPSServerSingleton*);

class SPSServerSingleton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPSServerSingleton)
			: _bIsBackupServer(false),
			  _Width(300.f),
			  _Height(300.f),
			  _Name(FString()),
			  _Config(SignallingServerConfig()),
			  _OnStateChanged(nullptr),
			  _OnCreateServer(nullptr),
			  _OnDeleteServer(nullptr)

		{
		}

		SLATE_ARGUMENT(bool, bIsBackupServer)
		SLATE_ARGUMENT(float, Width)
		SLATE_ARGUMENT(float, Height)
		SLATE_ARGUMENT(class FString, Name)
		SLATE_ARGUMENT(SignallingServerConfig, Config)

		SLATE_EVENT(FOnStateChanged, OnStateChanged)
		SLATE_EVENT(FOnCreateServer, OnCreateServer)
		SLATE_EVENT(FOnDeleteServer, OnDeleteServer)

	SLATE_END_ARGS()

	SPSServerSingleton();
	~SPSServerSingleton();

	bool bIsBackupServer;
	float Width;
	float Height;
	SignallingServerConfig Config;
	int32 HttpPort;
	int32 SFUPort;
	int32 StreamerPort;
	EServerState State;
	FString Name;
	FOnStateChanged OnStateChanged;
	FOnCreateServer OnCreateServer;
	FOnDeleteServer OnDeleteServer;

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
