#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CreatureCharacter.generated.h"

class UCaveUserWidget;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class FYP_PROTOTYPE_API ACreatureCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACreatureCharacter();

    void DismissStartMenu();

    void CloseLevelMenu();

    void SetLookSensitivity(float NewSensitivity);

    float GetLookSensitivity() const;

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaSeconds) override;

    virtual void PawnClientRestart() override;

    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent
    ) override;

    void ResolveInputAssets();

    void AddDefaultMappingContext();

    void ApplyStance();

    void ShowStartMenu();

    void EnsureCaveContrast();

    void HideManholePlaceholders();

    void UpdateGrassResponse();

    void ApplyDirectControls();

    void OpenLevelMenu();

    void HandleEscape();

    bool bBurrowTransitionStarted = false;

    bool bStartMenuShown = false;

    bool bAwaitingStart = false;

    bool bSurfaceSpawnCaptured = false;

    bool bManholeLocated = false;

    FVector SurfaceSpawn = FVector::ZeroVector;

    FVector ManholeLocation = FVector::ZeroVector;

    UPROPERTY()
    TObjectPtr<UCaveUserWidget> StartMenu;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Creature|Input"
    )
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Creature|Input"
    )
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Creature|Input"
    )
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Creature|Input"
    )
    float MouseSensitivity = 1.0f;

private:
    void Move(const FInputActionValue& Value);

    void Look(const FInputActionValue& Value);
};