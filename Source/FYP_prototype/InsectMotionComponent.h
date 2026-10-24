#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InsectMotionComponent.generated.h"

class UAudioComponent;
class USoundAttenuation;
class USoundBase;

UENUM(BlueprintType)
enum class EInsectSpecies : uint8
{
	DragonflyNymph,
	MayflyNymph,
	CaddisflyCased,
	DivingBeetleLarva,
	MosquitoLarva,
	WaterStrider,
	Backswimmer
};

UENUM(BlueprintType)
enum class EInsectActivity : uint8
{
	Cruise,
	Dash,
	Dive,
	Recover
};

/**
 * Species movement for the seven stream insects.
 * The species is read from the owner name or static mesh when it is not set by hand.
 * With bSurroundPlayer, each species holds its own slot on a slow ring around the player,
 * so its voice always reaches the listener from a distinct direction.
 */
UCLASS(ClassGroup = (Creature), Blueprintable, meta = (BlueprintSpawnableComponent))
class FYP_PROTOTYPE_API UInsectMotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInsectMotionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect")
	EInsectSpecies Species = EInsectSpecies::MayflyNymph;

	/** Fraction of incoming damage absorbed by the caddisfly case. Other species absorb none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float ShieldMitigation = 0.65f;

	UFUNCTION(BlueprintCallable, Category = "Insect")
	float MitigateDamage(float IncomingDamage) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Insect")
	EInsectActivity Activity = EInsectActivity::Cruise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect|Surround")
	bool bSurroundPlayer = true;

	/** Distance from the player to this species' slot. 0 picks a per-species default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect|Surround", meta = (ClampMin = "0.0"))
	float OrbitRadius = 0.0f;

	/** Degrees per second the whole ring turns around the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect|Surround")
	float OrbitSpeed = 9.0f;

	/** How far the insect may wander from its slot while surrounding the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect|Surround", meta = (ClampMin = "10.0"))
	float LeashRadius = 70.0f;

	/**
	 * Looping voice for this species. When empty, /Game/GameProject/Audio/Insects/SFX_<Species>
	 * is used if it exists, e.g. SFX_DragonflyNymph.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect|Sound")
	TObjectPtr<USoundBase> SpeciesSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insect|Sound")
	TObjectPtr<USoundAttenuation> SoundAttenuation;

	UFUNCTION(BlueprintPure, Category = "Insect|Sound")
	UAudioComponent* GetVoice() const { return Voice; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ResolveSpecies();
	void ResolveWaterColumn();
	void ResolveVoice();
	void FollowPlayer(float DeltaTime);
	void TickBackswimmer(float DeltaTime);
	void TickCaddisfly(float DeltaTime);
	void TickDivingBeetle(float DeltaTime);
	void TickDragonfly(float DeltaTime);
	void TickMayfly(float DeltaTime);
	void TickMosquito(float DeltaTime);
	void TickWaterStrider(float DeltaTime);
	void Move(const FVector& Velocity, float DeltaTime, bool bLockToSurface, bool bLockToBed);
	void Face(const FVector& Direction, float RollDegrees);

	UPROPERTY()
	TObjectPtr<UAudioComponent> Voice;

	FVector Home = FVector::ZeroVector;
	FVector Heading = FVector::ForwardVector;
	FVector Velocity = FVector::ZeroVector;
	float SurfaceZ = 0.0f;
	float BedZ = 0.0f;
	float Phase = 0.0f;
	float StateTime = 0.0f;
	float Cooldown = 0.0f;
	float Clock = 0.0f;
	float SlotAngle = 0.0f;
	float SlotDistance = 0.0f;
	float SlotHeight = 8.0f;
	float OrbitAngle = 0.0f;
	bool bSurfaceKnown = false;
	bool bAnchored = false;
};
