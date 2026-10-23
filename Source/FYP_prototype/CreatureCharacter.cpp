#include "CreatureCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Math/RotationMatrix.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ACreatureCharacter::ACreatureCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // Body yaws with the controller. Pitch stays on the camera.
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // Strafe in the look direction instead of turning toward velocity.
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Initial test speed.
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;

    // Content defaults. The Blueprint can still override these.
    static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextAsset(
        TEXT("/Game/L_test/Input/IMC_Player.IMC_Player")
    );
    static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(
        TEXT("/Game/L_test/Input/IA_Move.IA_Move")
    );
    static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(
        TEXT("/Game/L_test/Input/IA_Look.IA_Look")
    );

    if (MappingContextAsset.Succeeded())
    {
        DefaultMappingContext = MappingContextAsset.Object;
    }

    if (MoveActionAsset.Succeeded())
    {
        MoveAction = MoveActionAsset.Object;
    }

    if (LookActionAsset.Succeeded())
    {
        LookAction = LookActionAsset.Object;
    }
}

namespace
{
    const TCHAR* MappingContextPath = TEXT("/Game/L_test/Input/IMC_Player.IMC_Player");
    const TCHAR* MoveActionPath = TEXT("/Game/L_test/Input/IA_Move.IA_Move");
    const TCHAR* LookActionPath = TEXT("/Game/L_test/Input/IA_Look.IA_Look");
}

void ACreatureCharacter::ResolveInputAssets()
{
    // Blueprint class defaults can stay empty even after the native constructor
    // assigns these, so load the project assets at runtime when they are missing.
    if (!DefaultMappingContext)
    {
        DefaultMappingContext = LoadObject<UInputMappingContext>(
            nullptr,
            MappingContextPath
        );
    }

    if (!MoveAction)
    {
        MoveAction = LoadObject<UInputAction>(nullptr, MoveActionPath);
    }

    if (!LookAction)
    {
        LookAction = LoadObject<UInputAction>(nullptr, LookActionPath);
    }
}

void ACreatureCharacter::AddDefaultMappingContext()
{
    ResolveInputAssets();

    APlayerController* PlayerController =
        Cast<APlayerController>(GetController());

    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (DefaultMappingContext)
            {
                InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // Keep the cursor captured so the viewport receives mouse look.
    PlayerController->SetInputMode(FInputModeGameOnly());
    PlayerController->SetShowMouseCursor(false);
}

// Called when the game starts or when spawned
void ACreatureCharacter::BeginPlay()
{
    Super::BeginPlay();

    TArray<UCameraComponent*> Cameras;
    GetComponents<UCameraComponent>(Cameras);

    if (Cameras.IsEmpty())
    {
        UCameraComponent* Camera = NewObject<UCameraComponent>(
            this,
            TEXT("FirstPersonCamera")
        );
        Camera->SetupAttachment(GetCapsuleComponent());
        Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
        Camera->bUsePawnControlRotation = true;
        Camera->RegisterComponent();
        Cameras.Add(Camera);
    }

    for (UCameraComponent* Camera : Cameras)
    {
        if (Camera)
        {
            Camera->bUsePawnControlRotation = true;
        }
    }
}

void ACreatureCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();

    // Runs after this pawn is possessed by the local player.
    AddDefaultMappingContext();
}

// Called to bind functionality to input
void ACreatureCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent
)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    ResolveInputAssets();
    AddDefaultMappingContext();

    UEnhancedInputComponent* EnhancedInputComponent =
        Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!EnhancedInputComponent)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("CreatureCharacter: EnhancedInputComponent is missing.")
        );

        return;
    }

    if (MoveAction)
    {
        EnhancedInputComponent->BindAction(
            MoveAction,
            ETriggerEvent::Triggered,
            this,
            &ACreatureCharacter::Move
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("CreatureCharacter: Assign MoveAction in the Blueprint.")
        );
    }

    if (LookAction)
    {
        EnhancedInputComponent->BindAction(
            LookAction,
            ETriggerEvent::Triggered,
            this,
            &ACreatureCharacter::Look
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("CreatureCharacter: Assign LookAction in the Blueprint.")
        );
    }

    if (!DefaultMappingContext)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("CreatureCharacter: Assign DefaultMappingContext in the Blueprint.")
        );
    }
}

// WASD movement
void ACreatureCharacter::Move(const FInputActionValue& Value)
{
    if (!Controller)
    {
        return;
    }

    // Keep diagonal WASD at the same speed as a single direction.
    const FVector2D MovementInput = Value.Get<FVector2D>().GetSafeNormal();

    // Use horizontal facing only, not the camera's up/down angle.
    const FRotator ControlRotation = Controller->GetControlRotation();

    const FRotator YawRotation(
        0.0f,
        ControlRotation.Yaw,
        0.0f
    );

    const FVector ForwardDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    const FVector RightDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, MovementInput.Y);
    AddMovementInput(RightDirection, MovementInput.X);
}

// Mouse look
void ACreatureCharacter::Look(const FInputActionValue& Value)
{
    if (!Controller)
    {
        return;
    }

    const FVector2D LookInput = Value.Get<FVector2D>();

    AddControllerYawInput(LookInput.X * MouseSensitivity);
    AddControllerPitchInput(LookInput.Y * MouseSensitivity);
}