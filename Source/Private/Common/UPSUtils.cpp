#include "UPSUtils.h"

#include <string>

#include "Dom/JsonObject.h"
#include "PixelStreamingManager/Source/Private/FSettingsConfig.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Interfaces/IPv4/IPv4Address.h"

DEFINE_LOG_CATEGORY(LogPSUtils);

void UPSUtils::CopyToClipBoard(FString InContent)
{
	// 将字符串转换为宽字符
	const wchar_t* TextToCopy = *InContent;

	// 打开剪贴板
	if (OpenClipboard(nullptr))
	{
		// 清空剪贴板内容
		EmptyClipboard();

		// 计算字符串所需的内存大小（包括结尾的null字符）
		int32 Size = (InContent.Len() + 1) * sizeof(wchar_t);

		// 在剪贴板上分配内存
		HGLOBAL MemoryHandle = GlobalAlloc(GMEM_MOVEABLE, Size);

		// 将字符串复制到剪贴板内存中
		if (MemoryHandle)
		{
			void* MemoryPointer = GlobalLock(MemoryHandle);
			if (MemoryPointer)
			{
				FMemory::Memcpy(MemoryPointer, TextToCopy, Size);
				GlobalUnlock(MemoryHandle);

				// 将内存块设置为剪贴板数据
				SetClipboardData(CF_UNICODETEXT, MemoryHandle);
			}
			else
			{
				GlobalFree(MemoryHandle);
			}
		}

		// 关闭剪贴板
		CloseClipboard();
	}
}

bool UPSUtils::GetJsonValue(const FString& JsonString, const FString& FieldName, FString& OutValue)
{
	// 解析 JSON 字符串
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		// JSON 解析失败
		return false;
	}

	// 提取字段值
	if (!JsonObject->TryGetStringField(FieldName, OutValue))
	{
		// 字段不存在或类型不匹配
		return false;
	}

	// 成功提取字段值
	return true;
}

void UPSUtils::TerminateProcessByHandle(HANDLE& Handle)
{
	if (Handle != NULL && Handle != INVALID_HANDLE_VALUE)
	{
		DWORD processId = GetProcessId(Handle);
		UE_LOG(LogPSUtils,Warning,TEXT("Gonna close process that PID is %u."),processId);
		
		std::wstring command = L"taskkill /PID " + std::to_wstring(processId) + L" /T /F";
		STARTUPINFOW startupInfo = { sizeof(STARTUPINFOW) };
		PROCESS_INFORMATION processInfo;

		CreateProcessW(NULL, const_cast<LPWSTR>(command.c_str()), NULL, NULL, false, 0, NULL, NULL, &startupInfo, &processInfo);

		WaitForSingleObject(processInfo.hProcess, INFINITE);

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);

		Handle = INVALID_HANDLE_VALUE;
	}
}

bool UPSUtils::IsIPAddress(const FString& IPAddressString)
{
	FIPv4Address IPv4Address;

	return FIPv4Address::Parse(IPAddressString, IPv4Address);
}