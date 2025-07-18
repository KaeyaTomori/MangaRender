#include "SButtonMenu.h"

#include <Windows.h>
#include <minwindef.h>
#include <shobjidl_core.h>

#include "DataManager.h"
#include "SMainWidget.h"

SButtonMenu::SButtonMenu()
{
}

SButtonMenu::~SButtonMenu()
{
}

void SButtonMenu::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(NSLOCTEXT("L10N", "选择文件夹", "Open Folder"))
			.OnClicked_Lambda([]()->FReply
			{
				AsyncTask(ENamedThreads::GameThread, []()
				{
					auto data = DataManager::getInstance();
					data->OpenFolder();
				});
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(NSLOCTEXT("L10N", "ButtonContent", "切换单双页"))
			.OnClicked_Lambda([]()->FReply
			{
				AsyncTask(ENamedThreads::GameThread, []()
				{
					SMainWidget::GetInstance()->OnReadModeChanged();
				});
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(NSLOCTEXT("L10N", "ButtonContent", "切换左右顺序"))
			.OnClicked_Lambda([]()->FReply
			{
				AsyncTask(ENamedThreads::GameThread, []()
				{
					SMainWidget::GetInstance()->OnShowDirectionChanged();
				});
				return FReply::Handled();
			})
		]
	];
}
