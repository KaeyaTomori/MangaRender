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

	// 	UE_LOG(LogTemp, Display, TEXT("Getting brush index %d, LoadImageIndex = %d"), index, LoadImageIndex.Load());
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
	if (Page < (FileNames.Num() - 1 + ReadMode - 1) / ReadMode)
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

void FMangaImageCache::LoadImageAtIndex(int index)
{
	if (!FileNames.IsValidIndex(index)) return;

	const FString& fileName = FileNames[index];
	TArray<uint8> fileData;
	int32 imageWidth, imageHeight;
	if (!FImageDecoder::GetImageData(fileName, fileData, imageWidth, imageHeight)) return;

	AsyncTask(ENamedThreads::GameThread, [this, fileName, fileData = MoveTemp(fileData), imageWidth, imageHeight, index]() mutable {
		FScopeLock Lock(&DataLock);
		ImageRawDataMap.Add(fileName, MoveTemp(fileData));
		TSharedPtr<FSlateBrush> brush = FSlateDynamicImageBrush::CreateWithImageData(*fileName, FVector2D(imageWidth, imageHeight), ImageRawDataMap[fileName]);
		Brushes[index] = brush;
		isDirty = true;
	});
}
