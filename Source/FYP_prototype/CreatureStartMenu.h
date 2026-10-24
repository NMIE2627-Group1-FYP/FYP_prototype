#pragma once

#include "CoreMinimal.h"
#include "CaveUserWidget.h"
#include "CreatureStartMenu.generated.h"

/** Surface and pause menu. Behaviour lives on UCaveUserWidget so WBP_Base_CAVE can host it. */
UCLASS(Blueprintable)
class FYP_PROTOTYPE_API UCreatureStartMenu : public UCaveUserWidget
{
	GENERATED_BODY()
};
