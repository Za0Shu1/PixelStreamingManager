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

	const FString& GetProjectName()
	{
		return ProjectName; 
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
	void SetProjectName(FString NewName);
	void ValidServer(FString InCondition);

protected:
	FString SectionName;
	FString ProjectName;
	FString WebServersPath;
	bool bServerValid;
	FLaunchConfig LaunchConfig;


private:
	TArray<FString> ValidationConditions;
};

