// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraWorldCollectable.h"
#include "EngineUtils.h"

ALyraWorldCollectable::ALyraWorldCollectable()
{
}

void ALyraWorldCollectable::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder)
{
	InteractionBuilder.AddInteractionOption(Option);
}

FInventoryPickup ALyraWorldCollectable::GetPickupInventory() const
{
	return StaticInventory;
}