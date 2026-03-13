#pragma once

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
	// int32 WindowWidth = 1920;
	// int32 WindowHeight = 1080;
	int32 WindowWidth = 1280;
	int32 WindowHeight = 720;

	const int DefaultPage = 0;

private:
	bool isDirty = false;
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
};
