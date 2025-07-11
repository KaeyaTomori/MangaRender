#include "FileManager.h"

#include <shobjidl_core.h>

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFileManager.h"


static IDesktopPlatform* platformModule;
static FileManager* gFileManager;

FileManager* FileManager::getInstance()
{
	if (gFileManager == nullptr)
	{
		gFileManager = new FileManager();
	}
	return gFileManager;
}

FString FileManager::PickFolder()
{
	// FPlatformMisc::CoInitialize();
	// IFileDialog* pfd;
	// PWSTR pszPath = NULL;
	// CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileDialog, (void**)&pfd);
	// pfd->SetOptions(FOS_PICKFOLDERS);  // 关键：设置为文件夹模式
	// if (SUCCEEDED(pfd->Show(NULL))) {
	// 	IShellItem* psi;
	// 	pfd->GetResult(&psi);
	// 	psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
	// 	// 使用pszPath
	// 	CoTaskMemFree(pszPath);
	// 	psi->Release();
	// }
	// pfd->Release();
	// FPlatformMisc::CoUninitialize();
	// return pszPath;
	FString FolderPath;
	if (platformModule)
	{
		const bool bSuccess = platformModule->OpenDirectoryDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			TEXT("选择文件夹"),
			TEXT(""),  // 默认路径
			FolderPath
		);
        
		if (bSuccess && !FolderPath.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("选择的文件夹: %s"), *FolderPath);
		}
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