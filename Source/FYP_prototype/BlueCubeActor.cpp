#include "BlueCubeActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABlueCubeActor::ABlueCubeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
    SetRootComponent(MeshComponent);
    MeshComponent->SetMobility(EComponentMobility::Static);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(
        TEXT("/Engine/BasicShapes/Cube.Cube")
    );
    if (CubeMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterialAsset(
        TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial")
    );

    if (BaseMaterialAsset.Succeeded())
    {
        UMaterialInstanceDynamic* MaterialInstance =
            UMaterialInstanceDynamic::Create(BaseMaterialAsset.Object, this);
        MaterialInstance->SetVectorParameterValue(FName(TEXT("BaseColor")), FLinearColor::Blue);
        MaterialInstance->SetScalarParameterValue(FName(TEXT("Metallic")), 0.0f);
        MaterialInstance->SetScalarParameterValue(FName(TEXT("Roughness")), 0.5f);
        MeshComponent->SetMaterial(0, MaterialInstance);
    }
}

void ABlueCubeActor::BeginPlay()
{
    Super::BeginPlay();
}
