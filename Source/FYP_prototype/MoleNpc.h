#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoleNpc.generated.h"

class UCaveUserWidget;

UCLASS(Blueprintable)
class FYP_PROTOTYPE_API AMoleNpc : public AActor
{
    GENERATED_BODY()

public:
    AMoleNpc();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = true))
    TArray<FString> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (ClampMin = "1.0"))
    float StepDistance = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText WalkHint;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, Category = "Mole")
    TObjectPtr<USceneComponent> SceneRoot;

private:
    UPROPERTY()
    TObjectPtr<UCaveUserWidget> SpeechBox;

    void FaceCamera();

    void AdvanceDialogue();

    FVector StepOrigin = FVector::ZeroVector;

    bool bHasStepOrigin = false;
};
