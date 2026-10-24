#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StoryProgressComponent.generated.h"

class AExponentialHeightFog;
class ALight;
class APostProcessVolume;
class ASkyLight;
class UDataTable;
class UCaveUserWidget;

/**
 * Walk-driven dialogue. Distance is sampled on a 0.1s timer.
 * Every light starts off. Each line wakes the next StoryLight (nearest to the spawn first),
 * and the last line brings every light in the level up to full.
 */
UCLASS(ClassGroup = (Cave), Blueprintable, meta = (BlueprintSpawnableComponent))
class FYP_PROTOTYPE_API UStoryProgressComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStoryProgressComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	TObjectPtr<UDataTable> Lines;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	float SampleInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	float FadeDuration = 1.5f;

	/** Walked distance before the first line. 5 cm in Unreal units. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story", meta = (ClampMin = "0.0"))
	float FirstLineDistance = 5.0f;

	/** Extra walked distance between lines. 5 cm in Unreal units. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story", meta = (ClampMin = "1.0"))
	float LineStepDistance = 5.0f;

	/** A line stays at least this long before walking can replace it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story", meta = (ClampMin = "0.0"))
	float MinLineSeconds = 0.45f;

	/** Share of a story light's full intensity used before the last line. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StepLightFraction = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
	float AccumulatedDistance = 0.0f;

protected:
	virtual void BeginPlay() override;

private:
	void SampleDistance();
	void RevealLine(int32 LineIndex);
	void StartLightFade(ALight* Light, float Target);
	void TickLightFades();
	void FinishStory();
	void ApplyDarkStart();
	void RestoreAtmosphere();
	void NotifyContrast(float Progress, bool bComplete) const;
	float ThresholdForLine(int32 LineIndex) const;

	UPROPERTY()
	TArray<TObjectPtr<ALight>> StoryLights;

	UPROPERTY()
	TArray<TObjectPtr<ALight>> AmbientLights;

	UPROPERTY()
	TArray<TObjectPtr<ASkyLight>> SkyLights;

	UPROPERTY()
	TArray<TObjectPtr<AExponentialHeightFog>> Fogs;

	UPROPERTY()
	TArray<TObjectPtr<APostProcessVolume>> Volumes;

	TArray<float> TargetIntensity;
	TArray<float> AmbientTargets;
	TArray<float> FogDensities;
	TArray<float> SkyIntensities;
	TArray<FString> LineTexts;
	TArray<float> RowThresholds;

	struct FLightFade
	{
		TWeakObjectPtr<ALight> Light;
		float Current = 0.0f;
		float Target = 0.0f;
	};

	TArray<FLightFade> ActiveFades;

	FTimerHandle SampleTimer;
	FTimerHandle FadeTimer;

	FVector LastLocation = FVector::ZeroVector;
	bool bHasSample = false;
	int32 NextLine = 0;
	float LastRevealTime = -1000.0f;
	bool bStoryFinished = false;

	UPROPERTY()
	TObjectPtr<UCaveUserWidget> Dialogue;
};
