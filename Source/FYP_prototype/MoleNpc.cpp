#include "MoleNpc.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AMoleNpc::AMoleNpc()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoleBody"));
	Body->SetupAttachment(SceneRoot);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("MoleGlow"));
	Glow->SetupAttachment(SceneRoot);
	Glow->SetIntensity(0.0f);
	Glow->SetVisibility(false);
}

void AMoleNpc::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);

	TArray<UStaticMeshComponent*> Meshes;
	GetComponents<UStaticMeshComponent>(Meshes);
	const bool bAlreadyBuilt = Meshes.Num() > 1;
	if (!bAlreadyBuilt)
	{
		BuildBody();
		GetComponents<UStaticMeshComponent>(Meshes);
	}

	const FRotator Facing(0.0f, MeshYaw, 0.0f);
	for (UStaticMeshComponent* Mesh : Meshes)
	{
		if (Mesh)
		{
			Mesh->SetRelativeRotation(Facing);
		}
	}

	StandInFrontOfCamera();
}

void AMoleNpc::BuildBody()
{
	if (!Body)
	{
		return;
	}

	struct FPiece
	{
		const TCHAR* Path;
		FVector Offset;
		FVector Scale;
	};

	// The four meshes share one export pivot. Extra offsets pulled the claws off the body.
	const FPiece Pieces[] = {
		{ TEXT("/Game/GameProject/Mesh/SM_MoleSkin.SM_MoleSkin"), FVector::ZeroVector, FVector(0.1f) },
		{ TEXT("/Game/GameProject/Mesh/SM_MoleFur.SM_MoleFur"), FVector::ZeroVector, FVector(0.1f) },
		{ TEXT("/Game/GameProject/Mesh/SM_MoleEyes.SM_MoleEyes"), FVector::ZeroVector, FVector(0.1f) },
		{ TEXT("/Game/GameProject/Mesh/SM_MoleClaws.SM_MoleClaws"), FVector::ZeroVector, FVector(0.1f) },
	};

	// 288 left the tail toward the camera. 108 turns the snout around to face the player.
	Body->SetRelativeRotation(FRotator(0.0f, MeshYaw, 0.0f));
	Body->SetRelativeScale3D(FVector(0.1f));

	UStaticMesh* Skin = LoadObject<UStaticMesh>(nullptr, Pieces[0].Path);
	if (Skin)
	{
		Body->SetStaticMesh(Skin);
	}

	for (int32 Index = 1; Index < UE_ARRAY_COUNT(Pieces); ++Index)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Pieces[Index].Path);
		if (!Mesh)
		{
			continue;
		}

		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
		Part->SetStaticMesh(Mesh);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetupAttachment(SceneRoot);
		Part->SetRelativeLocation(FVector::ZeroVector);
		Part->SetRelativeRotation(Body->GetRelativeRotation());
		Part->SetRelativeScale3D(FVector(0.1f));
		Part->RegisterComponent();
	}
}

void AMoleNpc::StandInFrontOfCamera()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	FVector CameraLocation = FVector::ZeroVector;
	FRotator CameraRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector Forward = CameraRotation.Vector().GetSafeNormal2D();
	SetActorLocation(CameraLocation + Forward * 180.0f + FVector(0.0f, 0.0f, -70.0f));
	FRotator Face = (-Forward).Rotation();
	Face.Pitch = 0.0f;
	Face.Roll = 0.0f;
	SetActorRotation(Face);
	bPlaced = true;
}

void AMoleNpc::StepAwayFromCamera()
{
	if (!bPlaced)
	{
		StandInFrontOfCamera();
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	FVector CameraLocation = FVector::ZeroVector;
	FRotator CameraRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	const FVector Forward = CameraRotation.Vector().GetSafeNormal2D();
	SetActorLocation(GetActorLocation() + Forward * StepDistance);
	FRotator Face = (-Forward).Rotation();
	Face.Pitch = 0.0f;
	Face.Roll = 0.0f;
	SetActorRotation(Face);
}

void AMoleNpc::PresentLine(const FString& Line, bool bFinal)
{
	(void)Line;
	if (bGone)
	{
		return;
	}

	StepAwayFromCamera();
	if (bFinal)
	{
		Vanish();
	}
}

void AMoleNpc::Vanish()
{
	if (bGone)
	{
		return;
	}
	bGone = true;

	if (UNiagaraSystem* Dissolve = LoadObject<UNiagaraSystem>(
		nullptr, TEXT("/Game/GameProject/L_test/FX/NS_ArrowGlow.NS_ArrowGlow")))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Dissolve, GetActorLocation(), GetActorRotation());
	}

	if (Glow)
	{
		Glow->SetIntensity(0.0f);
		Glow->SetVisibility(false);
	}
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}
