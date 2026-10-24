#include "PrototypeGameMode.h"

#include "CreatureCharacter.h"
#include "GameFramework/PlayerController.h"

APrototypeGameMode::APrototypeGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultPawnClass = ACreatureCharacter::StaticClass();
}

void APrototypeGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        PlayerController->ConsoleCommand(TEXT("DisableAllScreenMessages"));
    }
}
