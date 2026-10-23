#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

class STextBlock;

#include "MoleSpeechBox.generated.h"

UCLASS(Blueprintable)
class FYP_PROTOTYPE_API UMoleSpeechBox : public UUserWidget
{
    GENERATED_BODY()

public:
    UMoleSpeechBox(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ShowLine(int32 LineIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetLines(const TArray<FString>& InLines);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetSpeech(const FString& InSpeech);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void Advance();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetWalkProgress(float Amount);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText WalkHint;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void Refresh();

    void HandleAdvance();

    int32 CurrentLine = 0;

    float WalkProgress = 0.0f;

    TSharedPtr<STextBlock> BodyText;

    TSharedPtr<STextBlock> HintText;

    TSharedPtr<SBox> ProgressFill;
};
