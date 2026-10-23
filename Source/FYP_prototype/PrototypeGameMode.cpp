#include "PrototypeGameMode.h"

#include "CreatureCharacter.h"

APrototypeGameMode::APrototypeGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultPawnClass = ACreatureCharacter::StaticClass();
}

void APrototypeGameMode::BeginPlay()
{
    Super::BeginPlay();
}
