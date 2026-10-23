#include "InsectMotionComponent.h"

UInsectMotionComponent::UInsectMotionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInsectMotionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        Home = Owner->GetActorLocation();
    }
}

void UInsectMotionComponent::TickComponent(
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

    Time += DeltaTime * Speed;
    FVector Offset = FVector::ZeroVector;

    switch (Motion)
    {
    case EInsectMotion::Zigzag:
        Offset.X = FMath::Sin(Time * 2.2f) * Amplitude;
        Offset.Y = FMath::Square(FMath::Sin(Time)) * Amplitude * 0.65f;
        break;
    case EInsectMotion::Circle:
        Offset.X = FMath::Cos(Time) * Amplitude;
        Offset.Y = FMath::Sin(Time * 1.35f) * Amplitude * 0.8f;
        break;
    case EInsectMotion::Dart:
        Offset.X = FMath::Fmod(Time * Amplitude * 0.35f, Amplitude * 2.0f) - Amplitude;
        Offset.Y = FMath::Sin(Time * 6.0f) * (Amplitude * 0.25f);
        break;
    case EInsectMotion::Drift:
    default:
        Offset.X = FMath::Sin(Time * 0.7f) * Amplitude;
        Offset.Y = FMath::Cos(Time * 0.45f) * Amplitude * 0.55f;
        break;
    }

    FVector Location = Home + Offset;
    Location.Z = Home.Z;
    Owner->SetActorLocation(Location);
}
