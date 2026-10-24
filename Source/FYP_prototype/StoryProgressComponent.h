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
 * Forward-walk dialogue. Distance is sampled on a short timer only to read position.
 * Strafing and time do not advance the story. Each forward step wakes the next light,
 * and the last line turns every light in the level on.
 */
UCLASS(ClassGroup = (Cave), Blueprintable, meta = (BlueprintSpawnableComponent))
class FYP_PROTOTYPE_API UStoryProgressComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStoryProgressComponent();

	/** Lines spoken on each forward press. Edit these in the Details panel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story|Dialogue", meta = (MultiLine = true))
	TArray<FText> DialogLines;

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

	/** Kept so older assets still load. Lights follow forward steps, not this timer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story", meta = (ClampMin = "0.0"))
	float MinLineSeconds = 0.0f;

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
	void DismissDialogue();
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
	FTimerHandle DismissTimer;

	FVector LastLocation = FVector::ZeroVector;
	bool bHasSample = false;
	int32 NextLine = 0;
	float LastRevealTime = -1000.0f;
	bool bStoryFinished = false;
	bool bForwardHeld = false;
	bool bDialogueDismissed = false;

	UPROPERTY()
	TObjectPtr<UCaveUserWidget> Dialogue;
};
