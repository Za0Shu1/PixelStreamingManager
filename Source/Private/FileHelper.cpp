#include "FileHelper.h"
#include "Json.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogPSFileHelper);

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

// Function to copy a folder recursively
// callback取右值引用
void FileHelper::CopyFolderRecursively(const FString& SrcDir, const FString& ParentDir, const FString& DirName,
                                       bool bOverwirteAllExisting,
                                       TUniqueFunction<void(bool)>&& CompletionCallback)
{
	CopyCompletionCallback = MoveTemp(CompletionCallback);
	const FString DestDir = ParentDir / DirName;

	// Get the platform file
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	// Check if the destination directory exists
	if (!FPaths::DirectoryExists(DestDir))
	{
		// If not, create it
		if (!PlatformFile.CreateDirectoryTree(*DestDir))
		{
			// If creation fails, call the completion callback with false
			CopyCompletionCallback(false);
		}
	}
	bool bSuccess = false;

	// Asynchronously copy the directory tree
	Async(
		EAsyncExecution::ThreadPool,
		[&,DestDir,this]()
		{
			// Get the platform file
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			// Copy the directory tree
			if (PlatformFile.CopyDirectoryTree(*DestDir, *SrcDir, bOverwirteAllExisting))
			{
				// Log success
				UE_LOG(LogPSFileHelper, Display, TEXT("Copy finished."));
				bSuccess = true;
			}
			else
			{
				// Log failure
				UE_LOG(LogPSFileHelper, Display, TEXT("Copy failed."));
				bSuccess = false;
			}
		},
		[&,this]()
		{
			// Call the completion callback with result
			AsyncTask(ENamedThreads::GameThread, [&,this]()
			{
				CopyCompletionCallback(bSuccess);
			});
		}
	);
}
