#include "FileManager.h"

#include <shobjidl_core.h>

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFileManager.h"

static TSharedPtr<FileManager> Instance;
static IDesktopPlatform* platformModule;
TSharedPtr<FSlateDynamicImageBrush> img;

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

TSharedPtr<FileManager> FileManager::GetInstance()
{
	if (Instance == nullptr)
	{
		return Instance = MakeShareable(new FileManager());
	}
	return Instance;
}

bool GetImage(const FString& ImagePath, TArray<uint8>& ImgData, int32& Width, int32& Height)
{
	// 加载文件数据
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *ImagePath);
		return false;
	}

	// 获取图像包装模块
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
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

	// 仅解析头部信息获取尺寸（不加载完整图像）
	if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse image header"));
		return false;
	}
	Width = ImageWrapper->GetWidth();
	Height = ImageWrapper->GetHeight();
	
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, ImgData))
	{
		return false;
	}
	return true;
}
static int c = 0;
TSharedPtr<FSlateBrush> FileManager::OpenAllFileInFolder(FString Path)
{
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	TArray<FString> FileNames;
	platformFile.FindFiles(FileNames, *Path, nullptr);

	int now = 0;
	for (auto FileName : FileNames)
	{
		if (now < c)
		{
			++now;
			continue;
		}

		// FSlateApplicationBase::Get().GetRenderer()->GenerateDynamicImageResource(InTextureName, (uint32)InImageSize.X, (uint32)InImageSize.Y, InImageData))
		++c;
		UE_LOG(LogTemp, Display, TEXT("%s"), *FileName);
		TArray<uint8> ImgData;
		int32 Width = 0, Height = 0;
		GetImage(FileName, ImgData, Width, Height);
		FVector2D size(Width, Height);
		img = FSlateDynamicImageBrush::CreateWithImageData("h1Texture", size, ImgData);
		auto x = img.Get();
		// img = MakeShareable(new FSlateImageBrush(FileName, GetImageDimensions(FileName)));
		return img;
	}
	return nullptr;
}

FileManager::FileManager()
{
	platformModule = FDesktopPlatformModule::Get();
}


// UTexture2D* FileManager::LoadTexture2DFromFile(const FString& FilePath)
// {
//     // 加载文件到字节数组
//     TArray<uint8> FileData;
//     if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
//     {
//         UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *FilePath);
//         return nullptr;
//     }
//
//     // 获取图像包装模块
//     IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
//
//     // 根据文件扩展名检测图像格式
//     EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
//     if (ImageFormat == EImageFormat::Invalid)
//     {
//         UE_LOG(LogTemp, Error, TEXT("Unsupported image format: %s"), *FilePath);
//         return nullptr;
//     }
//
//     // 创建图像包装器
//     TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
//     if (!ImageWrapper.IsValid())
//     {
//         UE_LOG(LogTemp, Error, TEXT("Failed to create image wrapper for: %s"), *FilePath);
//         return nullptr;
//     }
//
//     // 解压缩图像数据
//     if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
//     {
//         UE_LOG(LogTemp, Error, TEXT("Failed to parse image data: %s"), *FilePath);
//         return nullptr;
//     }
//
//     // 获取原始RGBA数据
//     TArray<uint8> RawData;
//     if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData))
//     {
//         UE_LOG(LogTemp, Error, TEXT("Failed to get raw image data: %s"), *FilePath);
//         return nullptr;
//     }
//
//     // 创建纹理
//     UTexture2D* Texture = UTexture2D::CreateTransient(
//         ImageWrapper->GetWidth(),
//         ImageWrapper->GetHeight(),
//         PF_R8G8B8A8
//     );
//     if (!Texture)
//     {
//         UE_LOG(LogTemp, Error, TEXT("Failed to create texture: %s"), *FilePath);
//         return nullptr;
//     }
//
//     // 填充纹理数据
//     void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
//     FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
//     Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
//     
//     // // 更新纹理资源
//     // Texture->UpdateResource();
//     
//     return Texture;
// }