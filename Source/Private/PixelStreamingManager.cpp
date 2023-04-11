// Copyright Epic Games, Inc. All Rights Reserved.


#include "PixelStreamingManager.h"

#include "RequiredProgramMainCPPInclude.h"

#include "Framework/Application/SlateApplication.h"
#include "StandaloneRenderer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"
#include "Widgets/Images/SImage.h"

DEFINE_LOG_CATEGORY_STATIC(LogPixelStreamingManager, Log, All);
/*
DECLARE_LOG_CATEGORY_EXTERN (LogMySlate, Log, All );
DEFINE_LOG_CATEGORY (LogMySlate)
*/
IMPLEMENT_APPLICATION(PixelStreamingManager, "PixelStreamingManager");

//INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
//{
//	GEngineLoop.PreInit(ArgC, ArgV);
//	UE_LOG(LogPixelStreamingManager, Display, TEXT("Hello World"));
//	return 0;
//}
int WINAPI WinMain(_In_ HINSTANCE hInInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int nCmdShow)
{
	GEngineLoop.PreInit(GetCommandLineW());
	FSlateApplication::InitializeAsStandaloneApplication(GetStandardStandaloneRenderer());
	const FSlateBrush* icon = FCoreStyle::Get().GetBrush(TEXT("TrashCan"));
	
	// 建立窗口
	TSharedPtr<SWindow> MainWindow = SNew(SWindow).ClientSize(FVector2D(1280, 720))
		[
			SNew(SImage).Image(icon)// 这里以Image做示范
		];
	FSlateApplication::Get().AddWindow(MainWindow.ToSharedRef());
	
	//UE_LOG(LogPixelStreamingManager, Display, TEXT("Hello World"));

	// 消息循环
	while (!IsEngineExitRequested())
	{
		FSlateApplication::Get().Tick();
		FSlateApplication::Get().PumpMessages();
	}

	FSlateApplication::Shutdown();
	return 0;
}
