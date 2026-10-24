#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CaveContrastComponent.generated.h"

class UMaterialParameterCollection;
class UPostProcessComponent;

/**
 * Reads the underground arrival blend and writes MPC_UI so text stays readable.
 * Dark scenes raise emissive and lighten the shadow. Bright scenes do the opposite.
 * The arrival square around the spawn acts as the trigger volume into Level 1 lighting.
 */
UCLASS(ClassGroup = (Cave), meta = (BlueprintSpawnableComponent))
class FYP_PROTOTYPE_API UCaveContrastComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCaveContrastComponent();

	void NotifyNarrativeComplete();

	/** 0 at the first step in the dark, 1 when the last line is on screen. */
	void SetNarrativeProgress(float Progress);

	/** Half width of the arrival square, centred on the spawn. */
	UPROPERTY(EditAnywhere, Category = "Cave")
	float BoundaryHalfExtent = 600.0f;

	/** Keep the mole inside the arrival square until the last line has been shown. */
	UPROPERTY(EditAnywhere, Category = "Cave")
	bool bConfineUntilNarrativeComplete = false;

	/** Scene brightness reached by the last line, before the boundary is crossed. */
	UPROPERTY(EditAnywhere, Category = "Cave", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NarrativeCeiling = 0.55f;

	/** Manual exposure bias while the burrow is dark, and once Level 1 lighting is reached. */
	UPROPERTY(EditAnywhere, Category = "Cave")
	float DarkExposureBias = -2.0f;

	UPROPERTY(EditAnywhere, Category = "Cave")
	float LitExposureBias = 0.2f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void EnsurePostProcess();
	void ApplyExposure(float Alpha) const;
	void UpdateCollection(float DeltaTime);

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> Collection;

	UPROPERTY()
	TObjectPtr<UPostProcessComponent> PostProcess;

	FVector BoundaryOrigin = FVector::ZeroVector;

	bool bHasOrigin = false;

	bool bUnderground = false;

	bool bNarrativeComplete = false;

	bool bRevealLighting = false;

	float NarrativeProgress = 0.0f;

	float BlendAlpha = 0.0f;

	float EmissiveIntensity = 2.2f;

	float ShadowStrength = 0.2f;

	float VignetteStrength = 0.15f;

	float BlurAmount = 0.0f;
};
