#pragma once

#include "DataManager.h"
#include "SImageWidget.h"

class SMainWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainWidget) {}
	SLATE_END_ARGS()

	static SMainWidget* GetInstance();
public:
	SMainWidget();
	~SMainWidget();
	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	// Mouse Event
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;


	void OnReadModeChanged();
	void OnShowDirectionChanged();
	
	void updateImageWidget(TSharedPtr<SImageWidget> imageWidget, int showIndex);
	void update();

private:
	void NextPage();
	void LastPage();
	
private:
	const float WheelSpeed = 0.1f;
	
	const FVector2D DefaultImageOffset = FVector2D::ZeroVector;
	const FVector2D DefaultRenderPivot = FVector2D(0.5f, 0.5f);
	const float DefaultZoomFactor = 1.0f;
	
	FVector2D ImageOffset = DefaultImageOffset;
	FVector2D RenderPivot = DefaultRenderPivot;
	float ZoomFactor = DefaultZoomFactor;
	int FirstImageToShow = 0;
	TSharedPtr<SImageWidget> imageWidgetL;
	TSharedPtr<SImageWidget> imageWidgetR;

	DataManager* data;

	bool bIsDragAccept = false;
};
