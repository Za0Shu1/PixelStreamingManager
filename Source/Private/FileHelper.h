#pragma once
#include "CoreMinimal.h"
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

class FileHelper
{
public:
	static FileHelper& Get()
	{
		static FileHelper Instance;
		return Instance;
	}

	SignallingServerConfig LoadServerConfigFromJsonFile(const FString& JsonFile);

	void CopyFolderRecursively(const FString& SrcDir, const FString& ParentDir, const FString& DirName, bool bOverwirteAllExisting,TUniqueFunction<void(bool)>&& CompletionCallback = nullptr);

private:
	TUniqueFunction<void(bool)> CopyCompletionCallback;
};
