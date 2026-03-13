#include "SMainWidget.h"

#include "MangaImageCache.h"
#include "SButtonMenu.h"
#include "SMangaPage.h"
#include "SThumbnailSidebar.h"

static SMainWidget* mainWidget;

void SMainWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Update();
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

SMainWidget* SMainWidget::GetInstance()
{
	return mainWidget;
}

void SMainWidget::OnReadModeChanged()
{
	if (ImageCache->GetReadMode() == EReadMode::SINGLE_PAGE)
	{
		ImageCache->SwitchReadMode(EReadMode::DOUBLE_PAGE);
	}
	else
	{
		ImageCache->SwitchReadMode(EReadMode::SINGLE_PAGE);
	}
	Construct(FArguments());
}

void SMainWidget::OnShowDirectionChanged()
{
	if (ImageCache->GetShowDirection() == LEFT_TO_RIGHT)
	{
		ImageCache->SwitchShowDirection(RIGHT_TO_LEFT);
	}
	else
	{
		ImageCache->SwitchShowDirection(LEFT_TO_RIGHT);
	}
}

SMainWidget::SMainWidget()
{
	ImageCache = FMangaImageCache::GetInstance();
	mainWidget = this;
}

SMainWidget::~SMainWidget()
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMainWidget::Construct(const FArguments& InArgs)
{
	imageWidgetL = nullptr;
	imageWidgetR = nullptr;
	
	// 创建侧边栏
	TSharedPtr<SThumbnailSidebar> Sidebar = SNew(SThumbnailSidebar)
		.OnThumbnailSelected(this, &SMainWidget::OnThumbnailSelected);
	ThumbnailSidebar = Sidebar;
	ThumbnailSidebar->SetImageCache(ImageCache);
	
	// 主内容区域
	TSharedRef<SWidget> MainContent = (ImageCache->GetReadMode() == EReadMode::SINGLE_PAGE)
		? SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					SAssignNew(imageWidgetL, SMangaPage)
					.ImageAlignment(EImageAlignment::CENTER)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SButtonMenu)
			]
		: SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					SAssignNew(imageWidgetL, SMangaPage)
					.ImageAlignment(EImageAlignment::RIGHT)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					SAssignNew(imageWidgetR, SMangaPage)
					.ImageAlignment(EImageAlignment::LEFT)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SButtonMenu)
			];

	// 整体布局：侧边栏 + 主内容
	ChildSlot
	[
		SNew(SHorizontalBox)
		// 侧边栏
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.Visibility_Lambda([this]() { return bShowThumbnailSidebar ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				ThumbnailSidebar.ToSharedRef()
			]
		]
		// 主内容区
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			MainContent
		]
	];
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMainWidget::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
}

void SMainWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
}

FReply SMainWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// UE_LOG(LogTemp, Display, TEXT("OnMouseButtonDown"));
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled();
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton)
	{
		LastPage();
		return FReply::Handled();
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton2)
	{
		NextPage();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SMainWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		
	}
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SMainWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FVector2D delta = MouseEvent.GetCursorDelta();
		ImageOffset += delta;
		imageWidgetL.Get()->UpdateOffset(ImageOffset);
		if (imageWidgetR.IsValid())
		{
			imageWidgetR.Get()->UpdateOffset(ImageOffset);
		}
	}
	
	return FReply::Handled();
}

FReply SMainWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// if (MouseEvent.IsLeftControlDown())
	{
		// 获取鼠标相对于控件的位置（本地坐标）
		FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		FVector2D WidgetSize = MyGeometry.GetLocalSize();

		// 归一化为0~1之间，作为缩放锚点
		RenderPivot = FVector2D(
			FMath::Clamp(LocalMousePosition.X / WidgetSize.X, 0.f, 1.f),
			FMath::Clamp(LocalMousePosition.Y / WidgetSize.Y, 0.f, 1.f)
		);
	}
	{
		float wheelDelta = MouseEvent.GetWheelDelta();
		ZoomFactor = FMath::Clamp(ZoomFactor + wheelDelta * WheelSpeed, 0.1f, 10.f);

		imageWidgetL.Get()->UpdateZoomFactor(ZoomFactor, RenderPivot);
		if (imageWidgetR.IsValid())
		{
			imageWidgetR.Get()->UpdateZoomFactor(ZoomFactor, RenderPivot);
		}
	}

	// if (imageWidgetL.IsValid())
	// {
	// 	imageWidgetL->Invalidate(EInvalidateWidget::Layout);
	// }
	return SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
}

