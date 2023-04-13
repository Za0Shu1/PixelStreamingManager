#pragma once

#include "SlateCore.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/ITableRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableViewBase.h"

struct FPSServerConfig
{
public:
	FFilePath WebServersPath;
};


class SServerConfigWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SServerConfigWidget) {}
	SLATE_END_ARGS()

	SServerConfigWidget(){}
	~SServerConfigWidget(){}

	void Construct(const FArguments& Args);

	/* Adds a new textbox with the string to the list */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FPSServerConfig> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/* The list of strings */
	TArray<TSharedPtr<FPSServerConfig>> Items;

	/* The actual UI list */
	TSharedPtr< SListView< TSharedPtr<FPSServerConfig> > > ListViewWidget;
};
