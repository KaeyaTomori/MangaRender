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
	if (index >= FileNames.Num())
	{
		return "";
	}
	return FileNames[index];
}

void DataManager::ClearAllFiles()
{
	FileNames.Empty();
	Brushes.Empty();
}

TArray<FString>& DataManager::GetFileNamesAndDirty()
{
	isDirty = true;
	return FileNames;
}

void DataManager::LoadAllImage()
{
	for (auto fileName : FileNames)
	{
		TArray<uint8> fileData;
		int32 imageWidth, imageHeight;
		FileManager::GetImageData(fileName, fileData, imageWidth, imageHeight);
		Brushes.Add(FSlateDynamicImageBrush::CreateWithImageData(*fileName, FVector2D(imageWidth, imageHeight), fileData));
	}
}

TSharedPtr<FSlateBrush> DataManager::GetBrush(int index)
{
	if (index < Brushes.Num())
	{
		return Brushes[index];
	}
	return nullptr;
}
