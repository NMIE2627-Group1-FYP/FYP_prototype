#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreatureStartMenu.generated.h"

class ACreatureCharacter;

UCLASS()
class FYP_PROTOTYPE_API UCreatureStartMenu : public UUserWidget
{
    GENERATED_BODY()

public:
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
};
