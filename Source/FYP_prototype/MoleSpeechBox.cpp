#include "MoleSpeechBox.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UMoleSpeechBox::UMoleSpeechBox(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SpeakerName = INVTEXT("Star-nosed mole");
    NextLabel = INVTEXT("Next");
    Lines = {
        INVTEXT("You made it below. From here, you are me."),
        INVTEXT("The dark is not empty. The mud, the insects, and the quiet are the story."),
        INVTEXT("Watch how each insect moves in its own way. That is how you learn this place."),
    };
}

TSharedRef<SWidget> UMoleSpeechBox::RebuildWidget()
{
    const FSlateFontInfo NameFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);
    const FSlateFontInfo BodyFont = FCoreStyle::GetDefaultFontStyle("Regular", 16);

    BodyText = SNew(STextBlock)
        .Font(BodyFont)
        .AutoWrapText(true)
        .ColorAndOpacity(FLinearColor(0.93f, 0.95f, 0.9f));

    Refresh();

    TSharedRef<SWidget> Card = SNew(SBox)
        .WidthOverride(640.0f)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.05f, 0.07f, 0.08f, 0.92f))
            .Padding(FMargin(22.0f, 16.0f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 8.0f)
                [
                    SNew(STextBlock)
                    .Text(SpeakerName)
                    .Font(NameFont)
                    .ColorAndOpacity(FLinearColor(0.55f, 0.82f, 1.0f))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 14.0f)
                [
                    BodyText.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Right)
                [
                    SNew(SBox)
                    .WidthOverride(140.0f)
                    .HeightOverride(40.0f)
                    [
                        SNew(SButton)
                        .ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .OnClicked_Lambda([this]()
                        {
                            HandleNext();
                            return FReply::Handled();
                        })
                        [
                            SNew(STextBlock)
                            .Text(NextLabel)
                            .Justification(ETextJustify::Center)
                        ]
                    ]
                ]
            ]
        ];

    return SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Bottom)
        .Padding(FMargin(24.0f, 0.0f, 24.0f, 36.0f))
        [
            Card
        ];
}

void UMoleSpeechBox::SetSpeech(const FString& InSpeech)
{
    Lines.Reset();
    if (!InSpeech.IsEmpty())
    {
        Lines.Add(FText::FromString(InSpeech));
    }
    CurrentLine = 0;
    Refresh();
}

void UMoleSpeechBox::ShowLine(int32 LineIndex)
{
    CurrentLine = LineIndex;
    Refresh();
}

void UMoleSpeechBox::HandleNext()
{
    if (Lines.Num() == 0)
    {
        RemoveFromParent();
        return;
    }

    CurrentLine += 1;
    if (CurrentLine >= Lines.Num())
    {
        RemoveFromParent();
        return;
    }

    Refresh();
}

void UMoleSpeechBox::Refresh()
{
    if (!BodyText.IsValid())
    {
        return;
    }

    if (Lines.IsValidIndex(CurrentLine))
    {
        BodyText->SetText(Lines[CurrentLine]);
    }
}
