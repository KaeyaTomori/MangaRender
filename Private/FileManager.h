#pragma once

#include "DesktopPlatformModule.h"

class FileManager
{
public:
	static TSharedPtr<FileManager> GetInstance();

	FString PickFolder();
	static TSharedPtr<FSlateBrush> OpenAllFileInFolder(FString Path);

	TSharedPtr<FSlateBrush> testImg;
	
private:
	FileManager();
	// UTexture2D* LoadTexture2DFromFile(const FString& FilePath);
};
