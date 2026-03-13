#include "SThumbnailSidebar.h"

#include "MangaImageCache.h"
#include "Utils/FImageDecoder.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"

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
	ImageCache = InCache;
	RefreshThumbnails();
}

void SThumbnailSidebar::RefreshThumbnails()
{
	ThumbnailBrushes.Empty();
	ThumbnailWidgets.Empty();
	ThumbnailContainer->ClearChildren();

	if (!ImageCache)
	{
		return;
	}

	RebuildThumbnailList();
	GenerateThumbnailsAsync();
}

void SThumbnailSidebar::RebuildThumbnailList()
{
	TArray<FString>& FileNames = ImageCache->GetFileNames();
	int32 PageCount = FileNames.Num();

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
	return FCoreStyle::Get().GetBrush("Checkerboard");
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
	if (bIsGeneratingThumbnails.Load())
	{
		bIsGeneratingThumbnails.Store(false);
		// 等待之前的生成任务完成
		FPlatformProcess::Sleep(0.01f);
	}

	bIsGeneratingThumbnails.Store(true);

	Async(EAsyncExecution::ThreadPool, [this]()
	{
		TArray<FString>& FileNames = ImageCache->GetFileNames();

		for (int32 i = 0; i < FileNames.Num() && bIsGeneratingThumbnails.Load(); ++i)
		{
			const FString& FilePath = FileNames[i];

			// 加载图片数据
			TArray<uint8> FileData;
			
			FScopeLock Lock(&ThumbnailLock);
			TSharedPtr<FSlateDynamicImageBrush> Brush = nullptr;
			if (FMangaImageCache::GetInstance()->GetImageRawData(FilePath, FileData))
			{
				Brush = FSlateDynamicImageBrush::CreateWithImageData(
					*FilePath,
					FVector2D(ThumbnailWidth, ThumbnailHeight),
					FileData
				);
			}
			
			// int32 OrigWidth, OrigHeight;
			//
			// if (!FImageDecoder::GetImageData(FilePath, FileData, OrigWidth, OrigHeight))
			// {
			// 	continue;
			// }
			//
			// // 计算缩略图尺寸（保持宽高比）
			// float ScaleX = ThumbnailWidth / OrigWidth;
			// float ScaleY = ThumbnailHeight / OrigHeight;
			// float Scale = FMath::Min(ScaleX, ScaleY);
			//
			// int32 ThumbWidth = FMath::RoundToInt(OrigWidth * Scale);
			// int32 ThumbHeight = FMath::RoundToInt(OrigHeight * Scale);
			//
			// FImageDecoder::GetImageData(FilePath, FileData, OrigWidth, OrigHeight);
			//
			// // 创建缩略图Brush
			// FScopeLock Lock(&ThumbnailLock);
			// TSharedPtr<FSlateDynamicImageBrush> Brush =FSlateDynamicImageBrush::CreateWithImageData(
			// 			*FilePath,
			// 			FVector2D(ThumbWidth, ThumbHeight),
			// 			FileData
			// 		);
			
			ThumbnailBrushes.Add(i, Brush);

			// 在主线程刷新UI
			AsyncTask(ENamedThreads::GameThread, [this, i]()
			{
				if (ThumbnailWidgets.IsValidIndex(i) && ThumbnailWidgets[i].IsValid())
				{
					ThumbnailWidgets[i]->Invalidate(EInvalidateWidget::PaintAndVolatility);
				}
			});
		}

		bIsGeneratingThumbnails.Store(false);
	});
}
