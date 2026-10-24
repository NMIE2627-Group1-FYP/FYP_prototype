#include "StoryProgressComponent.h"

#include "CaveContrastComponent.h"
#include "CaveUserWidget.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DataTable.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/Light.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "StoryLineRow.h"
#include "TimerManager.h"

UStoryProgressComponent::UStoryProgressComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStoryProgressComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World || !World->GetMapName().Contains(TEXT("Underground")))
	{
		return;
	}

	if (!Lines && FPackageName::DoesPackageExist(TEXT("/Game/GameProject/UI/DT_StoryLines")))
	{
		Lines = LoadObject<UDataTable>(nullptr, TEXT("/Game/GameProject/UI/DT_StoryLines.DT_StoryLines"));
	}

	LineTexts.Reset();
	RowThresholds.Reset();
	if (Lines)
	{
		for (const FName& RowName : Lines->GetRowNames())
		{
			if (const FStoryLineRow* Row = Lines->FindRow<FStoryLineRow>(RowName, TEXT("StoryProgress")))
			{
				LineTexts.Add(Row->Text.ToString());
				RowThresholds.Add(Row->DistanceThreshold);
			}
		}
	}
	if (LineTexts.IsEmpty())
	{
		LineTexts = {
			TEXT("You wake in the dark with an empty, aching stomach."),
			TEXT("The den's food store is bare."),
			TEXT("Behind you there is faint squeaking and slow breathing. The family depends on you."),
			TEXT("The floor hums under your feet. Find direction by hearing, and judge the ground by stepping."),
			TEXT("There is one purpose: find food and bring it home."),
		};
	}

	ApplyDarkStart();

	TArray<TPair<TObjectPtr<ALight>, float>> Story;
	for (TActorIterator<ALight> It(World); It; ++It)
	{
		ALight* Light = *It;
		if (!Light)
		{
			continue;
		}

		ULightComponent* Bulb = Light->GetLightComponent();
		const float Target = Bulb ? Bulb->Intensity : 0.0f;
		if (Bulb)
		{
			Bulb->SetMobility(EComponentMobility::Movable);
			Bulb->SetIntensity(0.0f);
			Bulb->SetVisibility(false);
		}
		Light->SetActorHiddenInGame(true);

		if (Light->ActorHasTag(TEXT("StoryLight")))
		{
			Story.Emplace(Light, Target);
		}
		else
		{
			AmbientLights.Add(Light);
			AmbientTargets.Add(Target);
		}
	}

	if (Story.IsEmpty())
	{
		for (int32 Index = 0; Index < AmbientLights.Num(); ++Index)
		{
			Story.Emplace(AmbientLights[Index], AmbientTargets[Index]);
		}
		AmbientLights.Reset();
		AmbientTargets.Reset();
	}

	const FVector Start = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	Story.Sort([&Start](const TPair<TObjectPtr<ALight>, float>& A, const TPair<TObjectPtr<ALight>, float>& B)
	{
		return FVector::DistSquared2D(A.Key->GetActorLocation(), Start) < FVector::DistSquared2D(B.Key->GetActorLocation(), Start);
	});
	for (const TPair<TObjectPtr<ALight>, float>& Entry : Story)
	{
		StoryLights.Add(Entry.Key);
		TargetIntensity.Add(Entry.Value);
	}

	NotifyContrast(0.0f, false);

	World->GetTimerManager().SetTimer(
		SampleTimer,
		this,
		&UStoryProgressComponent::SampleDistance,
		FMath::Max(SampleInterval, 0.01f),
		true);
}

void UStoryProgressComponent::ApplyDarkStart()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		It->SetActorHiddenInGame(true);
		if (USkyLightComponent* Sky = It->GetLightComponent())
		{
			Sky->SetIntensity(0.0f);
			Sky->SetVisibility(false);
		}
	}

	Fogs.Reset();
	FogDensities.Reset();
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		AExponentialHeightFog* Fog = *It;
		if (!Fog)
		{
			continue;
		}

		float Density = 0.02f;
		if (UExponentialHeightFogComponent* Component = Fog->GetComponent())
		{
			Density = Component->FogDensity;
			Component->SetFogDensity(0.0f);
			Component->SetVisibility(false);
		}
		Fog->SetActorHiddenInGame(true);
		Fogs.Add(Fog);
		FogDensities.Add(Density);
	}

	Volumes.Reset();
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* Volume = *It;
		if (!Volume)
		{
			continue;
		}
		Volume->bEnabled = false;
		Volumes.Add(Volume);
	}
}

void UStoryProgressComponent::RestoreAtmosphere()
{
	for (int32 Index = 0; Index < Fogs.Num(); ++Index)
	{
		AExponentialHeightFog* Fog = Fogs[Index];
		if (!Fog)
		{
			continue;
		}

		Fog->SetActorHiddenInGame(false);
		if (UExponentialHeightFogComponent* Component = Fog->GetComponent())
		{
			Component->SetVisibility(true);
			const float Density = FogDensities.IsValidIndex(Index) ? FogDensities[Index] : 0.02f;
			Component->SetFogDensity(Density);
		}
	}

	for (APostProcessVolume* Volume : Volumes)
	{
		if (Volume)
		{
			Volume->bEnabled = true;
		}
	}
}

