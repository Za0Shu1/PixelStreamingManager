#pragma once

#include "CoreMinimal.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateTypes.h"

class FPSManagerStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static const ISlateStyle& Get();

private:

	static TSharedRef<FSlateStyleSet> Create();

	static TSharedPtr<FSlateStyleSet> StyleSet;

	static inline FVector2d Icon20x20 = FVector2d(20.0f, 20.0f);
	static inline FVector2d Icon32x32 = FVector2d(32.0f, 32.0f);
	static inline FVector2d Icon48x48 = FVector2d(48.0f, 48.0f);
	static inline FVector2d Icon64x64 = FVector2d(64.0f, 64.0f);
};
