#include "SMainWidget.h"

#include "FileManager.h"
#include "SFileButton.h"

SMainWidget::SMainWidget()
{
}

SMainWidget::~SMainWidget()
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMainWidget::Construct(const FArguments& InArgs)
{
	// "D:/acg/111426387_p0.jpg"
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(imageWidgetL, SImageWidget)
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
		if (showImageIndex > 0)
		{
			showImageIndex--;
			isImageChange = true;
		}
		UE_LOG(LogTemp, Display, TEXT("ThumbMouseButton Clicked"));
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton2)
	{
		{
			showImageIndex++;
			isImageChange = true;
		}
		UE_LOG(LogTemp, Display, TEXT("ThumbMouseButton2 Clicked"));
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
	}

	// if (ImageWidget.IsValid())
	// {
	// 	ImageWidget->Invalidate(EInvalidateWidget::Layout);
	// }
	return SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
}

FReply SMainWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Left)
	{
		if (showImageIndex > 0)
		{
			showImageIndex--;
			isImageChange = true;
		}
		 
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

void SMainWidget::update()
{
	auto manager = FileManager::getInstance();
	if (isImageChange || manager->isDirty)
	{
		manager->isDirty = false;
		isImageChange = false;
		ImageOffset = FVector2D::ZeroVector;
		RenderPivot = FVector2D(0.5f, 0.5f);
		ZoomFactor = 1.0f;
		imageWidgetL.Get()->update(manager->FileNames[showImageIndex]);
		imageWidgetL.Get()->UpdateScroll(RenderPivot, ZoomFactor);
		imageWidgetL.Get()->UpdateMove(ImageOffset);
	}
}

