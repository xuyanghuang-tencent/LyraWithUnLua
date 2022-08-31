// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetTypeActions_LyraContextEffectsLibrary.h"
#include "Feedback/ContextEffects/LyraContextEffectsLibrary.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FAssetTypeActions_LyraContextEffectsLibrary::GetSupportedClass() const
{
	return ULyraContextEffectsLibrary::StaticClass();
}

#undef LOCTEXT_NAMESPACE
