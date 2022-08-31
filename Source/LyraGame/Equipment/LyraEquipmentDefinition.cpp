// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEquipmentDefinition.h"
#include "LyraEquipmentInstance.h"

ULyraEquipmentDefinition::ULyraEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstanceType = ULyraEquipmentInstance::StaticClass();
}
