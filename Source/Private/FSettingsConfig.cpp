#include "FSettingsConfig.h"

#include "Misc/ConfigCacheIni.h"

FSettingsConfig::FSettingsConfig()
    :SectionName("PixelStreamingManager")
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
