// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Subsystems/WorldSubsystem.h"
#include "PocketCaptureSubsystem.generated.h"

class ULevel;
class UWorld;

UCLASS(BlueprintType)
class POCKETWORLDS_API UPocketCaptureSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPocketCaptureSubsystem();

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "PocketCaptureClass"))
	UPocketCapture* CreateThumbnailRenderer(TSubclassOf<UPocketCapture> PocketCaptureClass);

	UFUNCTION(BlueprintCallable)
	void DestroyThumbnailRenderer(UPocketCapture* ThumbnailRenderer);

	void StreamThisFrame(TArray<UPrimitiveComponent*>& PrimitiveComponents);

protected:
	bool Tick(float DeltaTime);

	TArray<TWeakObjectPtr<UPrimitiveComponent>> StreamNextFrame;
	TArray<TWeakObjectPtr<UPrimitiveComponent>> StreamedLastFrameButNotNext;

private:
	TArray<TWeakObjectPtr<UPocketCapture>> ThumbnailRenderers;

	FTSTicker::FDelegateHandle TickHandle;
};