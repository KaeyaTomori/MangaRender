#include "SMainWidget.h"

#include "DataManager.h"
#include "SFileButton.h"

SMainWidget::SMainWidget()
{
	data = DataManager::getInstance();
}

SMainWidget::~SMainWidget()
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMainWidget::Construct(const FArguments& InArgs)
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
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFileButton)
				.Text(NSLOCTEXT("L10N", "ButtonContent", "Open Folder"))
			]
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
	if(MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton)
	{
		LastPage();
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton2)
	{
		NextPage();
	}
	
	return FReply::Handled();
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
		imageWidgetL.Get()->UpdateMove(ImageOffset);
		if (imageWidgetR.IsValid())
		{
			imageWidgetR.Get()->UpdateMove(ImageOffset);
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
		// 控件大小
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

		imageWidgetL.Get()->UpdateScroll(RenderPivot, ZoomFactor);
		if (imageWidgetR.IsValid())
		{
			imageWidgetR.Get()->UpdateScroll(RenderPivot, ZoomFactor);
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
		{
			showImageIndex++;
			isImageChange = true;
		}
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SMainWidget::updateImageWidget(TSharedPtr<SImageWidget> imageWidget, int showIndex) {
	if (imageWidget.IsValid())
	{
		imageWidget.Get()->update(showIndex);
		imageWidget.Get()->UpdateScroll(RenderPivot, ZoomFactor);
		imageWidget.Get()->UpdateMove(ImageOffset);
	}
}

void SMainWidget::update()
{
	if (isImageChange || data->isDirty)
	{
		data->isDirty = false;
		isImageChange = false;
		ImageOffset = FVector2D::ZeroVector;
		RenderPivot = FVector2D(0.5f, 0.5f);
		ZoomFactor = 1.0f;
		updateImageWidget(imageWidgetL, showImageIndex);
		updateImageWidget(imageWidgetR, showImageIndex + 1);
	}
}

void SMainWidget::NextPage()
{
	if (showImageIndex < data->FileNames.Num() - pageCount)
	{
		showImageIndex += pageCount;
		isImageChange = true;
	}
}

void SMainWidget::LastPage()
{
	if (showImageIndex > 0)
	{
		showImageIndex -= pageCount;
		isImageChange = true;
	}
}

