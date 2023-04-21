#include "FSettingsConfig.h"

#include "Misc/ConfigCacheIni.h"

FSettingsConfig::FSettingsConfig()
    :SectionName("PixelStreamingManager"),
	bServerValid(false),
	ValidationConditions({"Matchmaker","SFU","SignallingWebServer","run.bat","run_local.bat","Start_WithTURN_SignallingServer.ps1","turnserver.exe","node.exe"})
{
	if(!GConfig->GetString(*SectionName,TEXT("WebServersPaths"),WebServersPath,GEngineIni))
	{
		WebServersPath = "";
	}
}

void FSettingsConfig::SetWebServersPath(FString InPath)
{
	WebServersPath = InPath;
	GConfig->SetString( *SectionName, TEXT( "WebServersPaths" ), *InPath, GEngineIni );
}

void FSettingsConfig::SetProjectName(FString NewName)
{
	ProjectName = NewName;
	GConfig->SetString( *SectionName, TEXT( "ProjectName" ), *ProjectName, GEngineIni );
}

void FSettingsConfig::ValidServer(FString InCondition)
{
	if(ValidationConditions.Contains(InCondition))
	{
		ValidationConditions.Remove(InCondition);
	}
	
	bServerValid = ValidationConditions.IsEmpty();
}
