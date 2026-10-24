#include "InsectMotionComponent.h"

#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

namespace InsectMotionPrivate
{
	// Ring distance per species, in species order. Surface skaters sit wider, case-bearers closer.
	constexpr float SlotDistances[] = { 150.0f, 190.0f, 125.0f, 215.0f, 170.0f, 245.0f, 205.0f };
	// Height above the player's location so the voice stays around a mole-scale listener.
	constexpr float SlotHeights[] = { 6.0f, 10.0f, 4.0f, 8.0f, 14.0f, 16.0f, 12.0f };
	constexpr int32 SpeciesCount = UE_ARRAY_COUNT(SlotDistances);
}

UInsectMotionComponent::UInsectMotionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

float UInsectMotionComponent::MitigateDamage(float IncomingDamage) const
{
	if (Species != EInsectSpecies::CaddisflyCased)
	{
		return IncomingDamage;
	}
	return IncomingDamage * (1.0f - FMath::Clamp(ShieldMitigation, 0.0f, 0.95f));
}

void UInsectMotionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(true);

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	Owner->SetActorTickEnabled(true);
	if (USceneComponent* Root = Owner->GetRootComponent())
	{
		Root->SetMobility(EComponentMobility::Movable);
	}
	if (bSurroundPlayer)
	{
		Owner->SetActorEnableCollision(false);
	}

	Home = Owner->GetActorLocation();
	Heading = Owner->GetActorForwardVector().GetSafeNormal2D();
	if (Heading.IsNearlyZero())
	{
		Heading = FVector::ForwardVector;
	}

	ResolveSpecies();
	ResolveWaterColumn();
	ResolveVoice();

	const int32 SpeciesIndex = FMath::Clamp(static_cast<int32>(Species), 0, InsectMotionPrivate::SpeciesCount - 1);
	SlotAngle = SpeciesIndex * (360.0f / InsectMotionPrivate::SpeciesCount);
	SlotDistance = OrbitRadius > 0.0f ? OrbitRadius : InsectMotionPrivate::SlotDistances[SpeciesIndex];
	SlotHeight = InsectMotionPrivate::SlotHeights[SpeciesIndex];

	if (Species == EInsectSpecies::Backswimmer)
	{
		Face(Heading, 180.0f);
	}
}

void UInsectMotionComponent::ResolveVoice()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FString SpeciesName = StaticEnum<EInsectSpecies>()->GetNameStringByValue(static_cast<int64>(Species));
	USoundBase* Sound = SpeciesSound;
	if (!Sound)
	{
		const FString Package = FString::Printf(TEXT("/Game/GameProject/Audio/Insects/SFX_%s"), *SpeciesName);
		if (FPackageName::DoesPackageExist(Package))
		{
			Sound = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("%s.SFX_%s"), *Package, *SpeciesName));
		}
	}

	const FName VoiceName(*FString::Printf(TEXT("MotherVoice_%s"), *SpeciesName));
	if (UAudioComponent* Existing = Owner->FindComponentByClass<UAudioComponent>())
	{
		if (Existing->GetFName().ToString().Contains(TEXT("MotherVoice")))
		{
			Voice = Existing;
		}
	}
	if (!Voice)
	{
		Voice = NewObject<UAudioComponent>(Owner, VoiceName);
		Voice->SetupAttachment(Owner->GetRootComponent());
		Voice->bAutoActivate = false;
		Voice->RegisterComponent();
	}
	if (Sound)
	{
		Voice->SetSound(Sound);
	}
	if (SoundAttenuation)
	{
		Voice->AttenuationSettings = SoundAttenuation;
	}
	else
	{
		Voice->bOverrideAttenuation = true;
		Voice->AttenuationOverrides.bAttenuate = true;
		Voice->AttenuationOverrides.bSpatialize = true;
		Voice->AttenuationOverrides.FalloffDistance = 900.0f;
		Voice->AttenuationOverrides.AttenuationShapeExtents = FVector(60.0f);
	}
	if (Sound)
	{
		Voice->Play(FMath::FRandRange(0.0f, 2.0f));
	}
}

