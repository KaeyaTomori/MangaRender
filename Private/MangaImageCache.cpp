#include "MangaImageCache.h"

#include "Utils/FImageDecoder.h"

static FMangaImageCache* gDataManager;

FMangaImageCache* FMangaImageCache::GetInstance()
{
	if (gDataManager == nullptr)
	{
		gDataManager = new FMangaImageCache();
	}
	return gDataManager;
}

const FString FMangaImageCache::GetFile(int index)
{
	return FileNames.IsValidIndex(index) ? FileNames[index] : "";
}

void FMangaImageCache::ClearAllFiles()
{
	StopLoading();
	FScopeLock Lock(&DataLock);
	FileNames.Empty();
	Brushes.Empty();
	ImageRawDataMap.Empty();
	LoadImageIndex = 0;
	CurrentImage = 0;
	Page = DefaultPage;
	isDirty = true;
	LoadLevel.reset();

	// 清空已加载索引记录
	{
		FScopeLock IndicesLock(&LoadedIndicesLock);
		LoadedIndices.Empty();
	}
	WindowCenter = 0;
	bFolderChanged = true;
}

TArray<FString>& FMangaImageCache::GetFileNames()
{
	return FileNames;
}

void FMangaImageCache::LoadAllImage()
{
	if (IsLoading.Load()) return;
	LoadImageIndex = 0;
	IsLoading.Store(true);
	LoadLevel.reset();
	LoadImageInStages();
}

TSharedPtr<FSlateBrush> FMangaImageCache::GetBrush(int index)
{
	FScopeLock Lock(&DataLock);
	if (!Brushes.IsValidIndex(index))
	{
		UE_LOG(LogTemp, Display, TEXT("IsInValidIndex index = %d"), index);
	}
	return Brushes.IsValidIndex(index) ? Brushes[index] : nullptr;
}

bool FMangaImageCache::GetImageRawData(FString InFileNames, TArray<uint8>& OutData)
{
	if (ImageRawDataMap.Contains(InFileNames))
	{
		OutData = ImageRawDataMap[InFileNames];
		return true;
	}
	return false;
}

const int& FMangaImageCache::GetCurrentImageIndex()
{
	return CurrentImage;
}

void FMangaImageCache::NextPage()
{
	if (Page < (FileNames.Num() + ReadMode - 1) / ReadMode - 1)
	{
		ChangePageTo(Page + 1);
	}
}

void FMangaImageCache::LastPage()
{
	if (Page > 0)
	{
		ChangePageTo(Page - 1);
	}
}

bool FMangaImageCache::IsDirty()
{
	return isDirty;
}

void FMangaImageCache::AlreadyUpdate()
{
	isDirty = false;
}

void FMangaImageCache::OpenFolder()
{
	FString path = FImageDecoder::PickFolder();
	OpenFolder(path);
}

void FMangaImageCache::OpenFolder(const FString& Path)
{
	ClearAllFiles();
	FImageDecoder::GetAllFileInFolder(Path, FileNames);
	SortFileNames();
	{
		FScopeLock Lock(&DataLock);
		Brushes.SetNumZeroed(FileNames.Num());
	}
	LoadImageInStages();
}

const EReadMode& FMangaImageCache::GetReadMode()
{
	return ReadMode;
}

const EShowDirection& FMangaImageCache::GetShowDirection()
{
	return ShowDirection;
}

void FMangaImageCache::SortFileNames()
{
	FileNames.Sort(TLess<FString>());
}

void FMangaImageCache::SwitchReadMode(EReadMode&& readMode)
{
	ReadMode = readMode;
	if (ReadMode == SINGLE_PAGE)
	{
		SwitchShowDirection(LEFT_TO_RIGHT);
	} else
	{
		SwitchShowDirection(RIGHT_TO_LEFT);
	}
	isDirty = true;
	Page = CurrentImage / readMode;
	CurrentImage = Page * ReadMode;
}

void FMangaImageCache::SwitchShowDirection(EShowDirection&& InShowDirection)
{
	if (ReadMode == SINGLE_PAGE)
	{
		ShowDirection = EShowDirection::LEFT_TO_RIGHT;
		return;
	}
	ShowDirection = InShowDirection;
	isDirty = true;
}

void FMangaImageCache::ChangePageTo(int page)
{
	Page = page;
	CurrentImage = Page * ReadMode;
	isDirty = true;

	EnsureBufferLoaded();
}

int FMangaImageCache::PictrueIndexToPage(int Index)
{
	return Index / ReadMode;
}

void FMangaImageCache::EnsureBufferLoaded()
{
	if (IsLoading.Load()) return;

	int loadedCount = LoadImageIndex.Load();
	int remainingLoaded = loadedCount - CurrentImage;

	// 如果剩余已加载页数不足，触发加载
	int threshold = LoadLevel.Value();
	if (remainingLoaded < threshold / 4 && loadedCount < FileNames.Num())
	{
		LoadImageInStages();
	}
}

