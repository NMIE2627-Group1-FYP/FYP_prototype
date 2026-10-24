#include "CaveUserWidget.h"

#include "CaveUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "CreatureCharacter.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

UCaveUserWidget::UCaveUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectTitle = FText::FromString(TEXT("\u807D\u65E5 Hear Today, Gone Tomorrow"));
	ArtistStatement = INVTEXT(
		"We are four students working across hardware, sound, real-time graphics and storytelling. "
		"Together, we have created an installation that invites one person at a time to become a mole.\n\n"
		"We do not claim to know what a mole feels. Instead, we ask a larger question: if we slow down "
		"and listen, what else might we begin to notice?");
	SpeakerName = INVTEXT("Star-nosed mole");
	WalkHint = INVTEXT("Walk to hear the next line");
	Lines = {
		INVTEXT("You wake in the dark with an empty, aching stomach."),
		INVTEXT("The den's food store is bare."),
		INVTEXT("Behind you there is faint squeaking and slow breathing. The family depends on you."),
		INVTEXT("The floor hums under your feet. Find direction by hearing, and judge the ground by stepping."),
		INVTEXT("There is one purpose: find food and bring it home."),
	};
}

void UCaveUserWidget::SetOwnerCharacter(ACreatureCharacter* InCharacter)
{
	OwnerCharacter = InCharacter;
	if (SensitivitySlider && OwnerCharacter.IsValid())
	{
		SensitivitySlider->SetValue(OwnerCharacter->GetLookSensitivity());
	}
}

void UCaveUserWidget::SetIncludeStart(bool bInIncludeStart)
{
	bIncludeStart = bInIncludeStart;
	bCardOpen = bInIncludeStart;
	ApplyPanelVisibility();
}

bool UCaveUserWidget::IsCardOpen() const
{
	return bCardOpen;
}

bool UCaveUserWidget::IsNarrativeFinished() const
{
	return bNarrativeFinished;
}

void UCaveUserWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	Style = UCaveUIStyle::Load(this);
	if (WidgetTree)
	{
		ApplySharedStyle(WidgetTree->RootWidget);
	}
}

void UCaveUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!Style)
	{
		Style = UCaveUIStyle::Load(this);
	}
	if (WidgetTree)
	{
		ApplySharedStyle(WidgetTree->RootWidget);
	}

	if (SpeechStack)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SpeechStack->Slot))
		{
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.8f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(FVector2D::ZeroVector);
			CanvasSlot->SetSize(FVector2D(1000.0f, 200.0f));
		}
	}

	// Auto-wrap inside a centred overlay collapses to the widest word, so wrap at a fixed width.
	if (BodyText)
	{
		BodyText->SetAutoWrapText(false);
		BodyText->SetWrapTextAt(880.0f);
		BodyText->SetJustification(ETextJustify::Center);
	}

	if (StoryBody)
	{
		StoryBody->SetAutoWrapText(false);
		StoryBody->SetWrapTextAt(720.0f);
		FSlateFontInfo StatementFont = StoryBody->GetFont();
		StatementFont.Size = 18;
		StoryBody->SetFont(StatementFont);
	}

	if (TitleText)
	{
		TitleText->SetAutoWrapText(false);
		TitleText->SetWrapTextAt(760.0f);
	}

	if (HintText)
	{
		HintText->SetAutoWrapText(false);
	}

	if (UOverlay* CardOverlay = Cast<UOverlay>(Card); CardOverlay && WidgetTree && !CardScrim)
	{
		CardScrim = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("CardScrim"));
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(22.0f, 22.0f, 22.0f, 22.0f));
		Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Brush.TintColor = FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.62f));
		CardScrim->SetBrush(Brush);
		CardScrim->SetVisibility(ESlateVisibility::HitTestInvisible);
		CardOverlay->InsertChildAt(0, CardScrim);
		if (UOverlaySlot* ScrimSlot = Cast<UOverlaySlot>(CardScrim->Slot))
		{
			ScrimSlot->SetHorizontalAlignment(HAlign_Fill);
			ScrimSlot->SetVerticalAlignment(VAlign_Fill);
			ScrimSlot->SetPadding(FMargin(-36.0f, -28.0f));
		}
	}

	if (VignetteImage)
	{
		if (UMaterialInterface* BrushMaterial = Cast<UMaterialInterface>(VignetteImage->GetBrush().GetResourceObject()))
		{
			VignetteMaterial = UMaterialInstanceDynamic::Create(BrushMaterial, this);
			VignetteImage->SetBrushFromMaterial(VignetteMaterial);
		}
		VignetteImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	BindButton(StartButton, TEXT("Start"));
	BindButton(SettingsButton, TEXT("Settings"));
	BindButton(QuitButton, TEXT("Quit"));
	BindButton(LevelSettingsButton, TEXT("Settings"));
	BindButton(LevelQuitButton, TEXT("Quit"));
	BindButton(ResumeButton, TEXT("Resume"));
	BindButton(BackButton, TEXT("Back"));

	if (SensitivitySlider)
	{
		SensitivitySlider->SetMinValue(0.2f);
		SensitivitySlider->SetMaxValue(3.0f);
		if (OwnerCharacter.IsValid())
		{
			SensitivitySlider->SetValue(OwnerCharacter->GetLookSensitivity());
		}
		SensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UCaveUserWidget::HandleSensitivityChanged);
		const FLinearColor Ivory(0.898f, 0.867f, 0.796f, 1.0f);
		SensitivitySlider->SetSliderBarColor(FLinearColor(Ivory.R, Ivory.G, Ivory.B, 0.22f));
		SensitivitySlider->SetSliderHandleColor(Ivory);
	}

	if (SpeakerText)
	{
		SpeakerText->SetText(SpeakerName);
	}

	if (StoryBody)
	{
		StoryBody->SetText(ArtistStatement);
	}

	if (LevelBodyText && LevelBodyText->GetText().IsEmpty())
	{
		LevelBodyText->SetText(bIncludeStart
			? INVTEXT("Settings and quit stay available while you play. Press Esc to open them again.")
			: INVTEXT("You are the mole now. The gravel and the insects are close. Press Esc for Settings and Quit."));
	}

	if (HintText && HintText->GetText().IsEmpty())
	{
		HintText->SetText(INVTEXT("Esc    Settings  ·  Quit"));
	}

	ApplyPanelVisibility();
	RefreshSpeech();
	TickMenuOpacity(1.0f);
}

void UCaveUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	TickMenuOpacity(InDeltaTime);
}

void UCaveUserWidget::TickMenuOpacity(float DeltaTime)
{
	UButton* Buttons[] = {
		StartButton, SettingsButton, QuitButton, LevelSettingsButton,
		LevelQuitButton, ResumeButton, BackButton
	};

	for (UButton* Button : Buttons)
	{
		if (!Button)
		{
			continue;
		}
		const bool bHot = Button->IsHovered() || Button->HasKeyboardFocus();
		const float Target = bHot ? 1.0f : 0.26f;
		Button->SetRenderOpacity(FMath::FInterpTo(Button->GetRenderOpacity(), Target, DeltaTime, 7.0f));
	}
}

void UCaveUserWidget::ApplyRetinalImage(UImage* Image)
{
	if (!Image)
	{
		return;
	}

	const FString Name = Image->GetName();
	if (Name.Contains(TEXT("Scanline")))
	{
		if (UMaterialInterface* Scanline = LoadObject<UMaterialInterface>(
			nullptr, TEXT("/Game/GameProject/UI/Materials/M_UI_Scanline.M_UI_Scanline")))
		{
			Image->SetBrushFromMaterial(Scanline);
		}
		Image->SetColorAndOpacity(FLinearColor(0.898f, 0.867f, 0.796f, 0.18f));
		return;
	}

	if (Name.Contains(TEXT("Vignette")) && Style && Style->VignetteMaterial)
	{
		Image->SetBrushFromMaterial(Style->VignetteMaterial);
		return;
	}

	if (Name.Contains(TEXT("Prompt")) || Name.Contains(TEXT("Underline")))
	{
		if (Style && Style->PromptMaterial)
		{
			Image->SetBrushFromMaterial(Style->PromptMaterial);
		}
		FSlateBrush Brush = Image->GetBrush();
		Brush.ImageSize = FVector2D(180.0f, 2.0f);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Image->SetBrush(Brush);
		Image->SetDesiredSizeOverride(FVector2D(180.0f, 2.0f));
		return;
	}

	Image->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
}

