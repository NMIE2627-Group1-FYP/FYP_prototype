#pragma once

#include "CoreMinimal.h"
#include "CaveUserWidget.h"
#include "MoleSpeechBox.generated.h"

/** Dialogue widget. Behaviour lives on UCaveUserWidget so it can share WBP_Base_CAVE. */
UCLASS(Blueprintable)
class FYP_PROTOTYPE_API UMoleSpeechBox : public UCaveUserWidget
{
	GENERATED_BODY()
};
