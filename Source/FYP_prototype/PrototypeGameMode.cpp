#include "PrototypeGameMode.h"

#include "CreatureCharacter.h"
#include "PrototypeSceneActor.h"

APrototypeGameMode::APrototypeGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultPawnClass = ACreatureCharacter::StaticClass();
}

void APrototypeGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        GetWorld()->SpawnActor<APrototypeSceneActor>(
            APrototypeSceneActor::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParams
        );
    }
}
