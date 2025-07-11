#pragma once

#include "SImageWidget.h"

class SMainWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainWidget){}
	SLATE_END_ARGS()
	
public:
	SMainWidget();
	~SMainWidget();
	void Construct(const FArguments& InArgs);

	// Mouse Event
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	void update();
	
private:
	FVector2D MousePosition = FVector2D::ZeroVector;
	FVector2D ImageOffset = FVector2D::ZeroVector;

	FVector2D RenderPivot = FVector2D(0.5f, 0.5f);
	const float WheelSpeed = 0.1f;
	float ZoomFactor = 1.0f;

	TSharedPtr<SImageWidget> imageWidgetL;

	bool isImageChange = false;
	int showImageIndex = 0;
	/** 图片控件 */
	TSharedPtr<SImage> ImageWidget;
};
