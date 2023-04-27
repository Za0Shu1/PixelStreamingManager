#pragma once

#include <Windows/DirectX/Include/D3Dcompiler.h>

#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "CoreMinimal.h"

class FConfigFile;
DECLARE_LOG_CATEGORY_EXTERN(LogPSSettings, Log, All);

struct FLaunchConfig
{
public:
	FString MatchMakerBatchPath;
	FString SingnallingServerLocalPath;
	FString SingnallingServerPublicPath;
	FString SingnallingServerConfigPath;
	FString ServersRoot;
};

struct FSettingsConfig
{
	static FSettingsConfig& Get() 
	{
		static FSettingsConfig Instance;
		return Instance;
	}

	FSettingsConfig();

	/**** Getter ****/
	const FString& GetWebServersPath()
	{
		return WebServersPath; 
	}

	const FString& GetClientPath()
	{
		return ClientPath; 
	}

	const FString& GetPublicIP()
	{
		return PublicIP; 
	}

	const FString& GetProjectName()
	{
		return ProjectName; 
	}

	const FString& GetExtraCommands()
	{
		return ExtraCommands; 
	}

	const bool& GetIsPublic() const
	{
		return bIsPublic; 
	}

	const bool& GetUseMatchmaker() const
	{
		return bUseMatchmaker; 
	}

	const bool& IsServerValid() const
	{
		return bServerValid; 
	}

	const bool& IsClientValid();
	

	FLaunchConfig& GetLaunchConfig()
	{
		return LaunchConfig;
	}
	
	/**** Setter ****/
	void SetWebServersPath(FString InPath);
	void SetClientPath(FString InPath);
	void SetPublicIP(FString InIPAddr);
	void SetProjectName(FString NewName);
	void SetExtraCommands(FString NewCommands);
	void SetIsPublic(bool InBool);
	void SetUseMatchmaker(bool InBool);

	
	void ValidServer(FString InCondition);

protected:
	FString SectionName;
	FString ServerSectionName;
	
	FString WebServersPath;
	FString ClientPath;
	FString PublicIP;
	FString ProjectName;
	FString ExtraCommands;
	bool bIsPublic;
	bool bUseMatchmaker;
	
	bool bServerValid;
	bool bClientValid;
	FLaunchConfig LaunchConfig;


private:
	TArray<FString> ValidationConditions;
};

