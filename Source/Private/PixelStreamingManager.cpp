// Copyright Epic Games, Inc. All Rights Reserved.


#include "../Public/PixelStreamingManager.h"

#include "RequiredProgramMainCPPInclude.h"

#include "Framework/Application/SlateApplication.h"
#include "StandaloneRenderer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"
#include "Widgets/Images/SImage.h"

DEFINE_LOG_CATEGORY_STATIC(LogPixelStreamingManager, Log, All);

IMPLEMENT_APPLICATION(PixelStreamingManager, "PixelStreamingManager");

int WINAPI WinMain(_In_ HINSTANCE hInInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int nCmdShow)
{
	GEngineLoop.PreInit(GetCommandLineW());

	FSlateApplication::InitializeAsStandaloneApplication(GetStandardStandaloneRenderer());
	const FSlateBrush* icon = FCoreStyle::Get().GetBrush(TEXT("TrashCan"));

	//@todo how to get a slate brush from resources file
	//FSlateApplication::Get().SetAppIcon();
	
	// 建立窗口
	TSharedPtr<SWindow> MainWindow = SNew(SWindow).ClientSize(FVector2D(1280, 720))
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SImage).Image(icon)
			]
		];
	FSlateApplication::Get().AddWindow(MainWindow.ToSharedRef());
	
	// 消息循环
	while (!IsEngineExitRequested())
	{
		FSlateApplication::Get().Tick();
		FSlateApplication::Get().PumpMessages();
	}

	FSlateApplication::Shutdown();
	return 0;
}
