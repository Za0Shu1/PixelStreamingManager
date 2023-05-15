#pragma once
#include "CoreMinimal.h"
#include "Windows.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPSUtils, Log, All);

class UPSUtils
{
public:
	static UPSUtils& Get()
	{
		static UPSUtils Instance;
		return Instance;
	}

	void CopyToClipBoard(FString InContent);
	
	bool GetJsonValue(const FString& JsonString, const FString& FieldName, FString& OutValue);

	void TerminateProcessByHandle(HANDLE& Handle);
	bool IsIPAddress(const FString& IPAddressString);

private:
};