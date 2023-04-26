#include "FSettingsConfig.h"

#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY(LogPSSettings);

FSettingsConfig::FSettingsConfig()
	: SectionName("PixelStreamingManager"),
	  ServerSectionName("DefaultServer"),
	  bServerValid(false),
	  ValidationConditions({
		  "Matchmaker", "SFU", "SignallingWebServer", "run.bat", "run_local.bat", "Start_WithTURN_SignallingServer.ps1",
		  "turnserver.exe", "node.exe"
	  })
{
	if (!GConfig->GetString(*SectionName,TEXT("WebServersPaths"), WebServersPath, GEngineIni))
	{
		WebServersPath = "";
	}

	if (!GConfig->GetString(*SectionName,TEXT("ClientPath"), ClientPath, GEngineIni))
	{
		ClientPath = "";
	}

	if (!GConfig->GetString(*SectionName,TEXT("PublicIP"), PublicIP, GEngineIni))
	{
		PublicIP = "";
	}

	if (!GConfig->GetString(*SectionName,TEXT("ProjectName"), ProjectName, GEngineIni))
	{
		ProjectName = "未命名";
	}

	if (!GConfig->GetString(*SectionName,TEXT("ExtraCommands"), ExtraCommands, GEngineIni))
	{
		ExtraCommands = "";
	}

	if (!GConfig->GetBool(*SectionName,TEXT("bIsPublic"), bIsPublic, GEngineIni))
	{
		bIsPublic = false;
	}

	if (!GConfig->GetBool(*SectionName,TEXT("bUseMatchmaker"), bUseMatchmaker, GEngineIni))
	{
		bUseMatchmaker = false;
	}

#pragma region Default Server Config
	if (!GConfig->GetString(*ServerSectionName,TEXT("MatchMakerBatchPath"), LaunchConfig.MatchMakerBatchPath, GEngineIni))
	{
		LaunchConfig.MatchMakerBatchPath = "";
	}

	if (!GConfig->GetString(*ServerSectionName,TEXT("SingnallingServerConfigPath"), LaunchConfig.SingnallingServerConfigPath, GEngineIni))
	{
		LaunchConfig.SingnallingServerConfigPath = "";
	}

	if (!GConfig->GetString(*ServerSectionName,TEXT("SingnallingServerLocalPath"), LaunchConfig.SingnallingServerLocalPath, GEngineIni))
	{
		LaunchConfig.SingnallingServerLocalPath = "";
	}

	if (!GConfig->GetString(*ServerSectionName,TEXT("SingnallingServerConfigPath"), LaunchConfig.SingnallingServerConfigPath, GEngineIni))
	{
		LaunchConfig.SingnallingServerConfigPath = "";
	}
#pragma endregion

}

const bool& FSettingsConfig::IsClientValid()
{
	::DWORD VersionInfoSize = GetFileVersionInfoSize(*ClientPath, NULL);
	bClientValid = false;
	if (VersionInfoSize != 0)
	{
		TArray<uint8> VersionInfo;
		VersionInfo.AddUninitialized(VersionInfoSize);
		if (GetFileVersionInfo(*ClientPath, NULL, VersionInfoSize, VersionInfo.GetData()))
		{
			// 多重验证
			bClientValid = true;
			TCHAR* ProductVersion;
			::UINT ProductVersionLen;
			if (VerQueryValue(VersionInfo.GetData(), TEXT("\\StringFileInfo\\040904b0\\ProductVersion"),
			                  (LPVOID*)&ProductVersion, &ProductVersionLen))
			{
				UE_LOG(LogPSSettings, Display, TEXT("Product version : %s "), ProductVersion);
			}

			TCHAR* FileDescription;
			::UINT FileDescriptionLen;
			if (VerQueryValue(VersionInfo.GetData(), TEXT("\\StringFileInfo\\040904b0\\FileDescription"),
			                  (LPVOID*)&FileDescription, &FileDescriptionLen))
			{
				UE_LOG(LogPSSettings, Display, TEXT("File description : %s "), FileDescription);
				bClientValid &= FString(FileDescription).Equals("BootstrapPackagedGame");
			}
			else
			{
				bClientValid = false;
			}

			return bClientValid;
		}
	}

	UE_LOG(LogPSSettings, Error, TEXT("Can not valid exe file."));
	return bClientValid;
}

void FSettingsConfig::SetWebServersPath(FString InPath)
{
	WebServersPath = InPath;
	GConfig->SetString(*SectionName, TEXT("WebServersPaths"), *InPath, GEngineIni);
}

void FSettingsConfig::SetClientPath(FString InPath)
{
	ClientPath = InPath;
	GConfig->SetString(*SectionName, TEXT("ClientPath"), *InPath, GEngineIni);
}

void FSettingsConfig::SetPublicIP(FString InIPAddr)
{
	PublicIP = InIPAddr;
	GConfig->SetString(*SectionName, TEXT("PublicIP"), *InIPAddr, GEngineIni);
}

void FSettingsConfig::SetProjectName(FString NewName)
{
	ProjectName = NewName;
	GConfig->SetString(*SectionName, TEXT("ProjectName"), *ProjectName, GEngineIni);
}

void FSettingsConfig::SetExtraCommands(FString NewCommands)
{
	UE_LOG(LogTemp, Display, TEXT("Extra commands changed to %s ."), *NewCommands);

	ExtraCommands = NewCommands;
	GConfig->SetString(*SectionName, TEXT("ExtraCommands"), *ExtraCommands, GEngineIni);
}

void FSettingsConfig::SetIsPublic(bool InBool)
{
	bIsPublic = InBool;
	GConfig->SetBool(*SectionName,TEXT("bIsPublic"), bIsPublic, GEngineIni);
}

void FSettingsConfig::SetUseMatchmaker(bool InBool)
{
	bUseMatchmaker = InBool;
	GConfig->SetBool(*SectionName,TEXT("bUseMatchmaker"), bIsPublic, GEngineIni);
}

void FSettingsConfig::ValidServer(FString InCondition)
{
	if (ValidationConditions.Contains(InCondition))
	{
		ValidationConditions.Remove(InCondition);
	}

	bServerValid = ValidationConditions.IsEmpty();
	
	if(bServerValid)
	{
		GConfig->SetString(*ServerSectionName, TEXT("MatchMakerBatchPath"), *LaunchConfig.MatchMakerBatchPath, GEngineIni);
		GConfig->SetString(*ServerSectionName, TEXT("SingnallingServerConfigPath"), *LaunchConfig.SingnallingServerConfigPath, GEngineIni);
		GConfig->SetString(*ServerSectionName, TEXT("SingnallingServerLocalPath"), *LaunchConfig.SingnallingServerLocalPath, GEngineIni);
		GConfig->SetString(*ServerSectionName, TEXT("SingnallingServerPublicPath"), *LaunchConfig.SingnallingServerPublicPath, GEngineIni);
	}
}
