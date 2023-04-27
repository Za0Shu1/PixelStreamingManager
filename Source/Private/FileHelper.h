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

class FCopyDirectoryTask : public FNonAbandonableTask
{
public:
	friend class FAsyncTask<FCopyDirectoryTask>;
	FCopyDirectoryTask(FString _SrcDir,FString _DestDir,bool _bOverwirteAllExisting);

	void DoWork();
	
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(DoScanTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	FString SrcDir;
	FString DestDir;
	bool bOverwirteAllExisting;
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

	bool CopyFolderRecursively(const FString& SrcDir, const FString& DestDir, const FString& DirName, bool bOverwirteAllExisting) const;

private:
	// 用于生成复制命令的脚本，使用R("")进行源码转义
	FString CopyFileCommandTemplate = R"(
			@echo off
			set PasteTo=PasteToTemplate
			set CopyFrom=CopyFromTemplate
			set NewFolderName=NewFolderNameTemplate
			md %CopyPath%\%NewFolderName%

			xcopy /y/i/s/e %CopyFrom% %PasteTo%\%NewFolderName%
			)";
};
