#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CreatureCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class FYP_PROTOTYPE_API ACreatureCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACreatureCharacter();

protected:
    virtual void BeginPlay() override;

    virtual void PawnClientRestart() override;

    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent
    ) override;

    void ResolveInputAssets();

    void AddDefaultMappingContext();

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