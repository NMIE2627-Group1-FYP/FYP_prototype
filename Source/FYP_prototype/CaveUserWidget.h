#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CaveUserWidget.generated.h"

class ACreatureCharacter;
class UButton;
class UCaveUIStyle;
class UImage;
class UMaterialInstanceDynamic;
class USlider;
class UTextBlock;
class UWidget;

/**
 * Shared parent behaviour for WBP_Base_CAVE and every screen that inherits it.
 * Layout lives in the Designer. This class only routes input and the line fade.
 */
UCLASS(Blueprintable)
class FYP_PROTOTYPE_API UCaveUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCaveUserWidget(const FObjectInitializer& ObjectInitializer);

	void SetOwnerCharacter(ACreatureCharacter* InCharacter);

	void SetIncludeStart(bool bInIncludeStart);

	void ShowStartCard();

	void ShowLevelCard();

	void ShowPlaying();

	void ShowSettings();

	bool IsCardOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowLine(int32 LineIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetLines(const TArray<FString>& InLines);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetSpeech(const FString& InSpeech);

	/** Last line. Larger, warmer, and centered. The walk prompt goes away. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowFinale();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Advance();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetWalkProgress(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool IsNarrativeFinished() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FText> Lines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText WalkHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project")
	FText ProjectTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project", meta = (MultiLine = true))
	FText ArtistStatement;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleBackClicked();

	UFUNCTION()
	void HandleSensitivityChanged(float NewValue);

	UFUNCTION()
	void HandleButtonHoverChanged();

private:
	void ApplySharedStyle(UWidget* Widget);
	void BindMissingSpeechWidgets(UWidget* Widget);
	void ApplyPanelVisibility();
	void RefreshSpeech();
	void BeginLineFade();
	void TickLineFade();
	void TickMenuOpacity(float DeltaTime);
	void ApplyRetinalImage(UImage* Image);
	void BindButton(UButton* Button, FName Handler);
	void SetPanelVisible(UWidget* Panel, bool bVisible) const;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Card;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> StartPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> LevelPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SettingsPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Hint;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StoryBody;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelBodyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeakerText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BodyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeechHintText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SpeechStack;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SpeechColumn;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> PromptImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ScanlineImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> LevelSettingsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> LevelQuitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> SensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> VignetteImage;

	UPROPERTY()
	TObjectPtr<UImage> CardScrim;

	UPROPERTY()
	TObjectPtr<UCaveUIStyle> Style;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> VignetteMaterial;

	TWeakObjectPtr<ACreatureCharacter> OwnerCharacter;

	FTimerHandle FadeTimer;

	float FadeAlpha = 1.0f;

	bool bIncludeStart = true;

	bool bCardOpen = true;

	bool bReturnToStart = false;

	bool bNarrativeFinished = false;

	int32 CurrentLine = 0;
};