FReply SMainWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Left)
	{
		LastPage();		 
	}
	else if (InKeyEvent.GetKey() == EKeys::Right)
	{
		NextPage();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SMainWidget::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton)
	{
		LastPage();
		return FReply::Handled();
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton2)
	{
		NextPage();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SMainWidget::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	UE_LOG(LogTemp, Display, TEXT("OnDrop"));
	auto operation = DragDropEvent.GetOperationAs<FExternalDragOperation>();
	if (operation.IsValid())
	{
		const TArray<FString>& DroppedPaths = operation->GetFiles();

		for (const FString& Path : DroppedPaths)
		{
			if (FPaths::DirectoryExists(Path))
			{
				UE_LOG(LogTemp, Display, TEXT("folder: %s"), *Path);
				ImageCache->OpenFolder(Path);
			}
			else if (FPaths::FileExists(Path))
			{
				UE_LOG(LogTemp, Display, TEXT("file: %s"), *Path);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("drop unknow type: %s"), *Path);
			}
		}
		bIsDragAccept = false;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SMainWidget::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	UE_LOG(LogTemp, Display, TEXT("OnDragEnter"));
	auto operation = DragDropEvent.GetOperationAs<FExternalDragOperation>();
	if (operation.IsValid())
	{
		const TArray<FString>& DroppedPaths = operation->GetFiles();

		for (const FString& Path : DroppedPaths)
		{
			if (FPaths::DirectoryExists(Path))
			{
				operation->SetCursorOverride(EMouseCursor::SlashedCircle);
				bIsDragAccept = true;
			}
			else if (FPaths::FileExists(Path))
			{
				operation->SetCursorOverride(EMouseCursor::SlashedCircle);
				bIsDragAccept = true;
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("drag unknow type: %s"), *Path);
				bIsDragAccept = false;
			}
		}
	}
	SCompoundWidget::OnDragEnter(MyGeometry, DragDropEvent);
}

void SMainWidget::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	bIsDragAccept = false;
	SCompoundWidget::OnDragLeave(DragDropEvent);
}

FReply SMainWidget::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (bIsDragAccept)
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SMainWidget::updateImageWidget(TSharedPtr<SMangaPage> imageWidget, int showIndex) {
	if (imageWidget.IsValid())
	{
		imageWidget.Get()->update(showIndex, ImageOffset, ZoomFactor, RenderPivot);
	}
}

void SMainWidget::Update()
{
	if (ImageCache->IsDirty())
	{
		ImageCache->AlreadyUpdate();
		ImageOffset = DefaultImageOffset;
		RenderPivot = DefaultRenderPivot;
		ZoomFactor = DefaultZoomFactor;
		FirstImageToShow = ImageCache->GetCurrentImageIndex();
		updateImageWidget(imageWidgetL, FirstImageToShow + ImageCache->GetShowDirection());
		updateImageWidget(imageWidgetR, FirstImageToShow + (ImageCache->GetShowDirection() ^ 1));

		// 同步当前页到侧边栏
		// if (ThumbnailSidebar.IsValid())
		// {
		// 	int32 CurrentPage = ImageCache->GetCurrentImageIndex() / ImageCache->GetReadMode() + 1;
		// 	ThumbnailSidebar->SetSelectedPage(CurrentPage - 1);
		// }
	}
}

void SMainWidget::NextPage()
{
	ImageCache->NextPage();
}

void SMainWidget::LastPage()
{
	ImageCache->LastPage();
}

void SMainWidget::OnThumbnailSelected(int32 PageIndex)
{
	// 直接跳转到指定页
	ImageCache->ChangePageTo(ImageCache->PictrueIndexToPage(PageIndex));

	// 更新侧边栏选中状态
	if (ThumbnailSidebar.IsValid())
	{
		ThumbnailSidebar->SetSelectedPage(PageIndex);
		ThumbnailSidebar->ScrollToThumbnail(PageIndex);
	}
}

void SMainWidget::SyncThumbnailSelection()
{
	if (!ThumbnailSidebar.IsValid())
		return;

	int32 CurrentPage = ImageCache->GetCurrentImageIndex() / ImageCache->GetReadMode();
	ThumbnailSidebar->SetSelectedPage(CurrentPage);
	ThumbnailSidebar->ScrollToThumbnail(CurrentPage);
}