void UCaveUserWidget::BindButton(UButton* Button, FName Handler)
{
	if (!Button)
	{
		return;
	}

	Button->OnHovered.AddUniqueDynamic(this, &UCaveUserWidget::HandleButtonHoverChanged);
	Button->OnUnhovered.AddUniqueDynamic(this, &UCaveUserWidget::HandleButtonHoverChanged);

	if (Handler == TEXT("Start"))
	{
		Button->OnClicked.AddUniqueDynamic(this, &UCaveUserWidget::HandleStartClicked);
	}
	else if (Handler == TEXT("Settings"))
	{
		Button->OnClicked.AddUniqueDynamic(this, &UCaveUserWidget::HandleSettingsClicked);
	}
	else if (Handler == TEXT("Quit"))
	{
		Button->OnClicked.AddUniqueDynamic(this, &UCaveUserWidget::HandleQuitClicked);
	}
	else if (Handler == TEXT("Resume"))
	{
		Button->OnClicked.AddUniqueDynamic(this, &UCaveUserWidget::HandleResumeClicked);
	}
	else if (Handler == TEXT("Back"))
	{
		Button->OnClicked.AddUniqueDynamic(this, &UCaveUserWidget::HandleBackClicked);
	}
}

void UCaveUserWidget::ApplySharedStyle(UWidget* Widget)
{
	if (!Widget || !Style)
	{
		return;
	}

	if (UTextBlock* Text = Cast<UTextBlock>(Widget))
	{
		const FString Name = Text->GetName();
		const FTextBlockStyle& TextStyle =
			Name.Contains(TEXT("Title")) ? Style->TitleStyle :
			Name.Contains(TEXT("Hint")) || Name.Contains(TEXT("Speaker")) ? Style->HintStyle :
			Name.Contains(TEXT("Label")) ? Style->ButtonLabelStyle :
			Style->BodyStyle;

		FSlateFontInfo Font = Text->GetFont();
		Font.OutlineSettings = TextStyle.Font.OutlineSettings;
		if (Style->FontObject)
		{
			Font.FontObject = Style->FontObject;
			Font.TypefaceFontName = TextStyle.Font.TypefaceFontName;
			Font.Size = TextStyle.Font.Size;
		}
		else if (TextStyle.Font.HasValidFont())
		{
			Font = TextStyle.Font;
		}
		else
		{
			Font.Size = TextStyle.Font.Size;
		}

		const bool bSubtitle = Name == TEXT("BodyText") || Name.Contains(TEXT("Speaker")) || Name.Contains(TEXT("SpeechHint"));
		Font.FontMaterial = bSubtitle ? Style->EmissiveTextMaterial.Get() : nullptr;

		Text->SetFont(Font);
		Text->SetColorAndOpacity(TextStyle.ColorAndOpacity);
		Text->SetShadowOffset(TextStyle.ShadowOffset);
		Text->SetShadowColorAndOpacity(TextStyle.ShadowColorAndOpacity);
	}
	else if (UButton* Button = Cast<UButton>(Widget))
	{
		Button->SetStyle(Style->ButtonStyle);
		Button->SetBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	}
	else if (UImage* Image = Cast<UImage>(Widget))
	{
		ApplyRetinalImage(Image);
	}

	if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
	{
		for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
		{
			ApplySharedStyle(Panel->GetChildAt(Index));
		}
	}
}

