#pragma once

#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "CoreMinimal.h"

class FConfigFile;

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

	/**** Setter ****/
	void SetWebServersPath(FString InPath);

protected:
	FString SectionName;
	
	FString WebServersPath;
};

