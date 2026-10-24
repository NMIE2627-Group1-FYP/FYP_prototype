#include "CaveContrastComponent.h"

#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

UCaveContrastComponent::UCaveContrastComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCaveContrastComponent::NotifyNarrativeComplete()
{
	bNarrativeComplete = true;
	NarrativeProgress = 1.0f;
}

void UCaveContrastComponent::SetNarrativeProgress(float Progress)
{
	NarrativeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
}

void UCaveContrastComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
	{
		PlayerController->ConsoleCommand(TEXT("DisableAllScreenMessages"));
	}

	Collection = LoadObject<UMaterialParameterCollection>(
		nullptr,
		TEXT("/Game/GameProject/UI/MPC_UI.MPC_UI")
	);

	const UWorld* World = GetWorld();
	bUnderground = World && World->GetMapName().Contains(TEXT("Underground"));
	BlendAlpha = bUnderground ? 0.0f : 1.0f;

	if (bUnderground)
	{
		EnsurePostProcess();
		ApplyExposure(0.0f);
	}

	UpdateCollection(0.0f);
}

void UCaveContrastComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;

	if (bUnderground && Pawn)
	{
		if (!bHasOrigin)
		{
			BoundaryOrigin = Pawn->GetActorLocation();
			bHasOrigin = true;
		}

		FVector Location = Pawn->GetActorLocation();
		const FVector Offset = Location - BoundaryOrigin;
		const bool bOutsideSquare =
			FMath::Max(FMath::Abs(Offset.X), FMath::Abs(Offset.Y)) > BoundaryHalfExtent;

		if (bOutsideSquare && !bNarrativeComplete && bConfineUntilNarrativeComplete)
		{
			Location.X = BoundaryOrigin.X + FMath::Clamp(Offset.X, -BoundaryHalfExtent, BoundaryHalfExtent);
			Location.Y = BoundaryOrigin.Y + FMath::Clamp(Offset.Y, -BoundaryHalfExtent, BoundaryHalfExtent);
			Pawn->SetActorLocation(Location, true);
		}

		bRevealLighting = bRevealLighting || (bNarrativeComplete && bOutsideSquare);
	}

	float TargetAlpha = 1.0f;
	if (bUnderground && !bRevealLighting)
	{
		TargetAlpha = NarrativeProgress * NarrativeCeiling;
	}
	const float BlendSpeed = bRevealLighting ? 0.6f : 1.2f;
	BlendAlpha = FMath::FInterpTo(BlendAlpha, TargetAlpha, DeltaTime, BlendSpeed);

	if (bUnderground)
	{
		ApplyExposure(BlendAlpha);
	}

	UpdateCollection(DeltaTime);
}

void UCaveContrastComponent::EnsurePostProcess()
{
	AActor* Owner = GetOwner();
	if (!Owner || PostProcess)
	{
		return;
	}

	PostProcess = NewObject<UPostProcessComponent>(Owner, TEXT("CaveArrivalPostProcess"));
	PostProcess->bUnbound = true;
	PostProcess->Priority = 1000.0f;
	PostProcess->BlendWeight = 1.0f;
	PostProcess->RegisterComponent();
}

void UCaveContrastComponent::ApplyExposure(float Alpha) const
{
	if (!PostProcess)
	{
		return;
	}

	FPostProcessSettings& Settings = PostProcess->Settings;
	const float Shade = FMath::Clamp(Alpha, 0.0f, 1.0f);

	// Tint at 0 is true black (Lux 0). The UI is drawn after post-process, so the subtitles stay lit.
	Settings.bOverride_SceneColorTint = true;
	Settings.SceneColorTint = FLinearColor(Shade, Shade, Shade);

	Settings.bOverride_AutoExposureMethod = true;
	Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
	Settings.AutoExposureApplyPhysicalCameraExposure = false;
	Settings.bOverride_AutoExposureBias = true;
	Settings.AutoExposureBias = FMath::Lerp(DarkExposureBias, LitExposureBias, Shade);

	Settings.bOverride_BloomIntensity = true;
	Settings.BloomIntensity = FMath::Lerp(0.05f, 0.45f, Shade);

	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = FMath::Lerp(1.0f, 0.35f, Shade);

	Settings.bOverride_IndirectLightingIntensity = true;
	Settings.IndirectLightingIntensity = Shade;

	Settings.bOverride_AmbientCubemapIntensity = true;
	Settings.AmbientCubemapIntensity = 0.0f;
}

void UCaveContrastComponent::UpdateCollection(float DeltaTime)
{
	if (!Collection)
	{
		return;
	}

	const float Brightness = FMath::Clamp(BlendAlpha, 0.0f, 1.0f);
	// M_UI_EmissiveText multiplies the ink colour by this, so below 1 the letters turn grey in daylight.
	const float TargetEmissive = FMath::Lerp(1.8f, 1.0f, Brightness);
	const float TargetShadow = FMath::Lerp(0.2f, 1.0f, Brightness);
	const float TargetVignette = FMath::Lerp(0.15f, 0.85f, Brightness);
	const float TargetBlur = FMath::Lerp(0.05f, 0.55f, Brightness);
	const float InterpSpeed = DeltaTime > 0.0f ? 1.5f : 0.0f;

	if (InterpSpeed > 0.0f)
	{
		EmissiveIntensity = FMath::FInterpTo(EmissiveIntensity, TargetEmissive, DeltaTime, InterpSpeed);
		ShadowStrength = FMath::FInterpTo(ShadowStrength, TargetShadow, DeltaTime, InterpSpeed);
		VignetteStrength = FMath::FInterpTo(VignetteStrength, TargetVignette, DeltaTime, InterpSpeed);
		BlurAmount = FMath::FInterpTo(BlurAmount, TargetBlur, DeltaTime, InterpSpeed);
	}
	else
	{
		EmissiveIntensity = TargetEmissive;
		ShadowStrength = TargetShadow;
		VignetteStrength = TargetVignette;
		BlurAmount = TargetBlur;
	}

	UKismetMaterialLibrary::SetVectorParameterValue(
		this,
		Collection,
		TEXT("EmissiveColor"),
		FLinearColor(0.898f, 0.867f, 0.796f, 1.0f));
	UKismetMaterialLibrary::SetScalarParameterValue(this, Collection, TEXT("EmissiveIntensity"), EmissiveIntensity);
	UKismetMaterialLibrary::SetScalarParameterValue(this, Collection, TEXT("ShadowStrength"), ShadowStrength);
	UKismetMaterialLibrary::SetScalarParameterValue(this, Collection, TEXT("VignetteStrength"), VignetteStrength);
	UKismetMaterialLibrary::SetScalarParameterValue(this, Collection, TEXT("BlurAmount"), BlurAmount);
}
