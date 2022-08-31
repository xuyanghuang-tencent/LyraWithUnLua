// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/AimAssistInputModifier.h"
#include "UObject/Interface.h"
#include "IAimAssistTargetInterface.generated.h"

USTRUCT(BlueprintType)
struct FAimAssistTargetOptions
{
	GENERATED_BODY()
	
	FAimAssistTargetOptions()
		: bIsActive(true)
	{}

	/** The shape component that should be used when considering this target's hitbox */
	TWeakObjectPtr<UShapeComponent> TargetShapeComponent;

	/**
	 * Gameplay tags that are associated with this target that can be used to filter it out.
	 *
	 * If the player's aim assist settings have any tags that match these, it will be excluded.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer AssociatedTags;

	/** Whether or not this target is currently active. If false, it will not be considered for aim assist */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint8 bIsActive : 1;
};


UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UAimAssistTaget : public UInterface
{
	GENERATED_BODY()
};

/**
 * Used to define the shape of an aim assist target as well as let the aim assist manager know
 * about any associated gameplay tags.
 * 
 * The target will be considered when it is within the view of a player's outer reticle
 *
 * @see UAimAssistTargetComponent for an example
 */
class IAimAssistTaget
{
	GENERATED_BODY()

public:
	/** Populate the given target data with this interface. This will be called when a target is within view of the player */
	virtual void GatherTargetOptions(OUT FAimAssistTargetOptions& TargetData) = 0;
};
