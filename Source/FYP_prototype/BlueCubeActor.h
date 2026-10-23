#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlueCubeActor.generated.h"

UCLASS()
class FYP_PROTOTYPE_API ABlueCubeActor : public AActor
{
    GENERATED_BODY()

public:
    ABlueCubeActor();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype|Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> MeshComponent;
};
