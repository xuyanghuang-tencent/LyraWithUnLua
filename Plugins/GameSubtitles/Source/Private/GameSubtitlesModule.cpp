// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "GameplayTagsManager.h"
#include "Misc/Paths.h"

class FGameSubtitlesModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FGameSubtitlesModule::StartupModule()
{
	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::ProjectPluginsDir() / TEXT("GameSubtitles/Config/Tags"));
}

void FGameSubtitlesModule::ShutdownModule()
{
}
	
IMPLEMENT_MODULE(FGameSubtitlesModule, GameSubtitles)