void UInsectMotionComponent::FollowPlayer(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	OrbitAngle = FMath::Fmod(OrbitAngle + OrbitSpeed * DeltaTime, 360.0f);
	const FVector Offset = FRotator(0.0f, SlotAngle + OrbitAngle, 0.0f).Vector() * SlotDistance;
	const FVector PlayerLocation = Player->GetActorLocation();
	const FVector Slot(
		PlayerLocation.X + Offset.X,
		PlayerLocation.Y + Offset.Y,
		PlayerLocation.Z + SlotHeight);

	FVector Location = Owner->GetActorLocation();
	const float SnapSpeed = bAnchored ? 6.0f : 20.0f;
	Location = FMath::VInterpTo(Location, Slot, DeltaTime, SnapSpeed);
	if (!bAnchored && FVector::Dist2D(Location, Slot) < 8.0f)
	{
		Location = Slot;
		bAnchored = true;
	}

	Home = Slot;
	SurfaceZ = Slot.Z + 6.0f;
	BedZ = Slot.Z - 14.0f;
	Owner->SetActorLocation(Location);
}

void UInsectMotionComponent::ResolveSpecies()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FString Identity = Owner->GetActorNameOrLabel();
	TArray<UStaticMeshComponent*> Meshes;
	Owner->GetComponents<UStaticMeshComponent>(Meshes);
	for (const UStaticMeshComponent* Mesh : Meshes)
	{
		if (Mesh && Mesh->GetStaticMesh())
		{
			Identity += Mesh->GetStaticMesh()->GetName();
		}
	}

	if (Identity.Contains(TEXT("I07")) || Identity.Contains(TEXT("Backswimmer")))
	{
		Species = EInsectSpecies::Backswimmer;
	}
	else if (Identity.Contains(TEXT("I06")) || Identity.Contains(TEXT("Strider")))
	{
		Species = EInsectSpecies::WaterStrider;
	}
	else if (Identity.Contains(TEXT("I05")) || Identity.Contains(TEXT("Mosquito")))
	{
		Species = EInsectSpecies::MosquitoLarva;
	}
	else if (Identity.Contains(TEXT("I04")) || Identity.Contains(TEXT("Diving")))
	{
		Species = EInsectSpecies::DivingBeetleLarva;
	}
	else if (Identity.Contains(TEXT("I03")) || Identity.Contains(TEXT("Caddis")))
	{
		Species = EInsectSpecies::CaddisflyCased;
	}
	else if (Identity.Contains(TEXT("I01")) || Identity.Contains(TEXT("Dragonfly")))
	{
		Species = EInsectSpecies::DragonflyNymph;
	}
	else if (Identity.Contains(TEXT("I02")) || Identity.Contains(TEXT("Mayfly")))
	{
		Species = EInsectSpecies::MayflyNymph;
	}
}

void UInsectMotionComponent::ResolveWaterColumn()
{
	SurfaceZ = Home.Z;
	BedZ = Home.Z - 40.0f;
	bSurfaceKnown = false;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InsectWater), false, GetOwner());
	FHitResult Hit;
	const FVector Start = Home + FVector(0.0f, 0.0f, 400.0f);
	const FVector End = Home - FVector(0.0f, 0.0f, 800.0f);
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
	{
		BedZ = Hit.ImpactPoint.Z;
		const FString HitName = Hit.GetActor() ? Hit.GetActor()->GetActorNameOrLabel() : FString();
		if (HitName.Contains(TEXT("Water")) || HitName.Contains(TEXT("Stream")))
		{
			SurfaceZ = Hit.ImpactPoint.Z;
			bSurfaceKnown = true;
		}
	}

	if (!bSurfaceKnown)
	{
		const bool bSurfaceInsect =
			Species == EInsectSpecies::WaterStrider ||
			Species == EInsectSpecies::MosquitoLarva ||
			Species == EInsectSpecies::Backswimmer;
		// Without water the trace hit dry ground, so no species may sink below it.
		SurfaceZ = bSurfaceInsect ? Home.Z : Home.Z + 60.0f;
		BedZ = Home.Z;
	}
}

void UInsectMotionComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!GetOwner())
	{
		return;
	}

	Clock += DeltaTime;
	if (bSurroundPlayer)
	{
		FollowPlayer(DeltaTime);
	}

	switch (Species)
	{
	case EInsectSpecies::Backswimmer: TickBackswimmer(DeltaTime); break;
	case EInsectSpecies::CaddisflyCased: TickCaddisfly(DeltaTime); break;
	case EInsectSpecies::DivingBeetleLarva: TickDivingBeetle(DeltaTime); break;
	case EInsectSpecies::DragonflyNymph: TickDragonfly(DeltaTime); break;
	case EInsectSpecies::MayflyNymph: TickMayfly(DeltaTime); break;
	case EInsectSpecies::MosquitoLarva: TickMosquito(DeltaTime); break;
	case EInsectSpecies::WaterStrider: TickWaterStrider(DeltaTime); break;
	default: break;
	}
}

