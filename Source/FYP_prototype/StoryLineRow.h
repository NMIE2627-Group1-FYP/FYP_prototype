#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StoryLineRow.generated.h"

USTRUCT(BlueprintType)
struct FStoryLineRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FText Text;

	/** Distance, in unreal units, at which this line and its lights appear. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	float DistanceThreshold = 5.0f;
};
