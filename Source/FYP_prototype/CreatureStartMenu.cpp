#include "CreatureStartMenu.h"

#include "CreatureCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UCreatureStartMenu::UCreatureStartMenu(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SurfaceTitle = INVTEXT("Surface");
    UndergroundTitle = INVTEXT("Underground");
    StoryText = INVTEXT("A star-nosed mole is waiting by the path. Follow the arrows to the manhole. Under the ground, you become the mole: the dark is home, and the small lives in the mud are the world you came to meet.");
    StartLabel = INVTEXT("Start");
    SettingsLabel = INVTEXT("Settings");
    QuitLabel = INVTEXT("Quit");
    ResumeLabel = INVTEXT("Back to game");
    BackLabel = INVTEXT("Back");
    EscHint = INVTEXT("Esc    Settings  ·  Quit");
    UndergroundHint = INVTEXT("Underground     Esc    Settings  ·  Quit");
    EscHelpText = INVTEXT("Settings and quit stay available while you play. Press Esc to open them again.");
    UndergroundHelpText = INVTEXT("You are the mole now. The gravel and the insects are close. Press Esc for Settings and Quit.");
    CardPadding = FMargin(40.0f, 36.0f);
}

void UCreatureStartMenu::SetOwnerCharacter(ACreatureCharacter* InCharacter)
{
    OwnerCharacter = InCharacter;
}

void UCreatureStartMenu::SetIncludeStart(bool bInIncludeStart)
{
    bIncludeStart = bInIncludeStart;
    bCardOpen = bInIncludeStart;
}

bool UCreatureStartMenu::IsCardOpen() const
{
    return bCardOpen;
}

TSharedRef<SWidget> UCreatureStartMenu::MakeButton(
    const FString& Label,
    TFunction<void()> Callback
) const
{
    const FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);

    return SNew(SBox)
        .HeightOverride(52.0f)
        .Padding(FMargin(0.0f, 8.0f))
        [
            SNew(SButton)
            .ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .OnClicked_Lambda([Callback]()
            {
                Callback();
                return FReply::Handled();
            })
            [
                SNew(STextBlock)
                .Text(FText::FromString(Label))
                .Font(ButtonFont)
                .Justification(ETextJustify::Center)
            ]
        ];
}

TSharedRef<SWidget> UCreatureStartMenu::RebuildWidget()
{
    const FLinearColor PanelColor(0.04f, 0.06f, 0.05f, 0.94f);
    const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 32);
    const FSlateFontInfo BodyFont = FCoreStyle::GetDefaultFontStyle("Regular", 16);
    const FSlateFontInfo HintFont = FCoreStyle::GetDefaultFontStyle("Regular", 14);
    const FText LevelTitle = bIncludeStart ? SurfaceTitle : UndergroundTitle;

    StartPanel = SNew(SVerticalBox)
        .Visibility(bIncludeStart ? EVisibility::Visible : EVisibility::Collapsed)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.0f, 0.0f, 0.0f, 12.0f)
        [
            SNew(STextBlock)
            .Text(LevelTitle)
            .Font(TitleFont)
            .Justification(ETextJustify::Center)
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.0f, 0.0f, 8.0f, 28.0f)
        [
            SNew(STextBlock)
            .Text(StoryText)
            .Font(BodyFont)
            .Justification(ETextJustify::Center)
            .AutoWrapText(true)
            .ColorAndOpacity(FLinearColor(0.86f, 0.9f, 0.84f))
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            MakeButton(StartLabel.ToString(), [this]() { HandleStart(); })
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            MakeButton(SettingsLabel.ToString(), [this]() { ShowSettings(); })
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            MakeButton(QuitLabel.ToString(), [this]() { HandleQuit(); })
        ];

    LevelPanel = SNew(SVerticalBox)
        .Visibility(EVisibility::Collapsed)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.0f, 0.0f, 0.0f, 12.0f)
        [
            SNew(STextBlock)
            .Text(LevelTitle)
            .Font(TitleFont)
            .Justification(ETextJustify::Center)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.0f, 0.0f, 8.0f, 28.0f)
        [
            SNew(STextBlock)
            .Text(bIncludeStart ? EscHelpText : UndergroundHelpText)
            .Font(BodyFont)
            .Justification(ETextJustify::Center)
            .AutoWrapText(true)
            .ColorAndOpacity(FLinearColor(0.86f, 0.9f, 0.84f))
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            MakeButton(SettingsLabel.ToString(), [this]() { ShowSettings(); })
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            MakeButton(QuitLabel.ToString(), [this]() { HandleQuit(); })
        ]
        + SVerticalBox::Slot().AutoHeight()
        .Padding(0.0f, 8.0f, 0.0f, 0.0f)
        [
            MakeButton(ResumeLabel.ToString(), [this]() { HandleResume(); })
        ];

    SettingsPanel = SNew(SVerticalBox)
        .Visibility(EVisibility::Collapsed)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.0f, 0.0f, 0.0f, 16.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Settings")))
            .Font(TitleFont)
            .Justification(ETextJustify::Center)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f, 0.0f, 4.0f, 10.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Mouse sensitivity")))
            .Font(BodyFont)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f, 4.0f, 4.0f, 20.0f)
        [
            SNew(SSlider)
            .MinValue(0.2f)
            .MaxValue(3.0f)
            .Value(OwnerCharacter.IsValid() ? OwnerCharacter->GetLookSensitivity() : 1.0f)
            .OnValueChanged_Lambda([this](float NewValue)
            {
                HandleSensitivityChanged(NewValue);
            })
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f, 0.0f, 4.0f, 24.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("OSC control will be added in a later version.")))
            .Font(BodyFont)
            .AutoWrapText(true)
            .Justification(ETextJustify::Center)
            .ColorAndOpacity(FLinearColor(0.75f, 0.8f, 0.72f))
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            MakeButton(BackLabel.ToString(), [this]()
            {
                if (bReturnToStart)
                {
                    ShowStartCard();
                }
                else
                {
                    ShowLevelCard();
                }
            })
        ];

    Card = SNew(SBorder)
        .Visibility(bIncludeStart ? EVisibility::Visible : EVisibility::Collapsed)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(PanelColor)
        .Padding(CardPadding)
        [
            SNew(SBox)
            .WidthOverride(CardWidth)
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                [
                    StartPanel.ToSharedRef()
                ]
                + SOverlay::Slot()
                [
                    LevelPanel.ToSharedRef()
                ]
                + SOverlay::Slot()
                [
                    SettingsPanel.ToSharedRef()
                ]
            ]
        ];

    Hint = SNew(SBorder)
        .Visibility(bIncludeStart ? EVisibility::Collapsed : EVisibility::Visible)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.08f, 0.1f, 0.08f, 0.9f))
        .Padding(FMargin(18.0f, 12.0f))
        [
            SNew(STextBlock)
            .Text(bIncludeStart ? EscHint : UndergroundHint)
            .Font(HintFont)
            .ColorAndOpacity(FLinearColor(0.96f, 0.97f, 0.94f))
        ];

    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SColorBlock)
            .Color(FLinearColor(0.0f, 0.0f, 0.0f, bIncludeStart ? 0.45f : 0.0f))
            .Visibility(EVisibility::HitTestInvisible)
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            Card.ToSharedRef()
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Top)
        .Padding(FMargin(0.0f, 28.0f, 28.0f, 0.0f))
        [
            Hint.ToSharedRef()
        ];
}

