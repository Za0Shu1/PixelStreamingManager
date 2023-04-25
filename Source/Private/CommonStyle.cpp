#include "CommonStyle.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr< FSlateStyleSet > FPSManagerStyle::StyleSet = nullptr;

void FPSManagerStyle::Initialize()
{
	if(!StyleSet.IsValid())
	{
		StyleSet = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}
}

void FPSManagerStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
	ensure(StyleSet.IsUnique());
	StyleSet.Reset();
}

const ISlateStyle& FPSManagerStyle::Get()
{
	return *StyleSet;
}

TSharedRef<FSlateStyleSet> FPSManagerStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("PSManagerStyle"));

	Style->SetContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
	Style->Set("CustomAppIcon",new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Icons/Icon64.png"),FVector2d(64,64)));
	Style->Set("Stop",new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Icons/Stop64.png"),FVector2d(32)));
	Style->Set("Run",new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Icons/Play64.png"),FVector2d(32)));
	Style->Set("Background",new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Custom/background.png"),FVector2d(2560,1440)));
	return Style;
}
