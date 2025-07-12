#include "SFileButton.h"

#include <Windows.h>
#include <minwindef.h>
#include <shobjidl_core.h>

#include "DataManager.h"
#include "Tool/FileManager.h"

	// FPlatformMisc::OsExecute(L"open", L"D:\\acg");

FReply SFileButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	AsyncTask(ENamedThreads::GameThread, []()
	{
		FString path = FileManager::PickFolder();
		auto data = DataManager::getInstance();
		data->ClearAllFiles();
		FileManager::GetAllFileInFolder(path, data->GetFileNamesAndDirty());
		data->LoadAllImage();
		// Async(EAsyncExecution::Thread, []()
		// {
			// auto data = DataManager::getInstance();
			// data->LoadAllImage();
		// });
	});
	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
}