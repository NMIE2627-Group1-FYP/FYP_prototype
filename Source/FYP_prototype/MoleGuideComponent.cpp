#include "MoleGuideComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UMoleGuideComponent::UMoleGuideComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMoleGuideComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        Home = Owner->GetActorLocation();
    }
}

void UMoleGuideComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    WalkTime += DeltaTime;
    const float Along = FMath::Sin(WalkTime * (WalkSpeed / FMath::Max(WalkSpan, 1.0f))) * WalkSpan;
    Owner->SetActorLocation(Home + FVector(Along, 0.0f, 0.0f));

    if (!bFacePlayer)
    {
        return;
    }

    if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        FVector ToPlayer = Player->GetActorLocation() - Owner->GetActorLocation();
        ToPlayer.Z = 0.0f;
        if (!ToPlayer.IsNearlyZero())
        {
            Owner->SetActorRotation(ToPlayer.Rotation());
        }
    }
}
