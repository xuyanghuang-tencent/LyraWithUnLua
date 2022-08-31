// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraLoadingScreenSubsystem.h"

//////////////////////////////////////////////////////////////////////
// ULyraLoadingScreenSubsystem

ULyraLoadingScreenSubsystem::ULyraLoadingScreenSubsystem()
{
}

void ULyraLoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (LoadingScreenWidgetClass != NewWidgetClass)
	{
		LoadingScreenWidgetClass = NewWidgetClass;

		OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
	}
}

TSubclassOf<UUserWidget> ULyraLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
	return LoadingScreenWidgetClass;
}
