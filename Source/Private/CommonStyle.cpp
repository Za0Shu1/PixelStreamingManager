#include "CommonStyle.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"

TSharedPtr<FSlateStyleSet> FPSManagerStyle::StyleSet = nullptr;

void FPSManagerStyle::Initialize()
{
	if (!StyleSet.IsValid())
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

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

TSharedRef<FSlateStyleSet> FPSManagerStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("PSManagerStyle"));

	Style->SetContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
	Style->Set("CustomAppIcon",
	           new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Icons/Icon64.png"),
	                                FVector2d(64, 64)));
	Style->Set("Stop", new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Icons/Stop64.png"),
	                                        FVector2d(32)));
	Style->Set("Run", new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Icons/Play64.png"),
	                                       FVector2d(32)));
	Style->Set("Background",
	           new FSlateImageBrush(FPaths::EngineContentDir() / TEXT("Slate") / TEXT("Custom/background.png"),
	                                FVector2d(509, 207), FLinearColor(1, 1, 1, 1), ESlateBrushTileType::Both));


	const FButtonStyle CopyButton = FButtonStyle()
	                                .SetNormal(IMAGE_BRUSH("/Icons/Copy", Icon20x20))
	                                .SetHovered(IMAGE_BRUSH("/Icons/Copy", Icon20x20))
	                                .SetPressed(IMAGE_BRUSH("/Icons/Copy", Icon20x20));

	Style->Set("CopyButton", CopyButton);

	const FButtonStyle DeleteButton = FButtonStyle()
	                                  .SetNormal(IMAGE_BRUSH("/Icons/Delete", Icon20x20))
	                                  .SetHovered(IMAGE_BRUSH("/Icons/Delete", Icon20x20))
	                                  .SetPressed(IMAGE_BRUSH("/Icons/Delete", Icon20x20));

	Style->Set("DeleteButton", DeleteButton);
	return Style;
}