void UCreatureStartMenu::ShowStartCard()
{
    bCardOpen = true;

    if (Card.IsValid())
    {
        Card->SetVisibility(EVisibility::Visible);
    }

    if (StartPanel.IsValid())
    {
        StartPanel->SetVisibility(EVisibility::Visible);
    }

    if (LevelPanel.IsValid())
    {
        LevelPanel->SetVisibility(EVisibility::Collapsed);
    }

    if (SettingsPanel.IsValid())
    {
        SettingsPanel->SetVisibility(EVisibility::Collapsed);
    }

    if (Hint.IsValid())
    {
        Hint->SetVisibility(EVisibility::Collapsed);
    }
}

void UCreatureStartMenu::ShowLevelCard()
{
    bCardOpen = true;

    if (Card.IsValid())
    {
        Card->SetVisibility(EVisibility::Visible);
    }

    if (StartPanel.IsValid())
    {
        StartPanel->SetVisibility(EVisibility::Collapsed);
    }

    if (LevelPanel.IsValid())
    {
        LevelPanel->SetVisibility(EVisibility::Visible);
    }

    if (SettingsPanel.IsValid())
    {
        SettingsPanel->SetVisibility(EVisibility::Collapsed);
    }
}

void UCreatureStartMenu::ShowPlaying()
{
    bCardOpen = false;

    if (Card.IsValid())
    {
        Card->SetVisibility(EVisibility::Collapsed);
    }

    if (Hint.IsValid())
    {
        Hint->SetVisibility(EVisibility::Visible);
    }
}

void UCreatureStartMenu::ShowSettings()
{
    bReturnToStart = StartPanel.IsValid() && StartPanel->GetVisibility() == EVisibility::Visible;
    bCardOpen = true;

    if (Card.IsValid())
    {
        Card->SetVisibility(EVisibility::Visible);
    }

    if (StartPanel.IsValid())
    {
        StartPanel->SetVisibility(EVisibility::Collapsed);
    }

    if (LevelPanel.IsValid())
    {
        LevelPanel->SetVisibility(EVisibility::Collapsed);
    }

    if (SettingsPanel.IsValid())
    {
        SettingsPanel->SetVisibility(EVisibility::Visible);
    }
}

void UCreatureStartMenu::HandleStart()
{
    if (OwnerCharacter.IsValid())
    {
        OwnerCharacter->DismissStartMenu();
    }
}

void UCreatureStartMenu::HandleResume()
{
    if (OwnerCharacter.IsValid())
    {
        OwnerCharacter->CloseLevelMenu();
    }
}

void UCreatureStartMenu::HandleQuit()
{
    UKismetSystemLibrary::QuitGame(
        this,
        GetOwningPlayer(),
        EQuitPreference::Quit,
        false
    );
}

void UCreatureStartMenu::HandleSensitivityChanged(float NewValue)
{
    if (OwnerCharacter.IsValid())
    {
        OwnerCharacter->SetLookSensitivity(NewValue);
    }
}
