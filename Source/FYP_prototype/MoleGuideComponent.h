#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MoleGuideComponent.generated.h"

UCLASS(ClassGroup = (Creature), Blueprintable, meta = (BlueprintSpawnableComponent))
class FYP_PROTOTYPE_API UMoleGuideComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMoleGuideComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole")
    float WalkSpan = 160.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole")
    float WalkSpeed = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole")
    bool bFacePlayer = true;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FVector Home = FVector::ZeroVector;
    float WalkTime = 0.0f;
};
