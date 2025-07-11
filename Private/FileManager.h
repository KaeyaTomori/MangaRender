#pragma once

#include "IImageWrapperModule.h"
#include "DesktopPlatformModule.h"

class FileManager
{
public:
	static FileManager* getInstance();
	
	static FString PickFolder();
	static bool GetAllFileInFolder(FString Path, TArray<FString>& AllFileNames);
	static bool GetImageData(const FString& ImagePath, TArray<uint8>& ImgData, int32& Width, int32& Height);
	static void Init();

	FString OpenPath;
	TArray<FString> FileNames;
	bool isDirty = false;
	
private:
	// 图像包装模块
};
