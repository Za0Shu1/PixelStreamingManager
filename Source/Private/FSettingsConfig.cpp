#include "FSettingsConfig.h"

#include "Misc/ConfigCacheIni.h"

FSettingsConfig::FSettingsConfig()
	: SectionName("PixelStreamingManager"),
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
		ProjectName = "Unknown";
	}

	if (!GConfig->GetBool(*SectionName,TEXT("bIsPublic"), bIsPublic, GEngineIni))
	{
		bIsPublic = false;
	}

	if (!GConfig->GetBool(*SectionName,TEXT("bUseMatchmaker"), bUseMatchmaker, GEngineIni))
	{
		bUseMatchmaker = false;
	}
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
}
