#include "MoleNpc.h"

#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MoleSpeechBox.h"

AMoleNpc::AMoleNpc()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SpeechText = TEXT("You made it below. From here, you are me.");
}

void AMoleNpc::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World || !World->GetMapName().Contains(TEXT("Underground")))
    {
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        return;
    }

    UClass* WidgetClass = LoadClass<UMoleSpeechBox>(
        nullptr,
        TEXT("/Game/GameProject/Blueprints/WBP_MoleSpeech.WBP_MoleSpeech_C")
    );
    if (!WidgetClass)
    {
        WidgetClass = UMoleSpeechBox::StaticClass();
    }

    SpeechBox = CreateWidget<UMoleSpeechBox>(PlayerController, WidgetClass);
    if (!SpeechBox)
    {
        return;
    }

    SpeechBox->SetSpeech(SpeechText);
    SpeechBox->AddToViewport(20);
}

void AMoleNpc::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    FaceCamera();
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
