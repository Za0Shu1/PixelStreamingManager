#include "SServerConfigWidget.h"

#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "ServerConfigWidget"

void SServerConfigWidget::Construct(const FArguments& Args)
{
	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SAssignNew(ListViewWidget, SListView<TSharedPtr<FPSServerConfig>>)
			.ItemHeight(24)
			.ListItemsSource(&Items) //The Items array is the source of this listview
			.OnGenerateRow(this, &SServerConfigWidget::OnGenerateRowForList)
		]
	];
}

TSharedRef<ITableRow> SServerConfigWidget::OnGenerateRowForList(TSharedPtr<FPSServerConfig> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	//Create the row
	return
		SNew(STableRow< TSharedPtr<FPSServerConfig> >, OwnerTable)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText(LOCTEXT("PixelStreamingManager", "路径")))
		];
}