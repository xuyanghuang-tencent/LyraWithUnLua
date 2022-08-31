// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraRuntimeOptions.h"

ULyraRuntimeOptions::ULyraRuntimeOptions()
{
	OptionCommandPrefix = TEXT("ro");
}

ULyraRuntimeOptions* ULyraRuntimeOptions::GetRuntimeOptions()
{
	return GetMutableDefault<ULyraRuntimeOptions>();
}

const ULyraRuntimeOptions& ULyraRuntimeOptions::Get()
{
	const ULyraRuntimeOptions& RuntimeOptions = *GetDefault<ULyraRuntimeOptions>();
	return RuntimeOptions;
}