// Copyright Epic Games, Inc. All Rights Reserved.

#include "MangaRender.h"

#include "MangaImageCache.h"
#include "RequiredProgramMainCPPInclude.h"
#include "SMainWidget.h"
#include "StandaloneRenderer.h"
#include "Utils/FImageDecoder.h"

DEFINE_LOG_CATEGORY_STATIC(LogMangaRender, Log, All);

IMPLEMENT_APPLICATION(MangaRender, "MangaRender");


int WINAPI WinMain( _In_ HINSTANCE hInInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int nCmdShow )
{
	GEngineLoop.PreInit(GetCommandLineW());

	FSlateApplication::InitializeAsStandaloneApplication(GetStandardStandaloneRenderer());
	FSlateApplication::InitHighDPI(true);

	auto ImageCache = FMangaImageCache::GetInstance();
	
	TSharedRef<SMainWidget> mainWidget = SNew(SMainWidget);
	TSharedRef<SWindow> Window = SNew(SWindow)
	.ClientSize(FVector2D(ImageCache->WindowWidth, ImageCache->WindowHeight))
	[
		mainWidget
	];
	
	FSlateApplication::Get().AddWindow(Window);

	FTaskGraphInterface::Startup(FPlatformMisc::NumberOfWorkerThreadsToSpawn());
	FTaskGraphInterface::Get().AttachToThread(ENamedThreads::GameThread);
	
	FImageDecoder::Init();

	while (!IsEngineExitRequested())
	{
		BeginExitIfRequested();
		
		FSlateApplication::Get().PumpMessages();
		FSlateApplication::Get().Tick();

		// do Task
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		
		FPlatformProcess::Sleep(0.0166f);
		// FPlatformProcess::Sleep(0.5f);
	}
	
	FSlateApplication::Shutdown();

	GEngineLoop.AppPreExit();
	GEngineLoop.AppExit();

	return 0;
}
