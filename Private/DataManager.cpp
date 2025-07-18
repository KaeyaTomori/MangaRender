#include "DataManager.h"

#include "Tool/FileManager.h"

static DataManager* gDataManager;

DataManager* DataManager::getInstance()
{
	if (gDataManager == nullptr)
	{
		gDataManager = new DataManager();
	}
	return gDataManager;
}

const FString DataManager::GetFile(int index)
{
	return FileNames.IsValidIndex(index) ? FileNames[index] : "";
}

void DataManager::ClearAllFiles()
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

TArray<FString>& DataManager::GetFileNames()
{
	return FileNames;
}

void DataManager::LoadAllImage()
{
	if (IsLoading.Load()) return;
	LoadImageIndex = 0;
	IsLoading.Store(true);
	LoadLevel.reset();
	LoadImageInStages();

}

void DataManager::LoadImageFirst(int count)
{
	// UE_LOG(LogTemp, Display, TEXT("LoadingImage count = %d"), count);
	int endLoadImageIndex = LoadImageIndex + count;
	IsLoading.Store(true);
	for (; LoadImageIndex < FileNames.Num() && LoadImageIndex < endLoadImageIndex; ++LoadImageIndex)
	{
		LoadImage(FileNames[LoadImageIndex]);
	}
	IsLoading.Store(false);
}

void DataManager::LoadImageByCount(int count)
{
	UE_LOG(LogTemp, Display, TEXT("LoadingImage count = %d"), count);
	int endLoadImageIndex = LoadImageIndex + count;
	IsLoading.Store(true);
	for (; LoadImageIndex < FileNames.Num() && LoadImageIndex < endLoadImageIndex; ++LoadImageIndex)
	{
		LoadImage(FileNames[LoadImageIndex]);
	}
	IsLoading.Store(false);
}

TSharedPtr<FSlateBrush> DataManager::GetBrush(int index)
{
	FScopeLock Lock(&DataLock);
	if (!Brushes.IsValidIndex(index))
	{
		UE_LOG(LogTemp, Display, TEXT("IsInValidIndex index = %d"), index);
	}
	return Brushes.IsValidIndex(index) ? Brushes[index] : PlaceholderBrush;
	
	// 	UE_LOG(LogTemp, Display, TEXT("Getting brush index %d, LoadImageIndex = %d"), index, LoadImageIndex.Load());
}

const int& DataManager::GetCurrentImageIndex()
{
	return CurrentImage;
}

void DataManager::NextPage()
{
	if (Page < (FileNames.Num() + ReadMode - 1) / ReadMode)
	{
		ChangePageTo(Page + 1);
	}
}

void DataManager::LastPage()
{
	if (Page > 1)
	{
		ChangePageTo(Page - 1);
	}
}

bool DataManager::IsDirty()
{
	return isDirty;
}

void DataManager::AlreadyUpdate()
{
	isDirty = false;
}

void DataManager::OpenFolder()
{
	FString path = FileManager::PickFolder();
	OpenFolder(path);
	
	// LoadImageFirst(2);
	// ImageLoader = MakeShareable(new FImageLoader(FileNames, LoadImageIndex, FileNames.Num() - LoadImageIndex));
}

void DataManager::OpenFolder(const FString& Path)
{
	ClearAllFiles();
	FileManager::GetAllFileInFolder(Path, FileNames);
	{
		FScopeLock Lock(&DataLock);
		Brushes.SetNumZeroed(FileNames.Num());
	}
	LoadAllImage();
}

const EReadMode& DataManager::GetReadMode()
{
	return ReadMode;
}

const EShowDirection& DataManager::GetShowDirection()
{
	return ShowDirection;
}

void DataManager::SortFileNames()
{
	FileNames.Sort(TLess<FString>());
}

void DataManager::SwitchReadMode(EReadMode&& readMode)
{
	ReadMode = readMode;
	if (ReadMode == SINGLE_PAGE)
	{
		SwitchShowDirection(LEFT_TO_RIGHT);
	}
	isDirty = true;
	Page = CurrentImage / readMode + 1;
	CurrentImage = (Page - 1) * ReadMode;
}

void DataManager::SwitchShowDirection(EShowDirection&& InShowDirection)
{
	if (ReadMode == SINGLE_PAGE)
	{
		ShowDirection = EShowDirection::LEFT_TO_RIGHT;
		return;
	}
	ShowDirection = InShowDirection;
	isDirty = true;
}

void DataManager::ChangePageTo(int page)
{
	Page = page;
	CurrentImage = (Page - 1) * ReadMode;
	isDirty = true;
	

	// if (!IsLoading && CurrentImage > LoadImageIndex - LoadLevel.Value() / 2)
	// {
	// 	Async(EAsyncExecution::ThreadPool, [this]()
	// 	{
	// 		LoadImageByCount(LoadLevel++);
	// 	});
	// }
}

void DataManager::LoadImage(FString fileName)
{
	TArray<uint8> fileData;
	int32 imageWidth, imageHeight;
	FileManager::GetImageData(fileName, fileData, imageWidth, imageHeight);
	ImageRawDataMap.Add(fileName, MoveTemp(fileData));
	{
		Brushes.Add(FSlateDynamicImageBrush::CreateWithImageData(*fileName, FVector2D(imageWidth, imageHeight), ImageRawDataMap[fileName]));
	}
}

DataManager::DataManager()
{
	PlaceholderBrush = MakeShared<FSlateImageBrush>(FName("/Game/UI/placeholder"), FVector2D(512, 512));
}

void DataManager::StopLoading()
{
	IsLoading.Store(false);
}

void DataManager::LoadImageInStages()
{
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
		if (LoadImageIndex < FileNames.Num())
		{
			LoadImageInStages();
		}
		else
		{
			IsLoading.Store(false);
		}
	});
}

void DataManager::LoadImageAtIndex(int index)
{
	if (!FileNames.IsValidIndex(index)) return;

	const FString& fileName = FileNames[index];
	TArray<uint8> fileData;
	int32 imageWidth, imageHeight;
	if (!FileManager::GetImageData(fileName, fileData, imageWidth, imageHeight)) return;

	AsyncTask(ENamedThreads::GameThread, [this, fileName, fileData = MoveTemp(fileData), imageWidth, imageHeight, index]() mutable {
		FScopeLock Lock(&DataLock);
		ImageRawDataMap.Add(fileName, MoveTemp(fileData));
		TSharedPtr<FSlateBrush> brush = FSlateDynamicImageBrush::CreateWithImageData(*fileName, FVector2D(imageWidth, imageHeight), ImageRawDataMap[fileName]);
		Brushes[index] = brush;
		isDirty = true;
	});
}
