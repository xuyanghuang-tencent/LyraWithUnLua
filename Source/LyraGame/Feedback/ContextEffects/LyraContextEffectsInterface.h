// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "LyraContextEffectsInterface.generated.h"

class UStaticMeshComponent;
class UAnimSequenceBase;
class ULyraContextEffectsLibrary;

/**
 *
 */
UENUM()
enum EEffectsContextMatchType
{
	ExactMatch,
	BestMatch
};

/**
 *
 */
 UINTERFACE(Blueprintable)
 class LYRAGAME_API ULyraContextEffectsInterface : public UInterface
 {
	 GENERATED_BODY()

 };
 
 class LYRAGAME_API ILyraContextEffectsInterface : public IInterface
 {
	 GENERATED_BODY()

 public:

	/** */
 	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AnimMotionEffect(const FName Bone
		, const FGameplayTag MotionEffect
		, USceneComponent* StaticMeshComponent
		, const FVector LocationOffset
		, const FRotator RotationOffset
		, const UAnimSequenceBase* AnimationSequence
		, const bool bHitSuccess
		, const FHitResult HitResult
		, FGameplayTagContainer Contexts
		, FVector VFXScale = FVector(1)
		, float AudioVolume = 1
		, float AudioPitch = 1);
 };

