#include "StoryProgressComponent.h"

#include "CaveContrastComponent.h"
#include "CaveUserWidget.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "MoleNpc.h"
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
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "StoryLineRow.h"
#include "TimerManager.h"

UStoryProgressComponent::UStoryProgressComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DialogLines = {
		NSLOCTEXT("Story", "Line1", "You wake in the dark with an empty, aching stomach."),
		NSLOCTEXT("Story", "Line2", "The den's food store is bare."),
		NSLOCTEXT("Story", "Line3", "Behind you there is faint squeaking and slow breathing. The family depends on you."),
		NSLOCTEXT("Story", "Line4", "The floor hums under your feet. Find direction by hearing, and judge the ground by stepping."),
		NSLOCTEXT("Story", "Line5", "There is one purpose: find food and bring it home."),
	};
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
	for (const FText& Line : DialogLines)
	{
		if (!Line.IsEmpty())
		{
			LineTexts.Add(Line.ToString());
		}
	}
	if (LineTexts.IsEmpty() && Lines)
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

	SkyLights.Reset();
	SkyIntensities.Reset();
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		ASkyLight* SkyActor = *It;
		if (!SkyActor)
		{
			continue;
		}

		float Intensity = 1.0f;
		if (USkyLightComponent* Sky = SkyActor->GetLightComponent())
		{
			Intensity = Sky->Intensity;
			Sky->SetVisibility(true);
			Sky->SetIntensity(FMath::Max(Intensity * 0.12f, 0.2f));
		}
		SkyActor->SetActorHiddenInGame(false);
		SkyLights.Add(SkyActor);
		SkyIntensities.Add(Intensity);
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
	for (int32 Index = 0; Index < SkyLights.Num(); ++Index)
	{
		ASkyLight* SkyActor = SkyLights[Index];
		if (!SkyActor)
		{
			continue;
		}

		SkyActor->SetActorHiddenInGame(false);
		if (USkyLightComponent* Sky = SkyActor->GetLightComponent())
		{
			Sky->SetVisibility(true);
			const float Saved = SkyIntensities.IsValidIndex(Index) ? SkyIntensities[Index] : 1.0f;
			Sky->SetIntensity(FMath::Max(Saved * 0.12f, 0.2f));
		}
	}

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
			Component->SetFogDensity(Density * 0.35f);
		}
	}

	// The burrow post-process was the line filter. It stays off so the dark read comes from the lights.
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
	return FirstLineDistance + LineIndex * LineStepDistance;
}

void UStoryProgressComponent::SampleDistance()
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	const bool bForwardDown = PlayerController && (
		PlayerController->IsInputKeyDown(EKeys::W) ||
		PlayerController->IsInputKeyDown(EKeys::Up));
	const bool bPressed = bForwardDown && !bForwardHeld;
	bForwardHeld = bForwardDown;
	if (!bPressed)
	{
		return;
	}

	if (bStoryFinished)
	{
		if (Dialogue && !bDialogueDismissed)
		{
			Dialogue->RemoveFromParent();
			Dialogue = nullptr;
			bDialogueDismissed = true;
		}
		return;
	}

	if (!LineTexts.IsValidIndex(NextLine))
	{
		return;
	}

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
					Dialogue->SetIncludeStart(false);
					Dialogue->AddToViewport(20);
					Dialogue->ShowPlaying();
				}
			}
		}
	}

	const bool bFinal = LineIndex == LineTexts.Num() - 1;
	if (UWorld* World = GetWorld())
	{
		AMoleNpc* Mole = nullptr;
		for (TActorIterator<AMoleNpc> It(World); It; ++It)
		{
			Mole = *It;
			break;
		}
		if (!Mole)
		{
			Mole = World->SpawnActor<AMoleNpc>(AMoleNpc::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		}
		if (Mole && LineTexts.IsValidIndex(LineIndex))
		{
			Mole->PresentLine(LineTexts[LineIndex], bFinal);
		}
	}

	if (Dialogue && LineTexts.IsValidIndex(LineIndex))
	{
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

void UStoryProgressComponent::DismissDialogue()
{
	if (Dialogue)
	{
		Dialogue->RemoveFromParent();
		Dialogue = nullptr;
	}
	bDialogueDismissed = true;
}

void UStoryProgressComponent::FinishStory()
{
	bStoryFinished = true;

	ALight* Above = nullptr;
	float Highest = -100000.0f;
	auto Consider = [&Above, &Highest](ALight* Light)
	{
		if (Light && Light->GetActorLocation().Z > Highest)
		{
			Highest = Light->GetActorLocation().Z;
			Above = Light;
		}
	};
	for (ALight* Light : StoryLights)
	{
		Consider(Light);
	}
	for (ALight* Light : AmbientLights)
	{
		Consider(Light);
	}

	for (int32 Index = 0; Index < StoryLights.Num(); ++Index)
	{
		const bool bKeep = StoryLights[Index] == Above;
		const float Full = TargetIntensity.IsValidIndex(Index) ? TargetIntensity[Index] : 0.0f;
		StartLightFade(StoryLights[Index], bKeep ? FMath::Max(Full * 0.15f, 80.0f) : 0.0f);
	}
	for (int32 Index = 0; Index < AmbientLights.Num(); ++Index)
	{
		const bool bKeep = AmbientLights[Index] == Above;
		const float Full = AmbientTargets.IsValidIndex(Index) ? AmbientTargets[Index] : 0.0f;
		StartLightFade(AmbientLights[Index], bKeep ? FMath::Max(Full * 0.15f, 80.0f) : 0.0f);
	}

	if (UWorld* World = GetWorld())
	{
		const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
		const FVector Origin = Player ? Player->GetActorLocation() : FVector::ZeroVector;
		if (APointLight* Below = World->SpawnActor<APointLight>(Origin + FVector(0.0f, 0.0f, -70.0f), FRotator::ZeroRotator))
		{
			if (UPointLightComponent* Bulb = Cast<UPointLightComponent>(Below->GetLightComponent()))
			{
				Bulb->SetIntensity(70.0f);
				Bulb->SetAttenuationRadius(900.0f);
				Bulb->SetLightColor(FLinearColor(0.35f, 0.42f, 0.55f));
				Bulb->SetCastShadows(false);
			}
		}
	}

	RestoreAtmosphere();
	NotifyContrast(1.0f, true);

	if (UWorld* DialogueWorld = GetWorld())
	{
		DialogueWorld->GetTimerManager().SetTimer(
			DismissTimer, this, &UStoryProgressComponent::DismissDialogue, 2.5f, false);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SampleTimer);
	}
}
