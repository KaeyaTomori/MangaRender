#pragma once

class DataManager
{
public:
	static DataManager* getInstance();

	const FString GetFile(int index);
	void ClearAllFiles();
	TArray<FString>& GetFileNamesAndDirty();
	void LoadAllImage();
	TSharedPtr<FSlateBrush> GetBrush(int index);
	
	FString OpenPath;
	TArray<FString> FileNames;
	bool isDirty = false;

	int32 WindowWidth = 1920;
	int32 WindowHeight = 1280;
	
private:
	TArray<TSharedPtr<FSlateBrush>> Brushes;
};
