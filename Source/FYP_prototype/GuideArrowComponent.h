#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuideArrowComponent.generated.h"

UCLASS(ClassGroup = (Creature), Blueprintable, meta = (BlueprintSpawnableComponent))
class FYP_PROTOTYPE_API UGuideArrowComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGuideArrowComponent();

    UFUNCTION(BlueprintCallable, Category = "Guide")
    void PointAtPortal();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void ResolvePortal();

    TWeakObjectPtr<AActor> Portal;
};
