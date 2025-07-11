#include "SFileButton.h"

#include <Windows.h>
#include <minwindef.h>
#include <shobjidl_core.h>

#include "FileManager.h"

	// FPlatformMisc::OsExecute(L"open", L"D:\\acg");

FReply SFileButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	AsyncTask(ENamedThreads::GameThread, []()
	{
		// FString path = FileManager::GetInstance()->PickFolder();
		// UE_LOG(LogTemp, Warning, TEXT(":%s"), *path);
		// FileManager::GetInstance()->OpenAllFileInFolder(path);
	});
	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
}

// 	// OPENFILENAME ofn = {0};
	// 	// TCHAR szFile[MAX_PATH] = {0};
	// 	//
	// 	// ofn.lStructSize = sizeof(ofn);
	// 	// ofn.lpstrFile = szFile; // 存储路径的缓冲区
	// 	// ofn.nMaxFile = MAX_PATH;
	// 	// ofn.lpstrFilter = L"文本文件(*.jpg)\0*.jpg\0所有文件(*.*)\0*.*\0"; // 文件类型过滤
	// 	// ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; // 路径和文件必须存在
	// 	// ofn.lpTemplateName = L"default";
	// 	//
	// 	// // if (GetOpenFileName(&ofn)) {
	// 	// // 	UE_LOG(LogTemp, Warning, TEXT(": %s"), szFile);
	// 	// // }
	// 	
	// 	// if (GetSaveFileName(&ofn))
	// 	// {
	// 	// 	UE_LOG(LogTemp, Warning, TEXT(": %s"), szFile);
	// 	// }
	// 	
