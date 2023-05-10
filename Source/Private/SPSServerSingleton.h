#pragma once

#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/UnrealString.h"
#include "FileHelper.h"

class SEditableText;
class SPSServerSingleton;
class STextProperty;

enum class EServerState : uint8
{
	E_Running,
	E_Stop,
};

enum class EPortType : uint8
{
	E_Http,
	E_SFU,
	E_Streamer
};

DECLARE_DELEGATE_TwoParams(FOnStateChanged, FBackupServerInfo, EServerState);
DECLARE_DELEGATE_TwoParams(FOnCreateServer, FBackupServerInfo, FString);
DECLARE_DELEGATE_ThreeParams(FOnDeleteServer, FBackupServerInfo, FString, SPSServerSingleton*);
DECLARE_DELEGATE_ThreeParams(FOnServerRename, SPSServerSingleton*, FBackupServerInfo, FString);
DECLARE_DELEGATE_FourParams(FOnChangePort, FBackupServerInfo&, EPortType, FString&, uint16&);

class SPSServerSingleton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPSServerSingleton)
			: _bIsBackupServer(false),
			  _Width(300.f),
			  _Height(300.f),
			  _Name(FString()),
			  _Config(FBackupServerInfo()),
			  _OnStateChanged(nullptr),
			  _OnCreateServer(nullptr),
			  _OnDeleteServer(nullptr),
			  _OnRenameServer(nullptr),
			  _OnChangePort(nullptr)

		{
		}

		SLATE_ARGUMENT(bool, bIsBackupServer)
		SLATE_ARGUMENT(float, Width)
		SLATE_ARGUMENT(float, Height)
		SLATE_ARGUMENT(class FString, Name)
		SLATE_ARGUMENT(FBackupServerInfo, Config)

		SLATE_EVENT(FOnStateChanged, OnStateChanged)
		SLATE_EVENT(FOnCreateServer, OnCreateServer)
		SLATE_EVENT(FOnDeleteServer, OnDeleteServer)
		SLATE_EVENT(FOnServerRename, OnRenameServer)
		SLATE_EVENT(FOnChangePort, OnChangePort)

	SLATE_END_ARGS()

	SPSServerSingleton();
	~SPSServerSingleton();

	bool bIsBackupServer;
	float Width;
	float Height;
	FBackupServerInfo Config;
	uint16 HttpPort;
	uint16 SFUPort;
	uint16 StreamerPort;
	EServerState State;
	FString Name;
	FOnStateChanged OnStateChanged;
	FOnCreateServer OnCreateServer;
	FOnDeleteServer OnDeleteServer;
	FOnServerRename OnServerRename;
	FOnChangePort OnChangePort;

	void Construct(const FArguments& InArgs);

public:
	TSharedPtr<SEditableText> ServerName;
	TSharedPtr<STextProperty> HttpPortText;
	TSharedPtr<STextProperty> SFUPortText;
	TSharedPtr<STextProperty> StreamerPortText;

protected:
	bool GetIsEnabled() const;
private:
};
