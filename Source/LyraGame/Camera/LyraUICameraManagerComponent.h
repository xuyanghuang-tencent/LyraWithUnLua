// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Camera/PlayerCameraManager.h"

#include "LyraUICameraManagerComponent.generated.h"

class APlayerController;
class ALyraPlayerCameraManager;
class AHUD;
class UCanvas;

UCLASS( Transient, Within=LyraPlayerCameraManager )
class ULyraUICameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static ULyraUICameraManagerComponent* GetComponent(APlayerController* PC);

public:
	ULyraUICameraManagerComponent();	
	virtual void InitializeComponent() override;

	bool IsSettingViewTarget() const { return bUpdatingViewTarget; }
	AActor* GetViewTarget() const { return ViewTarget; }
	void SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

	bool NeedsToUpdateViewTarget() const;
	void UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime);

	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

private:
	UPROPERTY(Transient)
	AActor* ViewTarget;
	
	UPROPERTY(Transient)
	bool bUpdatingViewTarget;
};
