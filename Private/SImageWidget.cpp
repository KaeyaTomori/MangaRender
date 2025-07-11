#include "SImageWidget.h"

#include "FileManager.h"

SImageWidget::SImageWidget()
{
}

SImageWidget::~SImageWidget()
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SImageWidget::Construct(const FArguments& InArgs)
{
	// Brush = InArgs._Brush;
}

void SImageWidget::update(FString fileName)
{
	TArray<uint8> fileData;
	FileManager::GetImageData(fileName, fileData, imageWidth, imageHeight);
	Brush = FSlateDynamicImageBrush::CreateWithImageData(*fileName, FVector2D(imageWidth, imageHeight), fileData);
}

void SImageWidget::UpdateMove(FVector2D imageOffset)
{
	ImageOffset = imageOffset;
}

void SImageWidget::UpdateScroll(FVector2D renderPivot, float zoomFactor)
{
	RenderPivot = renderPivot;
	ZoomFactor = zoomFactor;
}

bool fir = false;
int32 SImageWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (Brush.IsValid() && !fir)
	{
		FVector2D ImageSize = Brush->ImageSize;
	 	FVector2d s = AllottedGeometry.GetLocalSize();;
		FVector2D LayoutOffset = -ImageSize * 0.5f + s * 0.5f;
		// 可以修改 RenderTransform 来控制位置或缩放
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(imageWidth, imageHeight),
				FSlateLayoutTransform(LayoutOffset),
				FSlateRenderTransform(FScale2D(ZoomFactor), ImageOffset),
				FVector2D(0.5f, 0.5f)
				),
			Brush.Get(),
			ESlateDrawEffect::None,
			InWidgetStyle.GetColorAndOpacityTint()
		);
	}
	return LayerId;
}

FVector2D SImageWidget::ComputeDesiredSize(float) const
{
	return Brush.IsValid() ? Brush.Get()->ImageSize : FVector2D(0, 0);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