void FMangaImageCache::StopLoading()
{
	IsLoading.Store(false);
}

void FMangaImageCache::LoadImageInStages()
{
	if (IsLoading.Load()) return;
	IsLoading.Store(true);

	int count = LoadLevel++;
	int startIndex = LoadImageIndex;
	int endIndex = FMath::Min(startIndex + count, FileNames.Num());

	Async(EAsyncExecution::ThreadPool, [this, startIndex, endIndex]() {
		for (int i = startIndex; i < endIndex; ++i)
		{
			if (!IsLoading.Load()) return;
			LoadImageAtIndex(i);
		}
		LoadImageIndex = endIndex;
		IsLoading.Store(false);
	});
}

void FMangaImageCache::MoveWindowTo(int32 CenterPage)
{
	int32 TotalPages = GetTotalPageCount();
	if (TotalPages <= 0) return;

	int32 NewCenter = FMath::Clamp(CenterPage, 0, TotalPages - 1);
	if (NewCenter == WindowCenter) return;

	WindowCenter = NewCenter;

	// 获取窗口内未加载的图片索引
	TArray<int32> UnloadedIndices = GetUnloadedIndicesInWindow(WindowCenter);
	if (UnloadedIndices.Num() == 0) return;

	// 使用类似 LoadImageInStages 的分批加载
	Async(EAsyncExecution::ThreadPool, [this, UnloadedIndices]() {
		int32 BatchSize = LoadLevel.Value();
		int32 ProcessedCount = 0;

		for (int32 ImageIndex : UnloadedIndices)
		{
			LoadImageAtIndex(ImageIndex);

			ProcessedCount++;

			// 每批处理完后稍微停顿，避免阻塞
			if (ProcessedCount % BatchSize == 0)
			{
				FPlatformProcess::Sleep(0.001f);
			}
		}
	});
}

TArray<int32> FMangaImageCache::GetUnloadedIndicesInWindow(int32 CenterPage) const
{
	TArray<int32> Result;

	int32 TotalPages = GetTotalPageCount();
	if (TotalPages <= 0) return Result;

	// 计算窗口范围（页号转图片索引）
	int32 PageStart = FMath::Max(0, CenterPage - WindowSize);
	int32 PageEnd = FMath::Min(TotalPages - 1, CenterPage + WindowSize);

	int32 ImageStart = PageStart * static_cast<int32>(ReadMode);
	int32 ImageEnd = FMath::Min((PageEnd + 1) * static_cast<int32>(ReadMode), FileNames.Num());

	FScopeLock Lock(const_cast<FCriticalSection*>(&LoadedIndicesLock));

	for (int32 i = ImageStart; i < ImageEnd; ++i)
	{
		if (!LoadedIndices.Contains(i))
		{
			Result.Add(i);
		}
	}

	return Result;
}

bool FMangaImageCache::IsImageLoaded(int32 Index) const
{
	if (!FileNames.IsValidIndex(Index)) return false;

	FScopeLock Lock(const_cast<FCriticalSection*>(&LoadedIndicesLock));
	return LoadedIndices.Contains(Index);
}

int32 FMangaImageCache::GetTotalPageCount() const
{
	if (FileNames.Num() == 0) return 0;
	return (FileNames.Num() + static_cast<int32>(ReadMode) - 1) / static_cast<int32>(ReadMode);
}

void FMangaImageCache::LoadImageAtIndex(int index)
{
	if (!FileNames.IsValidIndex(index)) return;
	
	if (LoadedIndices.Contains(index)) return;
	
	// 检查是否已加载
	{
		FScopeLock Lock(&LoadedIndicesLock);
		if (LoadedIndices.Contains(index)) return;
		// UE_LOG(LogTemp, Display, TEXT("LoadImageAtIndex index = %d"), index);
		// 标记为已加载
		LoadedIndices.Add(index);
	}

	const FString& fileName = FileNames[index];
	TArray<uint8> fileData;
	int32 imageWidth, imageHeight;
	if (!FImageDecoder::GetImageData(fileName, fileData, imageWidth, imageHeight)) return;

	AsyncTask(ENamedThreads::GameThread, [this, fileName, fileData = MoveTemp(fileData), imageWidth, imageHeight, index]() mutable {
		FScopeLock Lock(&DataLock);
		
		ImageRawDataMap.Add(fileName, MoveTemp(fileData));
		TSharedPtr<FSlateBrush> brush = FSlateDynamicImageBrush::CreateWithImageData(*fileName, FVector2D(imageWidth, imageHeight), ImageRawDataMap[fileName]);
		Brushes[index] = brush;

		// 广播加载完成
		OnImageLoaded.Broadcast(index);

		isDirty = true;
	});
}
