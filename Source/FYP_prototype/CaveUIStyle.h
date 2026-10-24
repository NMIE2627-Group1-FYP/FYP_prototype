#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateTypes.h"
#include "CaveUIStyle.generated.h"

class UMaterialInterface;
class UObject;

/**
 * Shared Slate widget style for every CAVE widget.
 * Text, buttons, spacing, and UI materials are read from this asset.
 */
UCLASS(BlueprintType)
class FYP_PROTOTYPE_API UCaveUIStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	UCaveUIStyle();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Text")
	FTextBlockStyle TitleStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Text")
	FTextBlockStyle BodyStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Text")
	FTextBlockStyle HintStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Text")
	FTextBlockStyle ButtonLabelStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Buttons")
	FButtonStyle ButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Layout")
	FMargin CardPadding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Layout")
	FMargin ButtonPadding;

	/** Project font. When unset, widgets keep the font already set in the Designer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Font")
	TObjectPtr<UObject> FontObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Materials")
	TObjectPtr<UMaterialInterface> EmissiveTextMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Materials")
	TObjectPtr<UMaterialInterface> VignetteMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Materials")
	TObjectPtr<UMaterialInterface> PromptMaterial;

	/** Brighter content tint while a menu button is hovered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cave|Buttons")
	FLinearColor HoverTint;

	static const TCHAR* AssetPath;

	/** The project style asset, or a transient default built from the UI materials when it is missing. */
	static UCaveUIStyle* Load(UObject* Outer);
};
