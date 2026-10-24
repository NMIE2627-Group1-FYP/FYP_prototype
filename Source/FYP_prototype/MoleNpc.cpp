#include "MoleNpc.h"

#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "CaveContrastComponent.h"
#include "CaveUserWidget.h"

AMoleNpc::AMoleNpc()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    Lines = {
        TEXT("You wake in the dark with an empty, aching stomach."),
        TEXT("The den's food store is bare."),
        TEXT("Behind you there is faint squeaking and slow breathing. The family depends on you."),
        TEXT("The floor hums under your feet. Find direction by hearing, and judge the ground by stepping."),
        TEXT("There is one purpose: find food and bring it home."),
    };
    WalkHint = INVTEXT("Walk to hear the next line");
}

void AMoleNpc::BeginPlay()
{
    Super::BeginPlay();

    // The player is already the mole. Extra NPC moles stay out of the burrow.
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);
}

void AMoleNpc::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    FaceCamera();
    AdvanceDialogue();
}

void AMoleNpc::AdvanceDialogue()
{
    if (!SpeechBox)
    {
        return;
    }

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player)
    {
        return;
    }

    const FVector Location = Player->GetActorLocation();
    if (!bHasStepOrigin)
    {
        StepOrigin = Location;
        bHasStepOrigin = true;
        SpeechBox->SetWalkProgress(0.0f);
        return;
    }

    const float Travelled = FVector::Dist2D(Location, StepOrigin);
    const float Needed = FMath::Max(StepDistance, 1.0f);
    SpeechBox->SetWalkProgress(Travelled / Needed);
    if (Travelled < Needed)
    {
        return;
    }

    StepOrigin = Location;
    SpeechBox->SetWalkProgress(0.0f);
    SpeechBox->Advance();

    if (SpeechBox->IsNarrativeFinished())
    {
        if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
        {
            if (UCaveContrastComponent* Contrast = PlayerController->FindComponentByClass<UCaveContrastComponent>())
            {
                Contrast->NotifyNarrativeComplete();
            }
        }
    }
}

void AMoleNpc::FaceCamera()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        return;
    }

    FVector CameraLocation = FVector::ZeroVector;
    FRotator CameraRotation = FRotator::ZeroRotator;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CameraLocation);
    LookAt.Pitch = 0.0f;
    LookAt.Roll = 0.0f;
    SetActorRotation(LookAt);
}
