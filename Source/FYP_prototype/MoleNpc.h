#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoleNpc.generated.h"

class UMoleSpeechBox;

UCLASS(Blueprintable)
class FYP_PROTOTYPE_API AMoleNpc : public AActor
{
    GENERATED_BODY()

public:
    AMoleNpc();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = true))
    FString SpeechText;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, Category = "Mole")
    TObjectPtr<USceneComponent> SceneRoot;

private:
    UPROPERTY()
    TObjectPtr<UMoleSpeechBox> SpeechBox;

    void FaceCamera();
};
