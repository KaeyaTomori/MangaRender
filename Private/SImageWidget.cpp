#include "SImageWidget.h"

#include "Tool/FileManager.h"

SImageWidget::SImageWidget()
{
	data = DataManager::getInstance();
}

SImageWidget::~SImageWidget()
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SImageWidget::Construct(const FArguments& InArgs)
{
	ImageAlignment = InArgs._ImageAlignment;
}

void SImageWidget::update(int showIndex)
{
	Brush = data->GetBrush(showIndex);
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

int32 SImageWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (Brush.IsValid())
	{
		FVector2D ImageSize = Brush->ImageSize;
	 	FVector2D windowSize = AllottedGeometry.GetLocalSize();
		// FVector2D LayoutOffset = -ImageSize * 0.5f + windowSize * 0.5f;
		FVector2D LayoutOffset(0, -ImageSize.Y * 0.5f + windowSize.Y * 0.5f);
		FVector2D pivot;

		float xScal = windowSize.X / ImageSize.X;
		float yScal = windowSize.Y / ImageSize.Y;
		auto scal = std::min(xScal, yScal) * ZoomFactor;

		switch (ImageAlignment)
		{
		case EImageAlignment::LEFT:
			LayoutOffset.X = 0.f;
			pivot = FVector2D(0.f, 0.5f);
			break;
		case EImageAlignment::RIGHT:
			LayoutOffset.X = -ImageSize.X + windowSize.X;
			pivot = FVector2D(1.f, 0.5f);
			break;
		case EImageAlignment::CENTER:
			LayoutOffset.X = -ImageSize.X * 0.5f + windowSize.X * 0.5f;
			pivot = FVector2D(0.5f, 0.5f);
			break;
		}
		
		// 可以修改 RenderTransform 来控制位置或缩放
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(
				Brush->ImageSize,
				FSlateLayoutTransform(LayoutOffset),
				FSlateRenderTransform(FScale2D(scal), ImageOffset),
				pivot
				),
			Brush.Get(),
			ESlateDrawEffect::None,
			InWidgetStyle.GetColorAndOpacityTint()
		);
	}
	else
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			FAppStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			FLinearColor(0.01f, 0.01f, 0.01f)
		);
	}
	return LayerId;
}

FVector2D SImageWidget::ComputeDesiredSize(float) const
{
	return Brush.IsValid() ? Brush.Get()->ImageSize : FVector2D(0, 0);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
