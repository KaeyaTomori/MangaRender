#pragma once

#include "CoreMinimal.h"

class FMangaImageCache;

DECLARE_DELEGATE_OneParam(FOnThumbnailSelected, int32 /* PageIndex */);

class SThumbnailSidebar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SThumbnailSidebar)
		: _ThumbnailWidth(120)
		, _ThumbnailHeight(160)
		, _bShowPageNumber(true)
	{}
		SLATE_EVENT(FOnThumbnailSelected, OnThumbnailSelected)
		SLATE_ATTRIBUTE(float, ThumbnailWidth)
		SLATE_ATTRIBUTE(float, ThumbnailHeight)
		SLATE_ATTRIBUTE(bool, bShowPageNumber)
	SLATE_END_ARGS()

	~SThumbnailSidebar();
	void Construct(const FArguments& InArgs);

	// 设置数据源
	void SetImageCache(FMangaImageCache* InCache);

	// 绑定/解绑 ImageCache 的委托
	void BindToImageCache();
	void UnbindFromImageCache();

	// 刷新缩略图列表（文件夹改变时调用）
	void RefreshThumbnails();

	// 滚动到指定缩略图并高亮
	void ScrollToThumbnail(int32 PageIndex);

	// 设置当前选中页
	void SetSelectedPage(int32 PageIndex);

protected:
	// 生成缩略图列表
	void RebuildThumbnailList();

	// 创建单个缩略图项
	TSharedRef<SWidget> CreateThumbnailItem(int32 PageIndex);

	// 点击缩略图
	FReply OnThumbnailClicked(int32 PageIndex);

	// 获取缩略图笔刷
	const FSlateBrush* GetThumbnailBrush(int32 PageIndex) const;

	// 获取当前选中状态
	FSlateColor GetSelectionColor(int32 PageIndex) const;

	// 图片加载完成回调
	void OnImageLoaded(int32 ImageIndex);

private:
	FMangaImageCache* ImageCache = nullptr;

	// 缩略图容器
	TSharedPtr<class SScrollBox> ThumbnailContainer;

	// 缩略图小部件缓存
	TArray<TSharedPtr<class SBorder>> ThumbnailWidgets;

	// 回调
	FOnThumbnailSelected OnThumbnailSelected;

	// 配置
	float ThumbnailWidth = 120.0f;
	float ThumbnailHeight = 160.0f;
	bool bShowPageNumber = true;

	// 当前选中页
	int32 SelectedPageIndex = 0;

	// 缩略图缓存（索引 -> 动态图像刷）
	TMap<int32, TSharedPtr<FSlateDynamicImageBrush>> ThumbnailBrushes;

	// 生成状态
	TAtomic<bool> bIsGeneratingThumbnails{ false };
	FCriticalSection ThumbnailLock;

	// 委托句柄
	FDelegateHandle OnImageLoadedHandle;
};
