#pragma once

class FileManager
{
public:
	static FString PickFolder();
	static bool GetAllFileInFolder(FString Path, TArray<FString>& AllFileNames);
	static bool GetImageData(const FString& ImagePath, TArray<uint8>& ImgData, int32& Width, int32& Height);
	static void Init();

private:
	// 图像包装模块
};
