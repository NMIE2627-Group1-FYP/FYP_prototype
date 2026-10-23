#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChromeSphereActor.generated.h"

UCLASS()
class FYP_PROTOTYPE_API AChromeSphereActor : public AActor
{
    GENERATED_BODY()

public:
    AChromeSphereActor();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype|Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> MeshComponent;
};
