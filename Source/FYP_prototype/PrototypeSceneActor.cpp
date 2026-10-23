#include "PrototypeSceneActor.h"

#include "BlueCubeActor.h"
#include "ChromeSphereActor.h"

APrototypeSceneActor::APrototypeSceneActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APrototypeSceneActor::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        const FVector BlueCubeLocation(0.0f, 0.0f, 80.0f);
        const FVector ChromeSphereLocation(180.0f, 0.0f, 80.0f);

        GetWorld()->SpawnActor<ABlueCubeActor>(
            ABlueCubeActor::StaticClass(),
            BlueCubeLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );

        GetWorld()->SpawnActor<AChromeSphereActor>(
            AChromeSphereActor::StaticClass(),
            ChromeSphereLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );
    }
}
