#pragma once
#include "CoreMinimal.h"
#include "FSettingsConfig.h"
#include "Async/AsyncWork.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPSFileHelper, Log, All);

struct SignallingServerConfig
{
public:
	bool UseMatchmaker = false;
	bool UseHTTPS = false;

	FString HomepageFile = "";
	FString PublicIp = "";
	FString MatchmakerAddress = "";


	int MatchmakerPort = -1;

	int HttpPort = -1;
	int HttpsPort = -1;
	int StreamerPort = -1;
	int SFUPort = -1;
	int MaxPlayerCount = -1;
};

struct FBackupServerInfo
{
	FString ServerName;
	FString SingnallingServerLocalPath;
	FString SingnallingServerPublicPath;
	FString ConfigFilePath;

	SignallingServerConfig Config;
};

class FileHelper
{
public:
	static FileHelper& Get()
	{
		static FileHelper Instance;
		return Instance;
	}

	// server launch config
	SignallingServerConfig LoadServerConfigFromJsonFile(const FString& JsonFile);
	bool UpdateServerConfigIntoJsonFile(const FString& JsonFile, const SignallingServerConfig& NewConfig) const;

	// servers config
	TArray<FBackupServerInfo> LoadAllBackupServers(const FString& JsonFile);
	void AddServerIntoConfig(FBackupServerInfo Config);
	void DeleteServerFromConfig(FString ServerName);
	void ModifyServerFromConfig(FString From,FString To);

	void CopyFolderRecursively(const FString& SrcDir, const FString& ParentDir, const FString& DirName, bool bOverwirteAllExisting,TUniqueFunction<void(bool)>&& CompletionCallback = nullptr);
	void DeleteFolder(const FString& TargetPath,TUniqueFunction<void()>&& FailedCallback);
	void RenameFolder(const FString& From, const FString& To,TUniqueFunction<void(bool)>&& ResultCallback);

private:
	TUniqueFunction<void(bool)> CopyCompletionCallback;
	TUniqueFunction<void()> DeleteFailedCallback;
	TUniqueFunction<void(bool)> RenameResultCallback;
};
