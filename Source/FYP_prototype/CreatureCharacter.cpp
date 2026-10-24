#include "CreatureCharacter.h"

#include "CaveContrastComponent.h"
#include "CaveUserWidget.h"
#include "StoryProgressComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "BlueCubeActor.h"
#include "ChromeSphereActor.h"
#include "EngineUtils.h"
#include "MoleNpc.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "PrototypeSceneActor.h"
#include "Math/RotationMatrix.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ACreatureCharacter::ACreatureCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Body yaws with the controller. Pitch stays on the camera.
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // Strafe in the look direction instead of turning toward velocity.
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Surface pace is a walking speed. ApplyStance retunes this per level.
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
    GetCharacterMovement()->MaxAcceleration = 800.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 1400.0f;

    // Content defaults. The Blueprint can still override these.
    static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextAsset(
        TEXT("/Game/GameProject/L_test/Input/IMC_Player.IMC_Player")
    );
    static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(
        TEXT("/Game/GameProject/L_test/Input/IA_Move.IA_Move")
    );
    static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(
        TEXT("/Game/GameProject/L_test/Input/IA_Look.IA_Look")
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
    const TCHAR* MappingContextPath = TEXT("/Game/GameProject/L_test/Input/IMC_Player.IMC_Player");
    const TCHAR* MoveActionPath = TEXT("/Game/GameProject/L_test/Input/IA_Move.IA_Move");
    const TCHAR* LookActionPath = TEXT("/Game/GameProject/L_test/Input/IA_Look.IA_Look");
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
void ACreatureCharacter::EnsureCaveContrast()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController)
    {
        return;
    }

    PlayerController->ConsoleCommand(TEXT("DisableAllScreenMessages"));

    if (!PlayerController->FindComponentByClass<UCaveContrastComponent>())
    {
        UCaveContrastComponent* Contrast = NewObject<UCaveContrastComponent>(
            PlayerController,
            TEXT("CaveContrast"));
        Contrast->RegisterComponent();
    }

    if (!FindComponentByClass<UStoryProgressComponent>())
    {
        UStoryProgressComponent* Story = NewObject<UStoryProgressComponent>(this, TEXT("StoryProgress"));
        Story->RegisterComponent();
    }
}

void ACreatureCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        PlayerController->ConsoleCommand(TEXT("DisableAllScreenMessages"));
    }

    TArray<UCameraComponent*> Cameras;
    GetComponents<UCameraComponent>(Cameras);

    if (Cameras.IsEmpty())
    {
        UCameraComponent* Camera = NewObject<UCameraComponent>(
            this,
            TEXT("FirstPersonCamera")
        );
        Camera->SetupAttachment(GetCapsuleComponent());
        Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 72.0f));
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

    ApplyStance();
    HideManholePlaceholders();
}

void ACreatureCharacter::HideManholePlaceholders()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> RemoveOnPlay;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor)
        {
            continue;
        }

        if (Actor->IsA<ABlueCubeActor>() ||
            Actor->IsA<AChromeSphereActor>() ||
            Actor->IsA<APrototypeSceneActor>())
        {
            RemoveOnPlay.Add(Actor);
            continue;
        }

        const bool bSurface = !World->GetMapName().Contains(TEXT("Underground"));
        if (bSurface &&
            (Actor->ActorHasTag(TEXT("HideInGame")) ||
             Actor->ActorHasTag(TEXT("GuideArrow"))))
        {
            Actor->SetActorEnableCollision(false);
        }

        if (bSurface && Actor->ActorHasTag(TEXT("HideInGame")))
        {
            Actor->SetActorHiddenInGame(true);
        }
    }

    for (AActor* Actor : RemoveOnPlay)
    {
        Actor->Destroy();
    }
}

void ACreatureCharacter::ApplyStance()
{
    UWorld* World = GetWorld();
    const bool bMole =
        World && World->GetMapName().Contains(TEXT("Underground"));

    const float CapsuleRadius = bMole ? 18.0f : 34.0f;
    const float CapsuleHalfHeight = bMole ? 16.0f : 88.0f;
    // Surface eye line sits near 160 cm. Underground sits about 8 cm off the floor.
    const float CameraHeight = bMole ? -8.0f : 72.0f;
    const float WalkSpeed = bMole ? 180.0f : 500.0f;

    GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight, true);

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = WalkSpeed;
    Movement->MaxAcceleration = bMole ? 350.0f : 800.0f;
    Movement->BrakingDecelerationWalking = bMole ? 900.0f : 1400.0f;
    Movement->JumpZVelocity = bMole ? 180.0f : 420.0f;

    TArray<UCameraComponent*> Cameras;
    GetComponents<UCameraComponent>(Cameras);
    for (UCameraComponent* Camera : Cameras)
    {
        if (Camera)
        {
            Camera->SetRelativeLocation(FVector(0.0f, 0.0f, CameraHeight));
        }
    }

    const float BodyScale = bMole ? (CapsuleHalfHeight / 88.0f) : 1.0f;
    TArray<USkeletalMeshComponent*> BodyMeshes;
    GetComponents<USkeletalMeshComponent>(BodyMeshes);
    for (USkeletalMeshComponent* BodyMesh : BodyMeshes)
    {
        if (!BodyMesh)
        {
            continue;
        }

        BodyMesh->SetRelativeScale3D(FVector(BodyScale));
        BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalfHeight));
    }

    if (bMole)
    {
        UE_LOG(LogTemp, Log, TEXT("Entered the underground level."));
        for (TActorIterator<AMoleNpc> It(World); It; ++It)
        {
            AMoleNpc* ExtraMole = *It;
            if (!ExtraMole)
            {
                continue;
            }
            ExtraMole->SetActorHiddenInGame(true);
            ExtraMole->SetActorEnableCollision(false);
            ExtraMole->SetActorTickEnabled(false);
            ExtraMole->Destroy();
        }
    }
}

void ACreatureCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bBurrowTransitionStarted)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World || World->GetMapName().Contains(TEXT("Underground")))
    {
        return;
    }

    if (bAwaitingStart)
    {
        return;
    }

    UpdateGrassResponse();

    const FVector Location = GetActorLocation();
    const float DistanceFromManhole = FVector2D(Location.X, Location.Y).Size();
    const bool bStandingOnManhole =
        DistanceFromManhole < 260.0f && Location.Z > -40.0f && Location.Z < 280.0f;

    if (bStandingOnManhole)
    {
        bBurrowTransitionStarted = true;
        UE_LOG(LogTemp, Log, TEXT("Entering the underground level from the manhole."));

        UGameplayStatics::OpenLevel(
            this,
            FName(TEXT("/Game/GameProject/Levels/Underground"))
        );
    }
}

void ACreatureCharacter::ShowStartMenu()
{
    if (bStartMenuShown)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    UClass* MenuClass = LoadClass<UCaveUserWidget>(
        nullptr,
        TEXT("/Game/GameProject/Blueprints/WBP_StartMenu.WBP_StartMenu_C")
    );
    if (!MenuClass)
    {
        MenuClass = UCaveUserWidget::StaticClass();
    }

    StartMenu = CreateWidget<UCaveUserWidget>(PlayerController, MenuClass);

    if (!StartMenu)
    {
        return;
    }

    const bool bUnderground = World->GetMapName().Contains(TEXT("Underground"));
    bStartMenuShown = true;
    bAwaitingStart = !bUnderground;
    StartMenu->SetOwnerCharacter(this);
    StartMenu->SetIncludeStart(!bUnderground);
    StartMenu->AddToViewport(10);

    if (bUnderground)
    {
        StartMenu->ShowPlaying();
        return;
    }

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(StartMenu->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(InputMode);
    PlayerController->SetShowMouseCursor(true);
    PlayerController->SetPause(true);

    UE_LOG(LogTemp, Log, TEXT("Start menu open. Choose Start, Settings, or Quit."));
}

void ACreatureCharacter::UpdateGrassResponse()
{
    UMaterialParameterCollection* Collection = LoadObject<UMaterialParameterCollection>(
        nullptr,
        TEXT("/Game/GameProject/L_test/Materials/MPC_Player.MPC_Player")
    );
    if (!Collection)
    {
        return;
    }

    const FVector Velocity = GetVelocity();
    const FVector FlatVelocity(Velocity.X, Velocity.Y, 0.0f);
    const float Amount = FMath::Clamp(FlatVelocity.Size() / 500.0f, 0.0f, 1.0f);
    const FVector Direction = FlatVelocity.GetSafeNormal();
    const FVector Location = GetActorLocation();

    UKismetMaterialLibrary::SetVectorParameterValue(
        this, Collection, TEXT("PlayerPosition"), FLinearColor(Location.X, Location.Y, Location.Z, 1.0f));
    UKismetMaterialLibrary::SetVectorParameterValue(
        this, Collection, TEXT("MoveDirection"), FLinearColor(Direction.X, Direction.Y, 0.0f, 0.0f));
    UKismetMaterialLibrary::SetScalarParameterValue(this, Collection, TEXT("MoveAmount"), Amount);
}

void ACreatureCharacter::DismissStartMenu()
{
    bAwaitingStart = false;

    if (StartMenu)
    {
        StartMenu->ShowPlaying();
    }

    CloseLevelMenu();

    UE_LOG(LogTemp, Log, TEXT("Game started."));

    UGameplayStatics::OpenLevel(
        this,
        FName(TEXT("/Game/GameProject/Levels/Underground"))
    );
}

void ACreatureCharacter::OpenLevelMenu()
{
    if (bAwaitingStart || !StartMenu)
    {
        return;
    }

    StartMenu->ShowLevelCard();

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(StartMenu->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PlayerController->SetInputMode(InputMode);
        PlayerController->SetShowMouseCursor(true);
        PlayerController->SetPause(true);
    }
}

void ACreatureCharacter::CloseLevelMenu()
{
    if (StartMenu)
    {
        StartMenu->ShowPlaying();
    }

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        PlayerController->SetPause(false);
        PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->SetShowMouseCursor(false);
    }
}

void ACreatureCharacter::HandleEscape()
{
    if (bAwaitingStart || !StartMenu)
    {
        return;
    }

    if (StartMenu->IsCardOpen())
    {
        CloseLevelMenu();
    }
    else
    {
        OpenLevelMenu();
    }
}

void ACreatureCharacter::SetLookSensitivity(float NewSensitivity)
{
    MouseSensitivity = FMath::Clamp(NewSensitivity, 0.2f, 3.0f);
}

float ACreatureCharacter::GetLookSensitivity() const
{
    return MouseSensitivity;
}

void ACreatureCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();

    // Runs after this pawn is possessed by the local player.
    AddDefaultMappingContext();
    EnsureCaveContrast();
    ShowStartMenu();
}

// Called to bind functionality to input
void ACreatureCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent
)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindKey(
        EKeys::Escape,
        IE_Pressed,
        this,
        &ACreatureCharacter::HandleEscape
    );

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