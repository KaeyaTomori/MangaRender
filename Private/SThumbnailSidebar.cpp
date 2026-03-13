#include "SThumbnailSidebar.h"

#include "MangaImageCache.h"
#include "Utils/FImageDecoder.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"

SThumbnailSidebar::~SThumbnailSidebar()
{
	UnbindFromImageCache();
}

void SThumbnailSidebar::Construct(const FArguments& InArgs)
{
	OnThumbnailSelected = InArgs._OnThumbnailSelected;
	ThumbnailWidth = InArgs._ThumbnailWidth.Get();
	ThumbnailHeight = InArgs._ThumbnailHeight.Get();
	bShowPageNumber = InArgs._bShowPageNumber.Get();

	ChildSlot
	[
		SNew(SBorder)
		// .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
		[
			SNew(SVerticalBox)
			// 标题
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("ThumbnailSidebar", "Title", "Pages"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FLinearColor::White)
			]
			// 缩略图列表
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(ThumbnailContainer, SScrollBox)
				.Orientation(Orient_Vertical)
				.ScrollBarAlwaysVisible(true)
			]
		]
	];
}

void SThumbnailSidebar::SetImageCache(FMangaImageCache* InCache)
{
	UnbindFromImageCache();
	ImageCache = InCache;
	RefreshThumbnails();
	BindToImageCache();
}

void SThumbnailSidebar::BindToImageCache()
{
	
}

void SThumbnailSidebar::UnbindFromImageCache()
{
	
}

void SThumbnailSidebar::RefreshThumbnails()
{
	ThumbnailBrushes.Empty();
	ThumbnailWidgets.Empty();
	ThumbnailContainer->ClearChildren();
	SelectedPageIndex = 0;

	if (!ImageCache)
	{
		return;
	}

	RebuildThumbnailList();
}

void SThumbnailSidebar::RebuildThumbnailList()
{
	int32 PageCount = ImageCache->GetTotalPageCount();

	for (int32 i = 0; i < PageCount; ++i)
	{
		TSharedRef<SWidget> ThumbnailItem = CreateThumbnailItem(i);
		ThumbnailContainer->AddSlot()
			.AutoSize()
			[
				ThumbnailItem
			];
	}
}

TSharedRef<SWidget> SThumbnailSidebar::CreateThumbnailItem(int32 PageIndex)
{
	TSharedPtr<SBorder> ItemBorder;

	TSharedRef<SWidget> ItemWidget = SNew(SBox)
		.WidthOverride(ThumbnailWidth + 20.0f)
		.HeightOverride(ThumbnailHeight + (bShowPageNumber ? 25.0f : 10.0f))
		.Padding(5.0f)
		[
			SNew(SVerticalBox)
			// 缩略图图片
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(ItemBorder, SBorder)
				.BorderBackgroundColor(this, &SThumbnailSidebar::GetSelectionColor, PageIndex)
				.BorderImage(FCoreStyle::Get().GetBrush("Border"))
				.Padding(2.0f)
				[
					SNew(SBox)
					.WidthOverride(ThumbnailWidth)
					.HeightOverride(ThumbnailHeight)
					[
						SNew(SImage)
						.Image(this, &SThumbnailSidebar::GetThumbnailBrush, PageIndex)
					]
				]
			]
			// 页码
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				bShowPageNumber
				? SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("%d"), PageIndex + 1)))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
					.ColorAndOpacity(FLinearColor::White)
				: SNullWidget::NullWidget
			]
		];

	// 添加点击处理
	ItemBorder->SetOnMouseButtonDown(FPointerEventHandler::CreateLambda(
		[this, PageIndex](const FGeometry&, const FPointerEvent&)
		{
			return OnThumbnailClicked(PageIndex);
		}));

	ThumbnailWidgets.Add(ItemBorder);
	return ItemWidget;
}

FReply SThumbnailSidebar::OnThumbnailClicked(int32 PageIndex)
{
	SetSelectedPage(PageIndex);

	if (OnThumbnailSelected.IsBound())
	{
		OnThumbnailSelected.Execute(PageIndex);
	}

	return FReply::Handled();
}

const FSlateBrush* SThumbnailSidebar::GetThumbnailBrush(int32 PageIndex) const
{
	return FMangaImageCache::GetInstance()->GetBrush(PageIndex).Get();
}

FSlateColor SThumbnailSidebar::GetSelectionColor(int32 PageIndex) const
{
	if (PageIndex == SelectedPageIndex)
	{
		return FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // 蓝色高亮
	}
	return FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); // 默认边框色
}

void SThumbnailSidebar::SetSelectedPage(int32 PageIndex)
{
	SelectedPageIndex = PageIndex;

	// 刷新所有缩略图边框颜色
	for (int32 i = 0; i < ThumbnailWidgets.Num(); ++i)
	{
		if (ThumbnailWidgets[i].IsValid())
		{
			ThumbnailWidgets[i]->Invalidate(EInvalidateWidget::PaintAndVolatility);
		}
	}
}

void SThumbnailSidebar::ScrollToThumbnail(int32 PageIndex)
{
	if (!ThumbnailContainer.IsValid() || !ThumbnailWidgets.IsValidIndex(PageIndex))
	{
		return;
	}

	// 获取子控件的位置信息
    TSharedPtr<SWidget> TargetWidget = ThumbnailWidgets[PageIndex];
    if (TargetWidget.IsValid())
    {
        // 使用ScrollToChild方法（如果可用）
        ThumbnailContainer->ScrollDescendantIntoView(TargetWidget, true, EDescendantScrollDestination::Center);
    }
}

