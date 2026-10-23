#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrototypeSceneActor.generated.h"

UCLASS()
class FYP_PROTOTYPE_API APrototypeSceneActor : public AActor
{
    GENERATED_BODY()

public:
    APrototypeSceneActor();

protected:
    virtual void BeginPlay() override;
};
