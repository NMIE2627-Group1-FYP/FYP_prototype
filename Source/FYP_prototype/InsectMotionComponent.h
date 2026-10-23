#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InsectMotionComponent.generated.h"

UENUM(BlueprintType)
enum class EInsectMotion : uint8
{
    Drift,
    Zigzag,
    Circle,
    Dart
};

UCLASS(ClassGroup = (Creature), Blueprintable, meta = (BlueprintSpawnableComponent))
class FYP_PROTOTYPE_API UInsectMotionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInsectMotionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect")
    EInsectMotion Motion = EInsectMotion::Drift;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect")
    float Amplitude = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect")
    float Speed = 0.8f;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FVector Home = FVector::ZeroVector;
    float Time = 0.0f;
};
