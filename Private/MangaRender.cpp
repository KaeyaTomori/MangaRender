// Copyright Epic Games, Inc. All Rights Reserved.

#include "MangaRender.h"

#include "RequiredProgramMainCPPInclude.h"
#include "StandaloneRenderer.h"
#include "Fonts/FontMeasure.h"

DEFINE_LOG_CATEGORY_STATIC(LogMangaRender, Log, All);

IMPLEMENT_APPLICATION(MangaRender, "MangaRender");


int WINAPI WinMain( _In_ HINSTANCE hInInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int nCmdShow )
{
	GEngineLoop.PreInit(GetCommandLineW());

	FSlateApplication::InitializeAsStandaloneApplication(GetStandardStandaloneRenderer());
	

	FSlateApplication::InitHighDPI(true);

	TSharedRef<SWidget> myWidget = SNew(SMyWidget);
	TSharedRef<SWindow> Window = SNew(SWindow)
	.ClientSize(FVector2D(200.f, 200.f))
	[
		myWidget
	];
	
	FSlateApplication::Get().AddWindow(Window);
	
	while (!IsEngineExitRequested())
	{
		BeginExitIfRequested();
		
		FSlateApplication::Get().PumpMessages();
		FSlateApplication::Get().Tick();
	}
	
	FSlateApplication::Shutdown();

	GEngineLoop.AppPreExit();
	GEngineLoop.AppExit();

	return 0;
}

SMyWidget::SMyWidget()
{
}

SMyWidget::~SMyWidget()
{
}

void SMyWidget::Construct(const FArguments& InArgs)
{
	TSharedRef<FSlateStyleSet> StyleSet = MakeShareable(new FSlateStyleSet("MyWidgetStyle"));
	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
	
	static const FButtonStyle CloseButtonStyle = FButtonStyle()
	.SetNormal (FSlateImageBrush(StyleSet->RootToContentDir("Common/Window/WindowButton_Close_Normal", L".png"), FVector2D(44.0f, 18.0f)))
	.SetHovered(FSlateImageBrush(StyleSet->RootToContentDir("Common/Window/WindowButton_Close_Hovered", L".png"), FVector2D(44.0f, 18.0f)))
	.SetPressed(FSlateImageBrush(StyleSet->RootToContentDir("Common/Window/WindowButton_Close_Pressed", L".png"), FVector2D(44.0f, 18.0f)));
	
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(SBox)
			.WidthOverride(FOptionalSize(44.f))
			.HeightOverride(FOptionalSize(18.f))
			[
				SNew(SButton)
				.ButtonStyle(&CloseButtonStyle)
				.OnPressed(FSimpleDelegate::CreateLambda([]()
				{
					RequestEngineExit(FString("User Exit"));
				}))
			]
		]
	];
}

int32 SMyWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 绘制Box背景
	const FSlateBrush* Bursh = FAppStyle::Get().GetBrush("WhiteBrush");
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Bursh, ESlateDrawEffect::None, BackgroundColor);
	++LayerId;
	
	// 绘制文字
	FText ShowText = FText::FromString("Hello Slate");
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Bold",30);
	
	const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	FVector2D TextSize = FontMeasureService.Get().Measure(ShowText, FontInfo);
	FVector2D AreaSize = AllottedGeometry.GetLocalSize();
	FVector2D TextPosition = (AreaSize - TextSize) / 2.f;
	FSlateLayoutTransform TextLayoutTransform = FSlateLayoutTransform(FVector2D(TextPosition.X, TextPosition.Y));
	FPaintGeometry TextGeometry = AllottedGeometry.ToPaintGeometry();
	TextGeometry.AppendTransform(TextLayoutTransform);
	FSlateDrawElement::MakeText(OutDrawElements, LayerId, TextGeometry, ShowText, FontInfo);
	
	// 子部件绘制交给SCompoundWidget::OnPaint()
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SMyWidget::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	BackgroundColor = FLinearColor(0.6f, 0.5f, 0.9f);
}

void SMyWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	BackgroundColor = FLinearColor(0.1f, 0.5f, 0.9f);
}

FReply SMyWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		LastMouseDownPosition = MouseEvent.GetScreenSpacePosition();
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	
	return FReply::Handled();
}

FReply SMyWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SMyWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
		FVector2D CurrentWindowPosition = Window->GetPositionInScreen();
		FVector2D CurrentMousePosition = MouseEvent.GetScreenSpacePosition();
		FVector2D TargetPosition = CurrentWindowPosition - LastMouseDownPosition + CurrentMousePosition;
		Window->MoveWindowTo(TargetPosition);

		LastMouseDownPosition = CurrentMousePosition;
	}

	return FReply::Handled();
}

FVector2D SMyWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	FText ShowText = FText::FromString("Hello Slate");
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Bold",30);
	const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	FVector2D TextSize =FontMeasureService.Get().Measure(ShowText, FontInfo);

	return TextSize;
}