void UInsectMotionComponent::Move(const FVector& Desired, float DeltaTime, bool bLockToSurface, bool bLockToBed)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const float SurroundScale = bSurroundPlayer ? 0.22f : 1.0f;
	const float Response = (Species == EInsectSpecies::WaterStrider) ? 1.5f : 6.0f;
	Velocity = FMath::VInterpTo(Velocity, Desired * SurroundScale, DeltaTime, Response);

	FVector Location = Owner->GetActorLocation() + Velocity * DeltaTime;
	const float Leash = bSurroundPlayer ? LeashRadius : 420.0f;
	const FVector Flat = FVector(Location.X, Location.Y, Home.Z);
	if (FVector::Dist2D(Flat, Home) > Leash)
	{
		Heading = (Home - Location).GetSafeNormal2D();
		const FVector Pulled = Home + (Location - Home).GetClampedToMaxSize2D(Leash);
		Location.X = Pulled.X;
		Location.Y = Pulled.Y;
	}

	if (bLockToSurface)
	{
		Location.Z = SurfaceZ;
	}
	else if (bLockToBed)
	{
		Location.Z = BedZ + 4.0f;
	}
	else
	{
		Location.Z = FMath::Clamp(Location.Z, BedZ + 2.0f, SurfaceZ - 1.0f);
	}

	Owner->SetActorLocation(Location);
}

void UInsectMotionComponent::Face(const FVector& Direction, float RollDegrees)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}
	const FVector Flat = Direction.GetSafeNormal2D();
	if (Flat.IsNearlyZero())
	{
		return;
	}
	const float Pitch = FMath::RadiansToDegrees(FMath::Atan2(Direction.Z, Flat.Size2D()));
	Owner->SetActorRotation(FRotator(Pitch, Flat.Rotation().Yaw, RollDegrees));
}

void UInsectMotionComponent::TickBackswimmer(float DeltaTime)
{
	StateTime += DeltaTime;
	const bool bRowing = StateTime < 0.32f;
	Activity = bRowing ? EInsectActivity::Dash : EInsectActivity::Cruise;
	if (StateTime > 1.35f)
	{
		StateTime = 0.0f;
		const FVector Lateral(Heading.Y, -Heading.X, 0.0f);
		Heading = (Heading + Lateral * FMath::Sin(Clock * 0.7f)).GetSafeNormal2D();
	}

	// Upside-down rowing steers opposite the oar side, so the body faces away from travel.
	const FVector Travel = Heading * (bRowing ? 90.0f : 18.0f);
	Move(FVector(Travel.X, Travel.Y, 0.0f), DeltaTime, false, false);
	if (AActor* Owner = GetOwner())
	{
		FVector Location = Owner->GetActorLocation();
		Location.Z = FMath::Max(SurfaceZ - 10.0f, BedZ + 2.0f);
		Owner->SetActorLocation(Location);
	}
	Face(-Heading, 180.0f);
}

void UInsectMotionComponent::TickCaddisfly(float DeltaTime)
{
	Activity = EInsectActivity::Cruise;
	StateTime += DeltaTime;
	if (StateTime > 4.5f)
	{
		StateTime = 0.0f;
		Heading = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal2D();
	}
	Move(Heading * 12.0f, DeltaTime, false, true);
	Face(Heading, FMath::Sin(Clock * 2.0f) * 4.0f);
}

void UInsectMotionComponent::TickDivingBeetle(float DeltaTime)
{
	StateTime += DeltaTime;
	if (Activity != EInsectActivity::Dash && StateTime > 3.4f)
	{
		Activity = EInsectActivity::Dash;
		StateTime = 0.0f;
		Heading = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal2D();
	}
	else if (Activity == EInsectActivity::Dash && StateTime > 0.55f)
	{
		Activity = EInsectActivity::Cruise;
		StateTime = 0.0f;
	}

	const float Speed = Activity == EInsectActivity::Dash ? 160.0f : 22.0f;
	const float Paddle = Activity == EInsectActivity::Dash ? FMath::Sin(Clock * 28.0f) * 18.0f : 0.0f;
	Move(Heading * Speed, DeltaTime, false, Activity != EInsectActivity::Dash);
	Face(Heading, Paddle);
}