void UCaveUserWidget::SetPanelVisible(UWidget* Panel, bool bVisible) const
{
	if (Panel)
	{
		Panel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UCaveUserWidget::ApplyPanelVisibility()
{
	if (!Card && !StartPanel)
	{
		return;
	}

	if (!bCardOpen)
	{
		ShowPlaying();
		return;
	}

	if (bIncludeStart)
	{
		ShowStartCard();
		return;
	}

	ShowLevelCard();
}

void UCaveUserWidget::ShowStartCard()
{
	bCardOpen = true;
	bReturnToStart = true;
	if (TitleText)
	{
		TitleText->SetText(ProjectTitle);
	}
	SetPanelVisible(Card, true);
	SetPanelVisible(StartPanel, true);
	SetPanelVisible(LevelPanel, false);
	SetPanelVisible(SettingsPanel, false);
	SetPanelVisible(Hint, false);
}

void UCaveUserWidget::ShowLevelCard()
{
	bCardOpen = true;
	bReturnToStart = false;
	if (TitleText)
	{
		TitleText->SetText(bIncludeStart ? ProjectTitle : INVTEXT("Underground"));
	}
	SetPanelVisible(Card, true);
	SetPanelVisible(StartPanel, false);
	SetPanelVisible(LevelPanel, true);
	SetPanelVisible(SettingsPanel, false);
	SetPanelVisible(Hint, false);
}

void UCaveUserWidget::ShowPlaying()
{
	bCardOpen = false;
	SetPanelVisible(Card, false);
	SetPanelVisible(StartPanel, false);
	SetPanelVisible(LevelPanel, false);
	SetPanelVisible(SettingsPanel, false);
	SetPanelVisible(Hint, true);
}

void UCaveUserWidget::ShowSettings()
{
	bReturnToStart = StartPanel && StartPanel->GetVisibility() == ESlateVisibility::Visible;
	bCardOpen = true;
	SetPanelVisible(Card, true);
	SetPanelVisible(StartPanel, false);
	SetPanelVisible(LevelPanel, false);
	SetPanelVisible(SettingsPanel, true);
	SetPanelVisible(Hint, false);
}

void UCaveUserWidget::HandleStartClicked()
{
	if (OwnerCharacter.IsValid())
	{
		OwnerCharacter->DismissStartMenu();
	}
}

void UCaveUserWidget::HandleResumeClicked()
{
	if (OwnerCharacter.IsValid())
	{
		OwnerCharacter->CloseLevelMenu();
	}
}

void UCaveUserWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UCaveUserWidget::HandleSettingsClicked()
{
	ShowSettings();
}

void UCaveUserWidget::HandleBackClicked()
{
	if (bReturnToStart)
	{
		ShowStartCard();
	}
	else
	{
		ShowLevelCard();
	}
}

void UCaveUserWidget::HandleSensitivityChanged(float NewValue)
{
	if (OwnerCharacter.IsValid())
	{
		OwnerCharacter->SetLookSensitivity(NewValue);
	}
}

void UCaveUserWidget::HandleButtonHoverChanged()
{
	const FLinearColor Hover = Style ? Style->HoverTint : FLinearColor(1.35f, 1.28f, 1.08f, 1.0f);
	const UButton* Buttons[] = {
		StartButton, SettingsButton, QuitButton, LevelSettingsButton,
		LevelQuitButton, ResumeButton, BackButton
	};

	for (const UButton* Button : Buttons)
	{
		if (UButton* MutableButton = const_cast<UButton*>(Button))
		{
			MutableButton->SetColorAndOpacity(MutableButton->IsHovered() ? Hover : FLinearColor::White);
		}
	}
}

void UCaveUserWidget::SetLines(const TArray<FString>& InLines)
{
	Lines.Reset();
	for (const FString& Line : InLines)
	{
		if (!Line.IsEmpty())
		{
			Lines.Add(FText::FromString(Line));
		}
	}

	CurrentLine = 0;
	bNarrativeFinished = false;
	RefreshSpeech();
	BeginLineFade();
}

void UCaveUserWidget::SetSpeech(const FString& InSpeech)
{
	TArray<FString> AsLines;
	AsLines.Add(InSpeech);
	SetLines(AsLines);
}

void UCaveUserWidget::ShowFinale()
{
	SpeakerName = INVTEXT("The den");
	WalkHint = INVTEXT("The burrow is lit");

	if (SpeakerText)
	{
		SpeakerText->SetText(SpeakerName);
		SpeakerText->SetJustification(ETextJustify::Center);
		SpeakerText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.78f, 0.42f, 1.0f)));
		FSlateFontInfo Font = SpeakerText->GetFont();
		Font.Size = FMath::Max(Font.Size, 22);
		SpeakerText->SetFont(Font);
	}

	if (BodyText)
	{
		BodyText->SetJustification(ETextJustify::Center);
		BodyText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.94f, 0.78f, 1.0f)));
		BodyText->SetAutoWrapText(false);
		BodyText->SetWrapTextAt(860.0f);
		FSlateFontInfo Font = BodyText->GetFont();
		Font.Size = FMath::Max(Font.Size + 8, 32);
		BodyText->SetFont(Font);
	}

	if (SpeechHintText)
	{
		SpeechHintText->SetText(WalkHint);
		SpeechHintText->SetJustification(ETextJustify::Center);
		SpeechHintText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.86f, 0.55f, 0.9f)));
	}

	if (PromptImage)
	{
		PromptImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SpeechColumn)
	{
		SpeechColumn->SetRenderScale(FVector2D(1.12f, 1.12f));
	}

	if (SpeechStack)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SpeechStack->Slot))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.58f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(FVector2D::ZeroVector);
			CanvasSlot->SetSize(FVector2D(980.0f, 320.0f));
			CanvasSlot->SetAutoSize(false);
		}
	}
}