void UStoryProgressComponent::NotifyContrast(float Progress, bool bComplete) const
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	UCaveContrastComponent* Contrast = PlayerController ? PlayerController->FindComponentByClass<UCaveContrastComponent>() : nullptr;
	if (!Contrast)
	{
		return;
	}

	Contrast->SetNarrativeProgress(Progress);
	if (bComplete)
	{
		Contrast->NotifyNarrativeComplete();
	}
}

float UStoryProgressComponent::ThresholdForLine(int32 LineIndex) const
{
	const float Paced = FirstLineDistance + LineIndex * LineStepDistance;
	return RowThresholds.IsValidIndex(LineIndex) ? FMath::Max(Paced, RowThresholds[LineIndex]) : Paced;
}

void UStoryProgressComponent::SampleDistance()
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (!Owner || !World || bStoryFinished)
	{
		return;
	}

	const FVector Location = Owner->GetActorLocation();
	if (!bHasSample)
	{
		LastLocation = Location;
		bHasSample = true;
		return;
	}

	AccumulatedDistance += FVector::Dist2D(Location, LastLocation);
	LastLocation = Location;

	if (!LineTexts.IsValidIndex(NextLine) || AccumulatedDistance < ThresholdForLine(NextLine))
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (NextLine > 0 && Now - LastRevealTime < MinLineSeconds)
	{
		return;
	}

	// Walking far ahead while a line is still on screen must not bank several lines at once.
	AccumulatedDistance = FMath::Min(AccumulatedDistance, ThresholdForLine(NextLine) + LineStepDistance * 0.5f);
	LastRevealTime = Now;
	RevealLine(NextLine++);
}

void UStoryProgressComponent::RevealLine(int32 LineIndex)
{
	if (!Dialogue)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (UClass* WidgetClass = LoadClass<UCaveUserWidget>(
				nullptr, TEXT("/Game/GameProject/Blueprints/WBP_MoleSpeech.WBP_MoleSpeech_C")))
			{
				Dialogue = CreateWidget<UCaveUserWidget>(PlayerController, WidgetClass);
				if (Dialogue)
				{
					Dialogue->AddToViewport(20);
				}
			}
		}
	}

	const bool bFinal = LineIndex == LineTexts.Num() - 1;
	if (Dialogue && LineTexts.IsValidIndex(LineIndex))
	{
		if (bFinal)
		{
			Dialogue->ShowFinale();
		}
		Dialogue->SetSpeech(LineTexts[LineIndex]);
	}

	if (bFinal)
	{
		FinishStory();
		return;
	}

	if (StoryLights.IsValidIndex(LineIndex))
	{
		const float Full = TargetIntensity.IsValidIndex(LineIndex) ? TargetIntensity[LineIndex] : 0.0f;
		StartLightFade(StoryLights[LineIndex], Full * StepLightFraction);
	}
	NotifyContrast(static_cast<float>(LineIndex + 1) / FMath::Max(LineTexts.Num(), 1), false);
}

void UStoryProgressComponent::StartLightFade(ALight* Light, float Target)
{
	if (!Light)
	{
		return;
	}

	for (FLightFade& Fade : ActiveFades)
	{
		if (Fade.Light.Get() == Light)
		{
			Fade.Target = Target;
			return;
		}
	}

	Light->SetActorHiddenInGame(false);
	float Starting = 0.0f;
	if (ULightComponent* Bulb = Light->GetLightComponent())
	{
		Bulb->SetVisibility(true);
		Starting = Bulb->Intensity;
	}

	FLightFade Fade;
	Fade.Light = Light;
	Fade.Current = Starting;
	Fade.Target = Target;
	ActiveFades.Add(Fade);

	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(FadeTimer))
		{
			World->GetTimerManager().SetTimer(FadeTimer, this, &UStoryProgressComponent::TickLightFades, 0.03f, true);
		}
	}
}

void UStoryProgressComponent::TickLightFades()
{
	const float Step = 0.03f;
	const float Speed = FadeDuration > 0.0f ? 1.0f / FadeDuration : 1.0f;

	for (int32 Index = ActiveFades.Num() - 1; Index >= 0; --Index)
	{
		FLightFade& Fade = ActiveFades[Index];
		ALight* Light = Fade.Light.Get();
		if (!Light)
		{
			ActiveFades.RemoveAt(Index);
			continue;
		}

		Fade.Current = FMath::FInterpTo(Fade.Current, Fade.Target, Step, Speed * 3.0f);
		ULightComponent* Bulb = Light->GetLightComponent();
		if (Bulb)
		{
			Bulb->SetIntensity(Fade.Current);
		}
		if (FMath::IsNearlyEqual(Fade.Current, Fade.Target, 0.5f))
		{
			if (Bulb)
			{
				Bulb->SetIntensity(Fade.Target);
			}
			ActiveFades.RemoveAt(Index);
		}
	}

	if (ActiveFades.Num() == 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FadeTimer);
		}
	}
}

void UStoryProgressComponent::FinishStory()
{
	bStoryFinished = true;
	for (int32 Index = 0; Index < StoryLights.Num(); ++Index)
	{
		const float Full = TargetIntensity.IsValidIndex(Index) ? TargetIntensity[Index] : 0.0f;
		StartLightFade(StoryLights[Index], Full);
	}
	for (int32 Index = 0; Index < AmbientLights.Num(); ++Index)
	{
		const float Full = AmbientTargets.IsValidIndex(Index) ? AmbientTargets[Index] : 0.0f;
		StartLightFade(AmbientLights[Index], Full);
	}

	RestoreAtmosphere();
	NotifyContrast(1.0f, true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SampleTimer);
	}
}
