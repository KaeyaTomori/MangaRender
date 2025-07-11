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
		FString path = FileManager::PickFolder();
		// UE_LOG(LogTemp, Warning, TEXT(":%s"), *path);
		auto manager = FileManager::getInstance();
		FileManager::GetAllFileInFolder(path, manager->FileNames);
		manager->isDirty = true;
		
	});
	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
}