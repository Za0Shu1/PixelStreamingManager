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

TArray<FBackupServerInfo> FileHelper::LoadAllBackupServers(const FString& JsonFile)
{
	TArray<FBackupServerInfo> Result;
	if (FPaths::FileExists(JsonFile))
	{
		FString FileStr;
		FFileHelper::LoadFileToString(FileStr, *JsonFile);
		TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileStr);
		if (FJsonSerializer::Deserialize(JsonReader, RootObj))
		{
			const TArray<TSharedPtr<FJsonValue>>* OutArray;
			if (RootObj->TryGetArrayField(TEXT("Servers"), OutArray))
			{
				for (auto ServerInfo : *OutArray)
				{
					TSharedPtr<FJsonObject> obj = ServerInfo->AsObject();
					FBackupServerInfo Temp;
					Temp.ServerName = obj->GetStringField("ServerName");
					Temp.SingnallingServerLocalPath = obj->GetStringField("SingnallingServerLocalPath");
					Temp.SingnallingServerPublicPath = obj->GetStringField("SingnallingServerPublicPath");

					Result.Add(Temp);
				}
			}
		}
	}
	return Result;
}

void FileHelper::AddServerIntoConfig(FBackupServerInfo Config)
{
	TArray<FBackupServerInfo> Result;
	const FString JsonFile = FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup/Servers.json";
	if (FPaths::FileExists(JsonFile))
	{
		FString FileStr;
		FFileHelper::LoadFileToString(FileStr, *JsonFile);
		TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileStr);
		if (FJsonSerializer::Deserialize(JsonReader, RootObj))
		{
			const TArray<TSharedPtr<FJsonValue>>* OutArray;

			if (RootObj->TryGetArrayField(TEXT("Servers"), OutArray))
			{
				TSharedPtr<FJsonObject> obj =  MakeShareable(new FJsonObject());
				obj->SetStringField("ServerName", Config.ServerName);
				obj->SetStringField("SingnallingServerLocalPath", Config.SingnallingServerLocalPath);
				obj->SetStringField("SingnallingServerPublicPath", Config.SingnallingServerPublicPath);

				TArray<TSharedPtr<FJsonValue>>* Temp = const_cast<TArray<TSharedPtr<FJsonValue>>*>(OutArray);
				Temp->Add(MakeShareable(new FJsonValueObject(obj)));

				RootObj->SetArrayField(TEXT("Servers"),*Temp);
				
				//Write the json file
				FString Json;
				TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Json, 0);
				if (FJsonSerializer::Serialize(RootObj.ToSharedRef(), JsonWriter))
				{
					FFileHelper::SaveStringToFile(Json, *JsonFile);
				}
			}
		}
	}
}

void FileHelper::DeleteServerFromConfig(FString ServerName)
{
	TArray<FBackupServerInfo> Result;
	const FString JsonFile = FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup/Servers.json";
	if (FPaths::FileExists(JsonFile))
	{
		FString FileStr;
		FFileHelper::LoadFileToString(FileStr, *JsonFile);
		TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileStr);
		if (FJsonSerializer::Deserialize(JsonReader, RootObj))
		{
			const TArray<TSharedPtr<FJsonValue>>* OutArray;

			if (RootObj->TryGetArrayField(TEXT("Servers"), OutArray))
			{
				for (auto ServerInfo : *OutArray)
				{
					TSharedPtr<FJsonObject> obj = ServerInfo->AsObject();
					if( obj->GetStringField("ServerName") == ServerName)
					{
						TArray<TSharedPtr<FJsonValue>>* Temp = const_cast<TArray<TSharedPtr<FJsonValue>>*>(OutArray);
						Temp->Remove(ServerInfo);
						RootObj->SetArrayField(TEXT("Servers"),*Temp);
						break;
					}
				}

				//Write the json file
				FString Json;
				TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Json, 0);
				if (FJsonSerializer::Serialize(RootObj.ToSharedRef(), JsonWriter))
				{
					FFileHelper::SaveStringToFile(Json, *JsonFile);
				}
			}
		}
	}
}

void FileHelper::ModifyServerFromConfig(FString From, FString To)
{
	TArray<FBackupServerInfo> Result;
	const FString JsonFile = FSettingsConfig::Get().GetLaunchConfig().ServersRoot / "Backup/Servers.json";
	if (FPaths::FileExists(JsonFile))
	{
		FString FileStr;
		FFileHelper::LoadFileToString(FileStr, *JsonFile);
		TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileStr);
		if (FJsonSerializer::Deserialize(JsonReader, RootObj))
		{
			const TArray<TSharedPtr<FJsonValue>>* OutArray;

			if (RootObj->TryGetArrayField(TEXT("Servers"), OutArray))
			{
				for (auto ServerInfo : *OutArray)
				{
					TSharedPtr<FJsonObject> obj = ServerInfo->AsObject();
					if( obj->GetStringField("ServerName") == From)
					{
						TArray<TSharedPtr<FJsonValue>>* Temp = const_cast<TArray<TSharedPtr<FJsonValue>>*>(OutArray);
						Temp->Remove(ServerInfo);
						ServerInfo->AsObject()->SetStringField("ServerName",To);
						Temp->Add(ServerInfo);
						RootObj->SetArrayField(TEXT("Servers"),*Temp);
						break;
					}
				}

				//Write the json file
				FString Json;
				TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Json, 0);
				if (FJsonSerializer::Serialize(RootObj.ToSharedRef(), JsonWriter))
				{
					FFileHelper::SaveStringToFile(Json, *JsonFile);
				}
			}
		}
	}
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
			AsyncTask(ENamedThreads::GameThread, [&,bSuccess,this]()
			{
				CopyCompletionCallback(bSuccess);
			});
		}
	);
}

void FileHelper::DeleteFolder(const FString& TargetPath, TUniqueFunction<void()>&& FailedCallback)
{
	DeleteFailedCallback = MoveTemp(FailedCallback);

	// Check if the destination directory exists
	if (!FPaths::DirectoryExists(TargetPath))
	{
		return;
	}

	// Asynchronously delete the directory tree
	Async(
		EAsyncExecution::ThreadPool,
		[&,TargetPath,this]()
		{
			// Get the platform file
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			// Delete the directory tree
			if (!PlatformFile.DeleteDirectoryRecursively(*TargetPath))
			{
				// Call the completion callback with result
				AsyncTask(ENamedThreads::GameThread, [&,this]()
				{
					DeleteFailedCallback();
				});
			}
		}
	);
}

void FileHelper::RenameFolder(const FString& From, const FString& To, TUniqueFunction<void(bool)>&& ResultCallback)
{
	RenameResultCallback = MoveTemp(ResultCallback);

	// Check if the destination directory exists
	if (!FPaths::DirectoryExists(From))
	{
		UE_LOG(LogPSFileHelper, Display, TEXT("Directory not Exists..."));
		RenameResultCallback(false);
		return;
	}

	// Asynchronously rename the directory
	Async(
		EAsyncExecution::ThreadPool,
		[&,From,To,this]()
		{
			// Get the platform file
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			// Rename the directory tree
			bool bResult = PlatformFile.MoveFile(*To,*From);
			
			// Call the completion callback with result
			AsyncTask(ENamedThreads::GameThread, [&,bResult,this]()
			{
				RenameResultCallback(bResult);
			});
		}
	);
}
