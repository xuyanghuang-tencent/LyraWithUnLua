// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraLogChannels.h"
#include "Modules/ModuleManager.h"


/**
 * FLyraGameModule
 */
class FLyraGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame");
