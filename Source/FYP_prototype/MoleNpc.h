#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoleNpc.generated.h"

class UCaveUserWidget;
class UPointLightComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class FYP_PROTOTYPE_API AMoleNpc : public AActor
{
	GENERATED_BODY()

public:
	AMoleNpc();

	/** Show the next spoken line and step a little further from the camera. The last line sends the mole away. */
	void PresentLine(const FString& Line, bool bFinal);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (ClampMin = "1.0"))
	float StepDistance = 55.0f;

	/** Local yaw of the body meshes. 180 turns the snout from the tail-side toward the camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole")
	float MeshYaw = 180.0f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Mole")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Mole")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "Mole")
	TObjectPtr<UPointLightComponent> Glow;

private:
	void BuildBody();
	void StandInFrontOfCamera();
	void StepAwayFromCamera();
	void Vanish();

	bool bPlaced = false;
	bool bGone = false;
};
