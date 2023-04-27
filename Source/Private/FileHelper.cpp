#include "FileHelper.h"
#include "Json.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogPSFileHelper);

FCopyDirectoryTask::FCopyDirectoryTask(FString _SrcDir, FString _DestDir, bool _bOverwirteAllExisting)
	:SrcDir(_SrcDir),DestDir(_DestDir),bOverwirteAllExisting(_bOverwirteAllExisting)
{
	
}

void FCopyDirectoryTask::DoWork()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.CopyDirectoryTree(*(DestDir), *SrcDir, bOverwirteAllExisting))
	{
		UE_LOG(LogPSFileHelper,Display,TEXT("Copy finished."));
		//PlatformFile.DeleteDirectoryRecursively(*SrcDir);
		return;
	}
	UE_LOG(LogPSFileHelper,Display,TEXT("Copy failed."));
}

SignallingServerConfig FileHelper::LoadServerConfigFromJsonFile(const FString& JsonFile)
{
	SignallingServerConfig Result;

	if (FPaths::FileExists(JsonFile))
	{
		FString FileStr;
		FFileHelper::LoadFileToString(FileStr, *JsonFile);
		TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileStr);
		if (FJsonSerializer::Deserialize(JsonReader, RootObj))
		{
			Result.UseMatchmaker = RootObj->GetBoolField("UseMatchmaker");
			Result.UseHTTPS = RootObj->GetBoolField("UseHTTPS");

			Result.HomepageFile = RootObj->GetStringField("HomepageFile");
			Result.PublicIp = RootObj->GetStringField("PublicIp");
			Result.MatchmakerAddress = RootObj->GetStringField("MatchmakerAddress");

			Result.MatchmakerPort = int(RootObj->GetNumberField("MatchmakerPort"));

			Result.HttpPort = int(RootObj->GetNumberField("HttpPort"));
			Result.HttpsPort = int(RootObj->GetNumberField("HttpsPort"));
			Result.StreamerPort = int(RootObj->GetNumberField("StreamerPort"));
			Result.SFUPort = int(RootObj->GetNumberField("SFUPort"));
			Result.MaxPlayerCount = int(RootObj->GetNumberField("MaxPlayerCount"));
		}
	}

	return Result;
}

bool FileHelper::CopyFolderRecursively(const FString& SrcDir, const FString& DestDir, const FString& DirName, bool bOverwirteAllExisting) const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if(!FPaths::DirectoryExists(DestDir))
	{
		if(!PlatformFile.CreateDirectory(*DestDir))
		{
			return false;
		}
	}

	FAsyncTask<FCopyDirectoryTask>* Task = new FAsyncTask<FCopyDirectoryTask>(SrcDir,DestDir / DirName,bOverwirteAllExisting);
	Task->StartBackgroundTask();
	
	return false;
}

