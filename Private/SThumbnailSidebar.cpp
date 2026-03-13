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
	if (ImageCache)
	{
		OnImageLoadedHandle = ImageCache->OnImageLoaded.AddSP(
			SharedThis(this), &SThumbnailSidebar::OnImageLoaded);
	}
}

void SThumbnailSidebar::UnbindFromImageCache()
{
	if (ImageCache && OnImageLoadedHandle.IsValid())
	{
		ImageCache->OnImageLoaded.Remove(OnImageLoadedHandle);
		OnImageLoadedHandle.Reset();
	}
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

	// 尝试刷新已加载的图片缩略图
	for (int32 i = 0; i < ThumbnailWidgets.Num(); ++i)
	{
		TryRefreshThumbnail(i);
	}
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
	FScopeLock Lock(const_cast<FCriticalSection*>(&ThumbnailLock));

	if (const TSharedPtr<FSlateDynamicImageBrush>* BrushPtr = ThumbnailBrushes.Find(PageIndex))
	{
		return BrushPtr->Get();
	}

	// 返回默认占位图
	// return FCoreStyle::Get().GetBrush("Checkerboard");
	return nullptr;
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

void SThumbnailSidebar::GenerateThumbnailsAsync()
{
	// 不再使用：缩略图现在通过 OnImageLoaded 委托动态加载
	// 保留此方法用于兼容性，但不做任何操作
}

void SThumbnailSidebar::OnImageLoaded(int32 ImageIndex)
{
	if (!ImageCache) return;

	// 图片索引转换为页号
	int32 PageIndex = ImageCache->PictrueIndexToPage(ImageIndex);

	// 在主线程刷新对应缩略图
	AsyncTask(ENamedThreads::GameThread, [this, PageIndex]()
	{
		RefreshThumbnailAt(PageIndex);
	});
}

void SThumbnailSidebar::TryRefreshThumbnail(int32 PageIndex)
{
	if (!ImageCache || !ThumbnailWidgets.IsValidIndex(PageIndex))
	{
		return;
	}

	// 计算该页对应的图片索引
	int32 ImageIndex = PageIndex * static_cast<int32>(ImageCache->GetReadMode());

	// 检查图片是否已加载
	if (ImageCache->IsImageLoaded(ImageIndex))
	{
		RefreshThumbnailAt(PageIndex);
	}
}

void SThumbnailSidebar::RefreshThumbnailAt(int32 PageIndex)
{
	if (!ImageCache || !ThumbnailWidgets.IsValidIndex(PageIndex))
	{
		return;
	}

	// 计算该页对应的图片索引（取该页的第一张图）
	int32 ImageIndex = PageIndex * static_cast<int32>(ImageCache->GetReadMode());

	TArray<FString>& FileNames = ImageCache->GetFileNames();
	if (!FileNames.IsValidIndex(ImageIndex))
	{
		return;
	}

	// 尝试获取已加载的图片数据
	const FString& FilePath = FileNames[ImageIndex];
	TArray<uint8> FileData;

	if (ImageCache->GetImageRawData(FilePath, FileData))
	{
		// 创建缩略图Brush
		TSharedPtr<FSlateDynamicImageBrush> NewBrush = FSlateDynamicImageBrush::CreateWithImageData(
			*FilePath,
			FVector2D(ThumbnailWidth, ThumbnailHeight),
			FileData
		);

		{
			FScopeLock Lock(&ThumbnailLock);
			ThumbnailBrushes.Add(PageIndex, NewBrush);
		}

		// 只刷新该缩略图的UI
		if (ThumbnailWidgets[PageIndex].IsValid())
		{
			ThumbnailWidgets[PageIndex]->Invalidate(EInvalidateWidget::PaintAndVolatility);
		}
	}
}
