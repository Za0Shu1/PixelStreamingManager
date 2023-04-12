#include "../Public/CommonStyle.h"

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
	Style->Set("CustomAppIcon",new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Icons/Icon48.png"),FVector2d(40,40)));
	Style->Set("AppIcon",new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Icons/DefaultAppIcon.png"),FVector2d(20,20)));
	return Style;
}
