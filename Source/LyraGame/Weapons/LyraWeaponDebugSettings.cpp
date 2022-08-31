// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraWeaponDebugSettings.h"
#include "Misc/App.h"

ULyraWeaponDebugSettings::ULyraWeaponDebugSettings()
{
}

FName ULyraWeaponDebugSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}
