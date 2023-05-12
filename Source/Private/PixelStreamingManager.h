// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "DesktopPlatformModule.h"
#include "SPSServerSingleton.h"
#include "Async/AsyncWork.h"
#include "Containers/Ticker.h"
#include "DesktopPlatform/Private/Windows/WindowsNativeFeedbackContext.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Reply.h"

class SBorder;
class SBox;
class SWrapBox;
class STextBlock;
class SCircularThrobber;
struct SignallingServerConfig;

DECLARE_LOG_CATEGORY_EXTERN(LogPixelStreamingManager, Log, All);

#define FromHex(Hex) FLinearColor::FromSRGBColor(FColor::FromHex(Hex))

class FDoScanTask : public FNonAbandonableTask
{
public:
	friend class FAsyncTask<FDoScanTask>;
	FDoScanTask(FString WebServersPath);

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(DoScanTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	FString ScanWebServersFolder;
};

class FPixelStreamingManager
{
public:
	FPixelStreamingManager(FSlateApplication& InSlate);
	~FPixelStreamingManager();

	void ShutDown();

private:
	FSlateApplication& Slate;
	TSharedPtr<STextBlock> PathText;
	TSharedPtr<SOverlay> LoadingWidget;
	TSharedPtr<STextBlock> LoadingText;
	TSharedPtr<SWrapBox> ServersContainer;
	TSharedPtr<SBox> ContainerArea;
	TSharedPtr<SOverlay> RightPanel;

	/** To know if the ticker was started.*/
	FTSTicker::FDelegateHandle GlobalTickHandle;
	FTSTicker::FDelegateHandle ScanTickHandle;

	FAsyncTask<FDoScanTask>* ScanTask;

	float RightPanelWidth; //右侧区域宽度
	float WrapBoxPadding = 10.f;
	float ItemHorizontalPadding = 5.f; //服务器UI横向间隔
	float ItemVerticalPadding = 2.f; //服务器UI纵向向间隔
	float ItemWidth = 300.f;
	float ItemHeight = 200.f;

	const float ScrollBarThickness = 9.f;
	const float ScrollBarPadding = 2.f;

	TMap<FString, FBackupServerInfo> ExistsServer;
	bool bMatchMakerRunningInProgress = false;
	HANDLE HND_Matchmaker;
	TUniqueFunction<void(FString)> GetAddressCallback;
	
public:
	/****** MANNULLY TICK BEGIN ******/
	bool ScanTaskTick(float UnusedDeltaTime);

	bool Tick(float UnusedDeltaTime);

	void StartScanTicker();

	/****** MANNULLY TICK END ******/

	/****** DRAW WINDOW BEGIN ******/

	void Run();

	FText GenerateScanToolTip() const;
	FText GenerateLaunchMatchmakerToolTip() const;

	void OnWindowResized();

	/****** DRAW WINDOW END ******/


	/****** SCAN TASK BEGIN ******/
	bool CanDoScan() const;

	FReply DoScan();
	FReply ToggleMatchMaker();

	FReply OnGetAddressRequest();
	void AllocateAccessAddressFromMatchmaker(TUniqueFunction<void(FString)> Response);

	void RunBatScriptWithOutput(const FString& BatPath);
	void LaunchMatchmakerBatchServer(const FString& BatchScriptPath,TUniqueFunction<void()> Callback);

	void StartScan();

	void StopBackgroundThread();

	void StopScan();
	void CreateServerItem(const FBackupServerInfo& Config, bool bIsBackupServer = true);
	void CopyServer(FBackupServerInfo Config, FString Name);
	void DeleteServer(FBackupServerInfo Config, FString Name, SPSServerSingleton* Target);
	void ServerStateChanged(FBackupServerInfo Config, EServerState State);
	void RenameServer(SPSServerSingleton* Target, FBackupServerInfo Config, FString NewName);
	void ChangePort(FBackupServerInfo& Config, EPortType PortType, FString& NewPort, uint16& OldPort);

	/****** SCAN TASK END ******/

	/****** COMMON FUNCTIONS BEGIN ******/

	void ShowLoadingWidget(FString DisplayName);

	void HiddenLoadingWidget();

	FString AllocServerName(FString InPreferName, int SuffixIndex = 0);
	void AllocPorts(SignallingServerConfig& Config);
	bool IsPortAvailable(FString& Port);

	void AddExistsServerInformation(FBackupServerInfo Info);

	void RemoveExistsServerInformation(FBackupServerInfo Info);

	void UpdateExistsServerInformation(const FBackupServerInfo& From, const FBackupServerInfo& To);
	/****** COMMON FUNCTIONS END ******/
};
