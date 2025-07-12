#pragma once
#include "DataManager.h"

enum EImageAlignment : int
{
	CENTER,
	LEFT,
	RIGHT,
};

class SImageWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SImageWidget){}
		SLATE_ARGUMENT(EImageAlignment, ImageAlignment)
	SLATE_END_ARGS()
	
public:
	SImageWidget();
	~SImageWidget();
	void Construct(const FArguments& InArgs);

	void update(int showIndex);
	void UpdateMove(FVector2D imageOffset);
	void UpdateScroll(FVector2D renderPivot, float zoomFactor);
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	virtual FVector2D ComputeDesiredSize(float) const override;

	FVector2D ImageOffset = FVector2D::ZeroVector;
	FVector2D RenderPivot = FVector2D(0.5f, 0.5f);
	float ZoomFactor = 1.0f;
	int32 imageWidth = 1080;
	int32 imageHeight = 720;

	EImageAlignment ImageAlignment = EImageAlignment::CENTER;
	
	TSharedPtr<FSlateBrush> Brush;
	
	/** 图片控件 */
	TSharedPtr<SImage> ImageWidget;
	TSharedPtr<SBox> Box;

	DataManager* data;
};