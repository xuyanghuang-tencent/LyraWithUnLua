// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraActorUtilities.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

EBlueprintExposedNetMode ULyraActorUtilities::SwitchOnNetMode(const UObject* WorldContextObject)
{
	ENetMode NetMode = NM_Standalone;
	for (const UObject* TestObject = WorldContextObject; TestObject != nullptr; TestObject = TestObject->GetOuter())
	{
		if (const UActorComponent* Component = Cast<const UActorComponent>(WorldContextObject))
		{
			NetMode = Component->GetNetMode();
			break;
		}
		else if (const AActor* Actor = Cast<const AActor>(WorldContextObject))
		{
			NetMode = Actor->GetNetMode();
			break;
		}
	}

	switch (NetMode)
	{
	case NM_Client:
		return EBlueprintExposedNetMode::Client;
	case NM_Standalone:
		return EBlueprintExposedNetMode::Standalone;
	case NM_DedicatedServer:
		return EBlueprintExposedNetMode::DedicatedServer;
	case NM_ListenServer:
		return EBlueprintExposedNetMode::ListenServer;
	default:
		ensure(false);
		return EBlueprintExposedNetMode::Standalone;
	}
}
