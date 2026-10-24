#include "CaveUIStyle.h"

#include "Engine/Font.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Styling/SlateBrush.h"

const TCHAR* UCaveUIStyle::AssetPath = TEXT("/Game/GameProject/UI/DA_CaveSlateStyle.DA_CaveSlateStyle");

namespace CaveUIStylePrivate
{
	FSlateBrush MakeEmptyBrush()
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
		Brush.TintColor = FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
		return Brush;
	}

	void SeedTextStyle(FTextBlockStyle& Style, float Size, const FLinearColor& Color)
	{
		FSlateFontInfo Font;
		Font.Size = Size;
		Font.TypefaceFontName = TEXT("Regular");
		Font.OutlineSettings.OutlineSize = 1;
		Font.OutlineSettings.OutlineColor = FLinearColor::Black;
		Font.OutlineSettings.bApplyOutlineToDropShadows = true;
		Style.SetFont(Font);
		Style.SetColorAndOpacity(FSlateColor(Color));
		Style.SetShadowOffset(FVector2D(0.6f, 0.8f));
		Style.SetShadowColorAndOpacity(FLinearColor(0.05f, 0.03f, 0.0f, 0.55f));
	}
}

UCaveUIStyle::UCaveUIStyle()
{
	const FLinearColor CaveInk(0.898f, 0.867f, 0.796f, 1.0f);
	CaveUIStylePrivate::SeedTextStyle(TitleStyle, 34.0f, CaveInk);
	CaveUIStylePrivate::SeedTextStyle(BodyStyle, 24.0f, CaveInk);
	CaveUIStylePrivate::SeedTextStyle(HintStyle, 15.0f, CaveInk);
	CaveUIStylePrivate::SeedTextStyle(ButtonLabelStyle, 18.0f, CaveInk);

	const FSlateBrush EmptyBrush = CaveUIStylePrivate::MakeEmptyBrush();
	ButtonStyle.SetNormal(EmptyBrush);
	ButtonStyle.SetHovered(EmptyBrush);
	ButtonStyle.SetPressed(EmptyBrush);
	ButtonStyle.SetDisabled(EmptyBrush);
	ButtonStyle.NormalPadding = FMargin(12.0f, 8.0f);
	ButtonStyle.PressedPadding = FMargin(12.0f, 9.0f);

	CardPadding = FMargin(28.0f, 24.0f);
	ButtonPadding = FMargin(0.0f, 6.0f);
	HoverTint = FLinearColor(1.35f, 1.28f, 1.08f, 1.0f);
}

UCaveUIStyle* UCaveUIStyle::Load(UObject* Outer)
{
	if (FPackageName::DoesPackageExist(TEXT("/Game/GameProject/UI/DA_CaveSlateStyle")))
	{
		if (UCaveUIStyle* Asset = LoadObject<UCaveUIStyle>(nullptr, AssetPath))
		{
			return Asset;
		}
	}

	UCaveUIStyle* Style = NewObject<UCaveUIStyle>(Outer ? Outer : GetTransientPackage());
	Style->EmissiveTextMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/GameProject/UI/Materials/M_UI_EmissiveText.M_UI_EmissiveText"));
	Style->VignetteMaterial = nullptr;
	Style->PromptMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/GameProject/UI/Materials/M_UI_PromptPulse.M_UI_PromptPulse"));
	Style->FontObject = LoadObject<UFont>(nullptr, TEXT("/Game/GameProject/UI/Fonts/Font_Cave.Font_Cave"));
	return Style;
}
