#pragma once

#include "CoreMinimal.h"
#include "Styling/ISlateStyle.h"

class FPSManagerStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static const ISlateStyle& Get();

private:

	static TSharedRef<FSlateStyleSet> Create();

	static TSharedPtr<FSlateStyleSet> StyleSet;
	
};
