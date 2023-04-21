#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class SPSServerSingleton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPSServerSingleton)
	{
		
	}
	SLATE_END_ARGS()
	SPSServerSingleton();
	~SPSServerSingleton();

	void Construct(const FArguments& InArgs);
	
public:
	
protected:

private:
};
