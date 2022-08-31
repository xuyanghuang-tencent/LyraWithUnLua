// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAimSensitivityData.h"

ULyraAimSensitivityData::ULyraAimSensitivityData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SensitivityMap =
	{
		{ ELyraGamepadSensitivity::Slow,			0.5f },
		{ ELyraGamepadSensitivity::SlowPlus,		0.75f },
		{ ELyraGamepadSensitivity::SlowPlusPlus,	0.9f },
		{ ELyraGamepadSensitivity::Normal,		1.0f },
		{ ELyraGamepadSensitivity::NormalPlus,	1.1f },
		{ ELyraGamepadSensitivity::NormalPlusPlus,1.25f },
		{ ELyraGamepadSensitivity::Fast,			1.5f },
		{ ELyraGamepadSensitivity::FastPlus,		1.75f },
		{ ELyraGamepadSensitivity::FastPlusPlus,	2.0f },
		{ ELyraGamepadSensitivity::Insane,		2.5f },
	};
}

const float ULyraAimSensitivityData::SensitivtyEnumToFloat(const ELyraGamepadSensitivity InSensitivity) const
{
	if (const float* Sens = SensitivityMap.Find(InSensitivity))
	{
		return *Sens;
	}

	return 1.0f;
}
