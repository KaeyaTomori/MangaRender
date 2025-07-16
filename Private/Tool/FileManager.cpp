#include "FileManager.h"

#include <shobjidl_core.h>

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFileManager.h"


static IDesktopPlatform* platformModule;

FString FileManager::PickFolder()
{
	FString FolderPath;
	if (platformModule)
	{
		const bool bSuccess = platformModule->OpenDirectoryDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			TEXT("选择文件夹"),
			TEXT(""),  // 默认路径
			FolderPath
		);
        
		// if (bSuccess && !FolderPath.IsEmpty())
		// {
		// 	UE_LOG(LogTemp, Log, TEXT("选择的文件夹: %s"), *FolderPath);
		// }
	}
	return FolderPath;
}

bool FileManager::GetImageData(const FString& ImagePath, TArray<uint8>& ImgData, int32& Width, int32& Height)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	// 加载文件数据
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *ImagePath);
		return false;
	}
	
	// 检测图像格式
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
	if (ImageFormat == EImageFormat::Invalid)
	{
		UE_LOG(LogTemp, Error, TEXT("Unsupported image format: %s"), *ImagePath);
		return false;
	}
	// 创建图像包装器
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create image wrapper"));
		return false;
	}

	// 仅解析头部信息获取尺寸
	if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse image header"));
		return false;
	}
	Width = ImageWrapper->GetWidth();
	Height = ImageWrapper->GetHeight();
	
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, ImageWrapper->GetBitDepth(), ImgData))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to GetRaw"));
		return false;
	}
	return true;
}

void FileManager::Init()
{
	
	platformModule = FDesktopPlatformModule::Get();
}

bool FileManager::GetAllFileInFolder(FString Path, TArray<FString>& AllFileNames)
{
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	platformFile.FindFiles(AllFileNames, *Path, nullptr);

	return true;
}