void UInsectMotionComponent::TickDragonfly(float DeltaTime)
{
	Cooldown -= DeltaTime;
	if (Activity != EInsectActivity::Dash)
	{
		Activity = EInsectActivity::Cruise;
		if (Cooldown <= 0.0f)
		{
			Activity = EInsectActivity::Dash;
			StateTime = 0.0f;
			Cooldown = 4.6f;
			Heading = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal2D();
		}
	}

	if (Activity == EInsectActivity::Dash)
	{
		StateTime += DeltaTime;
		Move(Heading * 220.0f, DeltaTime, false, true);
		if (StateTime > 0.28f)
		{
			Activity = EInsectActivity::Recover;
			StateTime = 0.0f;
		}
	}
	else
	{
		Move(Heading * 16.0f, DeltaTime, false, true);
	}
	Face(Heading, 0.0f);
}

void UInsectMotionComponent::TickMayfly(float DeltaTime)
{
	Activity = EInsectActivity::Cruise;
	const float Turn = FMath::Sin(Clock * 0.55f) * 0.8f;
	const FVector Lateral(-Heading.Y, Heading.X, 0.0f);
	Heading = (Heading + Lateral * Turn * DeltaTime).GetSafeNormal2D();

	const FVector Wave = Lateral * FMath::Sin(Clock * 3.2f) * 34.0f;
	const float Depth = FMath::Lerp(BedZ + 8.0f, SurfaceZ - 12.0f, 0.5f + 0.5f * FMath::Sin(Clock * 1.3f));
	Move(Heading * 48.0f + Wave, DeltaTime, false, false);
	if (AActor* Owner = GetOwner())
	{
		FVector Location = Owner->GetActorLocation();
		Location.Z = FMath::FInterpTo(Location.Z, Depth, DeltaTime, 2.0f);
		Owner->SetActorLocation(Location);
	}
	Face(Heading + Wave.GetSafeNormal() * 0.2f, FMath::Sin(Clock * 3.2f) * 12.0f);
}

void UInsectMotionComponent::TickMosquito(float DeltaTime)
{
	StateTime += DeltaTime;
	if (Activity == EInsectActivity::Cruise && StateTime > 5.5f)
	{
		Activity = EInsectActivity::Dive;
		StateTime = 0.0f;
	}
	else if (Activity == EInsectActivity::Dive && StateTime > 0.7f)
	{
		Activity = EInsectActivity::Recover;
		StateTime = 0.0f;
	}
	else if (Activity == EInsectActivity::Recover && StateTime > 0.9f)
	{
		Activity = EInsectActivity::Cruise;
		StateTime = 0.0f;
	}

	float TargetZ = SurfaceZ - 2.0f;
	if (Activity == EInsectActivity::Dive)
	{
		const float Jerk = FMath::Fmod(StateTime, 0.16f) < 0.08f ? 1.0f : 0.35f;
		TargetZ = FMath::Lerp(SurfaceZ, BedZ + 6.0f, FMath::Clamp(StateTime / 0.7f, 0.0f, 1.0f) * Jerk);
	}

	if (AActor* Owner = GetOwner())
	{
		FVector Location = Owner->GetActorLocation();
		Location.X = Home.X + FMath::Sin(Clock * 6.0f) * (Activity == EInsectActivity::Dive ? 6.0f : 1.0f);
		Location.Y = Home.Y;
		const float Rise = Activity == EInsectActivity::Dive ? 14.0f : 3.0f;
		Location.Z = FMath::FInterpTo(Location.Z, TargetZ, DeltaTime, Rise);
		Owner->SetActorLocation(Location);
		Owner->SetActorRotation(FRotator(Activity == EInsectActivity::Dive ? -70.0f : 0.0f, 0.0f, 0.0f));
	}
}

void UInsectMotionComponent::TickWaterStrider(float DeltaTime)
{
	Activity = EInsectActivity::Cruise;
	StateTime += DeltaTime;
	if (StateTime > 2.2f)
	{
		StateTime = 0.0f;
		Heading = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal2D();
	}
	Move(Heading * 140.0f, DeltaTime, true, false);
	Face(Heading, 0.0f);
}
