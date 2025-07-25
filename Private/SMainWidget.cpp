#include "SMainWidget.h"

#include "DataManager.h"
#include "SButtonMenu.h"

static SMainWidget* mainWidget;

void SMainWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	update();
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

SMainWidget* SMainWidget::GetInstance()
{
	return mainWidget;
}

void SMainWidget::OnReadModeChanged()
{
	if (data->GetReadMode() == EReadMode::SINGLE_PAGE)
	{
		data->SwitchReadMode(EReadMode::DOUBLE_PAGE);
	}
	else
	{
		data->SwitchReadMode(EReadMode::SINGLE_PAGE);
	}
	Construct(FArguments());
}

void SMainWidget::OnShowDirectionChanged()
{
	if (data->GetShowDirection() == LEFT_TO_RIGHT)
	{
		data->SwitchShowDirection(RIGHT_TO_LEFT);
	}
	else
	{
		data->SwitchShowDirection(LEFT_TO_RIGHT);
	}
}

SMainWidget::SMainWidget()
{
	data = DataManager::getInstance();
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

	if (data->GetReadMode() == EReadMode::SINGLE_PAGE)
	{
		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					SAssignNew(imageWidgetL, SImageWidget)
					.ImageAlignment(EImageAlignment::CENTER)
				]
			]
			
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SButtonMenu)
			]
		];
	}
	else if (data->GetReadMode() == EReadMode::DOUBLE_PAGE)
	{
		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					SAssignNew(imageWidgetL, SImageWidget)
					.ImageAlignment(EImageAlignment::RIGHT)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					SAssignNew(imageWidgetR, SImageWidget)
					.ImageAlignment(EImageAlignment::LEFT)
				]
			]
			
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SButtonMenu)
			]
		];
	}

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
		return FReply::Unhandled();
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
				data->OpenFolder(Path);
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

void SMainWidget::updateImageWidget(TSharedPtr<SImageWidget> imageWidget, int showIndex) {
	if (imageWidget.IsValid())
	{
		imageWidget.Get()->update(showIndex, ImageOffset, ZoomFactor, RenderPivot);
	}
}

void SMainWidget::update()
{
	if (data->IsDirty())
	{
		data->AlreadyUpdate();
		ImageOffset = DefaultImageOffset;
		RenderPivot = DefaultRenderPivot;
		ZoomFactor = DefaultZoomFactor;
		FirstImageToShow = data->GetCurrentImageIndex();
		updateImageWidget(imageWidgetL, FirstImageToShow + data->GetShowDirection());
		updateImageWidget(imageWidgetR, FirstImageToShow + (data->GetShowDirection() ^ 1));
	}
}

void SMainWidget::NextPage()
{
	data->NextPage();
}

void SMainWidget::LastPage()
{
	data->LastPage();
}