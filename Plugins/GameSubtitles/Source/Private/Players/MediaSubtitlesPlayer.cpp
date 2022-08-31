// Copyright Epic Games, Inc. All Rights Reserved.

#include "Players/MediaSubtitlesPlayer.h"

#include "Overlays.h"
#include "SubtitleManager.h"
#include "MediaPlayer.h"

UMediaSubtitlesPlayer::UMediaSubtitlesPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MediaPlayer(nullptr)
	, bEnabled(false)
{
}

void UMediaSubtitlesPlayer::BeginDestroy()
{
	Stop();

	Super::BeginDestroy();
}

void UMediaSubtitlesPlayer::Play()
{
	bEnabled = true;
}

void UMediaSubtitlesPlayer::Stop()
{
	bEnabled = false;

	// Clear the movie subtitle for this object
	FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, TArray<FString>());
}

void UMediaSubtitlesPlayer::SetSubtitles(UOverlays* Subtitles)
{
	SourceSubtitles = Subtitles;
}

void UMediaSubtitlesPlayer::BindToMediaPlayer(UMediaPlayer* InMediaPlayer)
{
	MediaPlayer = InMediaPlayer;
}

void UMediaSubtitlesPlayer::Tick(float DeltaSeconds)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_UMediaSubtitlesPlayer_Tick);

	if (bEnabled && SourceSubtitles)
	{
		UMediaPlayer* MediaPlayerPtr = MediaPlayer.Get();
		if (MediaPlayerPtr)
		{
			FTimespan CurrentTime = MediaPlayerPtr->GetTime();
			TArray<FOverlayItem> CurrentSubtitles;
			SourceSubtitles->GetOverlaysForTime(CurrentTime, CurrentSubtitles);

			TArray<FString> SubtitlesText;
			for (const FOverlayItem& Subtitle : CurrentSubtitles)
			{
				SubtitlesText.Add(Subtitle.Text);
			}

			FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, SubtitlesText);
		}
		else
		{
			Stop();
		}
	}
}
