#include "SMainWidget.h"

#include "FileManager.h"
#include "SFileButton.h"
#include "Fonts/FontMeasure.h"

SMainWidget::SMainWidget()
{
}

SMainWidget::~SMainWidget()
{
}

FVector2D CurrentOffset(0, 0);

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMainWidget::Construct(const FArguments& InArgs)
{
	
	SOverlay::FOverlaySlot* Slot;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Expose(Slot)
		[
			SNew(SBox)
			.WidthOverride(2000)
			.HeightOverride(3000)
			[
				SNew(SImage)
				.Image(FileManager::OpenAllFileInFolder(FString("D:/acg")).Get())
				.DesiredSizeOverride(FVector2D(2000, 3000))
				.RenderTransform_Lambda([this]()
				{
					return FSlateRenderTransform(FVector2D(-CurrentOffset.X, -CurrentOffset.Y));
				})
				// .OnMouseButtonDown_Lambda([this](const FGeometry&, const FPointerEvent& Event)
				// {
				// 	if (Event.IsMouseButtonDown(EKeys::LeftMouseButton))
				// 	{
				// 		CurrentOffset += Event.GetCursorDelta();
				// 		return FReply::Handled();
				// 	}
				// 	return FReply::Unhandled();
				// })
			]
			// SNew(SImage)
			// .Image(FileManager::OpenAllFileInFolder(FString("D:/acg")).Get())
			// .DesiredSizeOverride(FVector2D(400, 600))

		]
		// + SOverlay::Slot()
		// .HAlign(HAlign_Left)
		// .VAlign(VAlign_Top)
		// [
		// 	SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth()
		// 	[
		// 		SNew(SFileButton).Text(NSLOCTEXT("L10N", "ButtonContent", "Button"))
		// 	]
		// ]
	];

	Slot->SetHorizontalAlignment(HAlign_Left);
	// SWidget& SlotWidget = Slot->GetWidget().Get();
	// SImage& Image = static_cast<SImage&>(SlotWidget);
	// Image.SetImage();
	
	// SHorizontalBox& HorizontalBox = static_cast<SHorizontalBox&>(SlotWidget);
	// for (int i = 0; i < 5; ++i)
	// {
	// 	HorizontalBox.AddSlot()
	// 	[
	// 		SNew(SButton).Text(NSLOCTEXT("L10N", "ButtonContent", "Button"))
	// 	];
	// }// 对于现在的你来说，应该已经完全理解了此处中括号的使用。
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

// int32 SMainWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
// 	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
// {
// 	
// 	
// 	// 绘制Box背景
// 	// FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Brush, ESlateDrawEffect::None, BackgroundColor);
// 	// ++LayerId;
// 	
// 	// 绘制文字
// 	// FText ShowText = FText::FromString("Hello Slate");
// 	// FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Bold",30);
// 	//
// 	// const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
// 	// FVector2D TextSize = FontMeasureService.Get().Measure(ShowText, FontInfo);
// 	// FVector2D AreaSize = AllottedGeometry.GetLocalSize();
// 	// FVector2D TextPosition = (AreaSize - TextSize) / 2.f;
// 	// FSlateLayoutTransform TextLayoutTransform = FSlateLayoutTransform(FVector2D(TextPosition.X, TextPosition.Y));
// 	// FPaintGeometry TextGeometry = AllottedGeometry.ToPaintGeometry();
// 	// TextGeometry.AppendTransform(TextLayoutTransform);
// 	// FSlateDrawElement::MakeText(OutDrawElements, LayerId, TextGeometry, ShowText, FontInfo);
// 	
// 	// 子部件绘制交给SCompoundWidget::OnPaint()
// 	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
// }

void SMainWidget::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	BackgroundColor = FLinearColor(0.6f, 0.5f, 0.9f);
}

void SMainWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	BackgroundColor = FLinearColor(0.1f, 0.5f, 0.9f);
}

FReply SMainWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		LastMouseDownPosition = MouseEvent.GetScreenSpacePosition();
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	
	return FReply::Handled();
}

FReply SMainWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SMainWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
		FVector2D CurrentWindowPosition = Window->GetPositionInScreen();
		FVector2D CurrentMousePosition = MouseEvent.GetScreenSpacePosition();
		FVector2D TargetPosition = CurrentWindowPosition - LastMouseDownPosition + CurrentMousePosition;
		Window->MoveWindowTo(TargetPosition);

		LastMouseDownPosition = CurrentMousePosition;
	}

	return FReply::Handled();
}

FVector2D SMainWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	FText ShowText = FText::FromString("Hello Slate");
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Bold",30);
	const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	FVector2D TextSize = FontMeasureService.Get().Measure(ShowText, FontInfo);

	return TextSize;
}
