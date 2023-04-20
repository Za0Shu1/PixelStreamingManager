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

DECLARE_LOG_CATEGORY_EXTERN(LogPixelStreamingManager, Log, All);

#define LOCTEXT_NAMESPACE "PixelStreamingManager"
#define FromHex(Hex) FLinearColor::FromSRGBColor(FColor::FromHex(Hex))

DECLARE_DELEGATE(FOnScanFinished)

class FDoScanTask : public FNonAbandonableTask
{
public:
	friend class FAsyncTask<FDoScanTask>;
	FOnScanFinished OnScanFinished;
	FDoScanTask(FOnScanFinished Callback);

	void DoWork() const;
	
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(DoScanTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};

class FPixelStreamingManager
{
public:
	FPixelStreamingManager(FSlateApplication& InSlate);
	~FPixelStreamingManager();
	
private:
	FSlateApplication& Slate;
	TSharedPtr<class STextBlock> PathText;
	FString WebServerPath = "";
	TSharedPtr<class SCircularThrobber> ScanningThrobber;
	bool bIsScanning = false;
	bool bScanFinished = false;
	
	/** To know if the ticker was started.*/
	FTSTicker::FDelegateHandle TickHandle;

public:
	
	bool Tick(float UnusedDeltaTime);

	void StartTicker();

	void Run();

	FReply PickupFolder();

	FText GenerateScanToolTip() const;

	bool CanDoScan() const;

	FReply DoScan();

	void StartScan();

	void OnScanTaskFinished()
	{
		bScanFinished = true;
	}
	
	void StopScan();
};