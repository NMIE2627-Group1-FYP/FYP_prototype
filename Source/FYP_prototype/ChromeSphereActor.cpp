#include "ChromeSphereActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AChromeSphereActor::AChromeSphereActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
    SetRootComponent(MeshComponent);
    MeshComponent->SetMobility(EComponentMobility::Static);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(
        TEXT("/Engine/BasicShapes/Sphere.Sphere")
    );
    if (SphereMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMeshAsset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterialAsset(
        TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial")
    );

    if (BaseMaterialAsset.Succeeded())
    {
        UMaterialInstanceDynamic* MaterialInstance =
            UMaterialInstanceDynamic::Create(BaseMaterialAsset.Object, this);
        MaterialInstance->SetVectorParameterValue(FName(TEXT("BaseColor")), FLinearColor(0.67f, 0.69f, 0.72f, 1.0f));
        MaterialInstance->SetScalarParameterValue(FName(TEXT("Metallic")), 1.0f);
        MaterialInstance->SetScalarParameterValue(FName(TEXT("Roughness")), 0.12f);
        MeshComponent->SetMaterial(0, MaterialInstance);
    }
}

void AChromeSphereActor::BeginPlay()
{
    Super::BeginPlay();
}
