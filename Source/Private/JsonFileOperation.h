#pragma once
#include "CoreMinimal.h"


struct SignallingServerConfig
{
public:
	bool UseMatchmaker;
	bool UseHTTPS;

	FString HomepageFile;
	FString PublicIp;
	FString MatchmakerAddress;

	
	int MatchmakerPort;
	
	int HttpPort;
	int HttpsPort;
	int StreamerPort;
	int SFUPort;
	int MaxPlayerCount;
};

class JsonFileHelper
{
public:
	static JsonFileHelper& Get() 
	{
		static JsonFileHelper Instance;
		return Instance;
	}

	SignallingServerConfig LoadServerConfigFromJsonFile(const FString& JsonFile);
	
};
