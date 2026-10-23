#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreatureStartMenu.generated.h"

class ACreatureCharacter;

UCLASS(Blueprintable)
class FYP_PROTOTYPE_API UCreatureStartMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    UCreatureStartMenu(const FObjectInitializer& ObjectInitializer);

    void SetOwnerCharacter(ACreatureCharacter* InCharacter);

    void SetIncludeStart(bool bInIncludeStart);

    void ShowStartCard();

    void ShowLevelCard();

    void ShowPlaying();

    bool IsCardOpen() const;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void ShowSettings();

    void HandleStart();

    void HandleResume();

    void HandleQuit();

    void HandleSensitivityChanged(float NewValue);

    TSharedRef<SWidget> MakeButton(const FString& Label, TFunction<void()> Callback) const;

    TWeakObjectPtr<ACreatureCharacter> OwnerCharacter;

    bool bIncludeStart = true;

    bool bCardOpen = true;

    bool bReturnToStart = false;

    TSharedPtr<SWidget> Card;

    TSharedPtr<SWidget> StartPanel;

    TSharedPtr<SWidget> LevelPanel;

    TSharedPtr<SWidget> SettingsPanel;

    TSharedPtr<SWidget> Hint;

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText SurfaceTitle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText UndergroundTitle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText StoryText;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText StartLabel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText SettingsLabel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText QuitLabel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText ResumeLabel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText BackLabel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText EscHint;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText UndergroundHint;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText EscHelpText;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Text")
    FText UndergroundHelpText;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Layout")
    float CardWidth = 460.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu|Layout")
    FMargin CardPadding;
};
