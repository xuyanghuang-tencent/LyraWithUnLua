// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraWidgetFactory.h"

TSubclassOf<UUserWidget> ULyraWidgetFactory::FindWidgetClassForData_Implementation(const UObject* Data) const
{
	return TSubclassOf<UUserWidget>();
}