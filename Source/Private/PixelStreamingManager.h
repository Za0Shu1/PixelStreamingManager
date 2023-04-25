// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "DesktopPlatformModule.h"
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

	FString ScanFolder;
};

class FPixelStreamingManager
{
public:
	FPixelStreamingManager(FSlateApplication& InSlate);
	~FPixelStreamingManager();

private:
	FSlateApplication& Slate;
	TSharedPtr<STextBlock> PathText;
	TSharedPtr<SCircularThrobber> ScanningThrobber;
	TSharedPtr<SWrapBox> ServersContainer;
	TSharedPtr<SBox> ContainerArea;
	TSharedPtr<SOverlay> RightPanel;

	/** To know if the ticker was started.*/
	FTSTicker::FDelegateHandle GlobalTickHandle;
	FTSTicker::FDelegateHandle ScanTickHandle;

	FAsyncTask<FDoScanTask>* ScanTask;

	float RightPanelWidth;//右侧区域宽度
	float WrapBoxPadding = 10.f;
	float ItemHorizontalPadding = 5.f;//服务器UI横向间隔
	float ItemVerticalPadding = 2.f;//服务器UI纵向向间隔
	float ItemWidth = 300.f;
	float ItemHeight = 200.f;
	
	const float ScrollBarThickness = 9.f;
	const float ScrollBarPadding = 2.f;

	bool bUseMatchmaker = false;

public:
	/****** MANNULLY TICK BEGIN ******/ 
	bool ScanTaskTick(float UnusedDeltaTime);
	
	bool Tick(float UnusedDeltaTime);

	void StartScanTicker();

	/****** MANNULLY TICK END ******/ 

	/****** DRAW WINDOW BEGIN ******/

	void Run();

	void InitializeConfig();

	FReply PickupFolder();

	FText GenerateScanToolTip() const;
	FText GenerateLaunchMatchmakerToolTip() const;

	void OnWindowResized();

	/****** DRAW WINDOW END ******/ 


	/****** SCAN TASK BEGIN ******/ 
	bool CanDoScan() const;

	FReply DoScan();
	FReply LaunchMatchMaker();

	void StartScan();
	
	void StopBackgroundThread();

	void StopScan();
	void CreateServerItem(FString Name, int32 HttpPort, bool bIsEnable);

	/****** SCAN TASK END ******/ 

};