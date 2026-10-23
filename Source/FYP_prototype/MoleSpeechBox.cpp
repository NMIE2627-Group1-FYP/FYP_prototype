#include "MoleSpeechBox.h"

#include "HAL/PlatformTime.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UMoleSpeechBox::UMoleSpeechBox(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SpeakerName = INVTEXT("Star-nosed mole");
    WalkHint = INVTEXT("Walk to hear the next line");
    Lines = {
        INVTEXT("You wake in the dark with an empty, aching stomach."),
        INVTEXT("The den's food store is bare."),
        INVTEXT("Behind you there is faint squeaking and slow breathing. The family depends on you."),
        INVTEXT("The floor hums under your feet. Find direction by hearing, and judge the ground by stepping."),
        INVTEXT("There is one purpose: find food and bring it home."),
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

    SAssignNew(HintText, STextBlock)
        .Text(WalkHint)
        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
        .ColorAndOpacity(FLinearColor(0.82f, 0.68f, 0.32f));

    SAssignNew(ProgressFill, SBox)
        .WidthOverride(8.0f)
        .HeightOverride(8.0f)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.82f, 0.68f, 0.32f, 0.95f))
        ];

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
                .Padding(0.0f, 0.0f, 0.0f, 12.0f)
                [
                    BodyText.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 8.0f)
                [
                    HintText.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBox)
                    .HeightOverride(8.0f)
                    [
                        SNew(SOverlay)
                        + SOverlay::Slot()
                        [
                            SNew(SBorder)
                            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                            .BorderBackgroundColor(FLinearColor(0.16f, 0.16f, 0.14f, 0.85f))
                        ]
                        + SOverlay::Slot()
                        .HAlign(HAlign_Left)
                        [
                            ProgressFill.ToSharedRef()
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

void UMoleSpeechBox::SetLines(const TArray<FString>& InLines)
{
    Lines.Reset();
    for (const FString& Line : InLines)
    {
        if (!Line.IsEmpty())
        {
            Lines.Add(FText::FromString(Line));
        }
    }
    CurrentLine = 0;
    Refresh();
}

void UMoleSpeechBox::SetSpeech(const FString& InSpeech)
{
    TArray<FString> AsLines;
    AsLines.Add(InSpeech);
    SetLines(AsLines);
}

void UMoleSpeechBox::Advance()
{
    HandleAdvance();
}

void UMoleSpeechBox::SetWalkProgress(float Amount)
{
    WalkProgress = FMath::Clamp(Amount, 0.0f, 1.0f);
    if (ProgressFill.IsValid())
    {
        ProgressFill->SetWidthOverride(FMath::Lerp(8.0f, 596.0f, WalkProgress));
    }

    if (HintText.IsValid())
    {
        const float Pulse = 0.55f + 0.45f * FMath::Abs(FMath::Sin(FPlatformTime::Seconds() * 3.2f));
        HintText->SetColorAndOpacity(FLinearColor(0.82f, 0.68f, 0.32f, Pulse));
        HintText->SetText(WalkHint);
    }
}

void UMoleSpeechBox::ShowLine(int32 LineIndex)
{
    CurrentLine = LineIndex;
    Refresh();
}

void UMoleSpeechBox::HandleAdvance()
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

    if (HintText.IsValid())
    {
        HintText->SetText(WalkHint);
    }
}
