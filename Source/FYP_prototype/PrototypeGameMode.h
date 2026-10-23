#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PrototypeGameMode.generated.h"

UCLASS()
class FYP_PROTOTYPE_API APrototypeGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APrototypeGameMode();

protected:
    virtual void BeginPlay() override;
};
