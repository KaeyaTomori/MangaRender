// Copyright Epic Games, Inc. All Rights Reserved.

#include "MangaRender.h"

#include "FileManager.h"
#include "RequiredProgramMainCPPInclude.h"
#include "SMainWidget.h"
#include "StandaloneRenderer.h"

DEFINE_LOG_CATEGORY_STATIC(LogMangaRender, Log, All);

IMPLEMENT_APPLICATION(MangaRender, "MangaRender");


int WINAPI WinMain( _In_ HINSTANCE hInInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int nCmdShow )
{
	GEngineLoop.PreInit(GetCommandLineW());

	FSlateApplication::InitializeAsStandaloneApplication(GetStandardStandaloneRenderer());
	FSlateApplication::InitHighDPI(true);

	TSharedRef<SMainWidget> mainWidget = SNew(SMainWidget);
	TSharedRef<SWindow> Window = SNew(SWindow)
	.ClientSize(FVector2D(1080.f, 720.f))
	[
		mainWidget
	];
	
	FSlateApplication::Get().AddWindow(Window);

	FTaskGraphInterface::Startup(FPlatformMisc::NumberOfWorkerThreadsToSpawn());
	FTaskGraphInterface::Get().AttachToThread(ENamedThreads::GameThread);
	
	FileManager::Init();

	while (!IsEngineExitRequested())
	{
		BeginExitIfRequested();
		
		FSlateApplication::Get().PumpMessages();
		FSlateApplication::Get().Tick();
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);

		mainWidget.Get().update();
		FPlatformProcess::Sleep(0.0166f);
	}
	
	FSlateApplication::Shutdown();

	GEngineLoop.AppPreExit();
	GEngineLoop.AppExit();

	return 0;
}
