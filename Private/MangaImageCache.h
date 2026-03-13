#pragma once

#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnImageLoaded, int32 /* ImageIndex */);

enum EReadMode : int8
{
	SINGLE_PAGE = 1,
	DOUBLE_PAGE = 2,
	MULTI_PAGE
};

enum EShowDirection : int8
{
	LEFT_TO_RIGHT = 0,
	RIGHT_TO_LEFT = 1,
};

struct ELoadLevel
{
	const int8 QUICK_LOAD_FIRST = 10;
	const int8 QUICK_LOAD = 20;
	const int8 LOAD = 40;
	const int8 PRE_SLOW_LOAD = 50;

	TArray<int> Level{ QUICK_LOAD_FIRST, QUICK_LOAD, LOAD, PRE_SLOW_LOAD };
	const int TotalLevels = Level.Num() - 1;

	void reset() { index = 0; }
	int8 Value() { return Level[index]; }
	int8 operator++(int) { return (index < TotalLevels) ? Level[index++] : Level[index]; }

private:
	int index = 0;
};

class FMangaImageCache
{
public:
	static FMangaImageCache* GetInstance();
	FMangaImageCache() = default;

	const FString GetFile(int index);
	const EReadMode& GetReadMode();
	const EShowDirection& GetShowDirection();
	TArray<FString>& GetFileNames();
	TSharedPtr<FSlateBrush> GetBrush(int index);
	bool GetImageRawData(FString InFileNames, TArray<uint8>& OutData);

	const int& GetCurrentImageIndex();
	void NextPage();
	void LastPage();

	bool IsDirty();
	void AlreadyUpdate();
	void OpenFolder();
	void OpenFolder(const FString& Path);
	void ClearAllFiles();

	// 检查是否刚打开新文件夹
	bool IsFolderChanged() const { return bFolderChanged; }
	void ClearFolderChangedFlag() { bFolderChanged = false; }
	void LoadAllImage();
	void SortFileNames();
	void SwitchReadMode(EReadMode&& readMode);
	void SwitchShowDirection(EShowDirection&& InShowDirection);
	void ChangePageTo(int page);
	int PictrueIndexToPage(int Index);

private:
	void LoadImageAtIndex(int index);
	void LoadImageInStages();
	void StopLoading();
	void EnsureBufferLoaded();

public:
	// 窗口管理：设置可见窗口大小（当前页前后各多少页）
	void SetWindowSize(int32 InWindowSize) { WindowSize = InWindowSize; }
	int32 GetWindowSize() const { return WindowSize; }

	// 移动窗口中心到指定页，加载窗口内未加载的图片
	void MoveWindowTo(int32 CenterPage);

	// 获取窗口范围内未加载的图片索引
	TArray<int32> GetUnloadedIndicesInWindow(int32 CenterPage) const;

	// 检查指定索引的图片是否已加载
	bool IsImageLoaded(int32 Index) const;

	// 获取总页数（根据阅读模式计算）
	int32 GetTotalPageCount() const;

public:
	int32 WindowWidth = 1280;
	int32 WindowHeight = 720;

	const int DefaultPage = 0;

private:
	bool isDirty = false;
	bool bFolderChanged = false;  // 标记是否刚打开新文件夹
	int CurrentImage = 0;
	int Page = DefaultPage;
	TAtomic<int> LoadImageIndex = 0;
	TAtomic<bool> IsLoading = false;
	ELoadLevel LoadLevel;

	TArray<FString> FileNames;
	EReadMode ReadMode = SINGLE_PAGE;
	EShowDirection ShowDirection = LEFT_TO_RIGHT;
	TArray<TSharedPtr<FSlateBrush>> Brushes;

	FCriticalSection DataLock;
	TMap<FString, TArray<uint8>> ImageRawDataMap;

	// 窗口管理
	int32 WindowSize = 2;  // 默认当前页前后各2页
	int32 WindowCenter = 0;
	TSet<int32> LoadedIndices;  // 已加载的图片索引集合
	FCriticalSection LoadedIndicesLock;

public:
	// 单张图片加载完成的委托（图片索引）
	FOnImageLoaded OnImageLoaded;
};