void UCaveUserWidget::ShowLine(int32 LineIndex)
{
	CurrentLine = LineIndex;
	RefreshSpeech();
	BeginLineFade();
}

void UCaveUserWidget::Advance()
{
	if (Lines.Num() == 0)
	{
		bNarrativeFinished = true;
		RemoveFromParent();
		return;
	}

	CurrentLine += 1;
	if (CurrentLine >= Lines.Num())
	{
		bNarrativeFinished = true;
		RemoveFromParent();
		return;
	}

	RefreshSpeech();
	BeginLineFade();
}

void UCaveUserWidget::SetWalkProgress(float Amount)
{
	// Distance is gameplay state. The prompt bar animates in its material, so this
	// does not rewrite text color.
	(void)Amount;
}

void UCaveUserWidget::RefreshSpeech()
{
	if (SpeakerText)
	{
		SpeakerText->SetText(SpeakerName);
	}

	if (BodyText && Lines.IsValidIndex(CurrentLine))
	{
		BodyText->SetText(Lines[CurrentLine]);
	}

	if (SpeechHintText)
	{
		const FString HintString = WalkHint.ToString();
		const bool bPulseRule = HintString.Contains(TEXT("Walk")) || HintString.Contains(TEXT("hear"));
		SpeechHintText->SetVisibility(bPulseRule ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		if (!bPulseRule)
		{
			SpeechHintText->SetText(WalkHint);
		}
	}

	if (PromptImage)
	{
		const FString HintString = WalkHint.ToString();
		const bool bPulseRule = HintString.Contains(TEXT("Walk")) || HintString.Contains(TEXT("hear"));
		PromptImage->SetVisibility(bPulseRule ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		ApplyRetinalImage(PromptImage);
	}
}

void UCaveUserWidget::BeginLineFade()
{
	if (!BodyText)
	{
		return;
	}

	FadeAlpha = 0.0f;
	BodyText->SetRenderOpacity(0.0f);
	BodyText->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	BodyText->SetRenderScale(FVector2D(1.04f, 1.04f));
	if (VignetteMaterial)
	{
		VignetteMaterial->SetScalarParameterValue(TEXT("BlurAmount"), 2.4f);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FadeTimer,
			this,
			&UCaveUserWidget::TickLineFade,
			0.016f,
			true);
	}
}

void UCaveUserWidget::TickLineFade()
{
	FadeAlpha = FMath::FInterpTo(FadeAlpha, 1.0f, 0.016f, 2.2f);
	if (BodyText)
	{
		BodyText->SetRenderOpacity(FadeAlpha);
		const float Settle = FMath::Lerp(1.04f, 1.0f, FadeAlpha);
		BodyText->SetRenderScale(FVector2D(Settle, Settle));
		const float RestShadow = Style ? Style->BodyStyle.ShadowOffset.Y : 1.0f;
		const float Shadow = FMath::Lerp(8.0f, RestShadow, FadeAlpha);
		BodyText->SetShadowOffset(FVector2D(Shadow, Shadow));
	}

	if (VignetteMaterial)
	{
		VignetteMaterial->SetScalarParameterValue(TEXT("BlurAmount"), 1.0f - FadeAlpha);
	}

	if (FadeAlpha > 0.98f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FadeTimer);
		}
	}
}
