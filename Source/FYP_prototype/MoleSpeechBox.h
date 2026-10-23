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
    void SetSpeech(const FString& InSpeech);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText NextLabel;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void HandleNext();

    void Refresh();

    int32 CurrentLine = 0;

    TSharedPtr<STextBlock> BodyText;
};
