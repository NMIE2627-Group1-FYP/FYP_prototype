#include "GuideArrowComponent.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"

UGuideArrowComponent::UGuideArrowComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGuideArrowComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        TArray<USceneComponent*> Parts;
        Owner->GetComponents<USceneComponent>(Parts);
        for (USceneComponent* Part : Parts)
        {
            Part->SetMobility(EComponentMobility::Movable);
        }
        Owner->SetActorEnableCollision(false);
    }

    ResolvePortal();
}

void UGuideArrowComponent::ResolvePortal()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }

        if (Actor->ActorHasTag(TEXT("ManholePortal")))
        {
            Portal = Actor;
            return;
        }
    }
}

void UGuideArrowComponent::TickComponent(
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

    if (!Portal.IsValid())
    {
        ResolvePortal();
    }

    UWorld* World = GetWorld();
    if (World && World->GetMapName().Contains(TEXT("Underground")))
    {
        return;
    }

    PointAtPortal();
}

void UGuideArrowComponent::PointAtPortal()
{
    AActor* Owner = GetOwner();
    AActor* Target = Portal.Get();
    if (!Owner || !Target)
    {
        return;
    }

    const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(
        Owner->GetActorLocation(),
        Target->GetActorLocation()
    );
    Owner->SetActorRotation(FRotator(0.0f, LookAt.Yaw, 0.0f));
}
