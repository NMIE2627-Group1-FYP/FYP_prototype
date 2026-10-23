#include "InsectMotionComponent.h"

#include "Components/SkeletalMeshComponent.h"

UInsectMotionComponent::UInsectMotionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInsectMotionComponent::BeginPlay()
{
    Super::BeginPlay();
    SetComponentTickEnabled(false);

    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorTickEnabled(false);
        TArray<USkeletalMeshComponent*> Meshes;
        Owner->GetComponents<USkeletalMeshComponent>(Meshes);
        for (USkeletalMeshComponent* Mesh : Meshes)
        {
            Mesh->Stop();
            Mesh->bPauseAnims = true;
        }
    }
}

void UInsectMotionComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
