#pragma once

#include <Windows/DirectX/Include/D3Dcompiler.h>

#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "CoreMinimal.h"

class FConfigFile;

struct FLaunchConfig
{
public:
	FString MatchMakerBatchPath;
	FString SingnallingServerLocalPath;
	FString SingnallingServerPublicPath;
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

	FLaunchConfig& GetLaunchConfig()
	{
		return LaunchConfig;
	}
	
	/**** Setter ****/
	void SetWebServersPath(FString InPath);
	void SetClientPath(FString InPath);
	void SetPublicIP(FString InIPAddr);
	void SetProjectName(FString NewName);
	void SetIsPublic(bool InBool);
	void SetUseMatchmaker(bool InBool);

	
	void ValidServer(FString InCondition);

protected:
	FString SectionName;
	
	FString WebServersPath;
	FString ClientPath;
	FString PublicIP;
	FString ProjectName;
	bool bIsPublic;
	bool bUseMatchmaker;
	
	bool bServerValid;
	FLaunchConfig LaunchConfig;


private:
	TArray<FString> ValidationConditions;
};

