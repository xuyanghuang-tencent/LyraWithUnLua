// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PocketCapture.generated.h"

class UMaterialInterface;
class USceneCaptureComponent2D;
class UPocketCaptureSubsystem;

UCLASS(Abstract, Within=PocketCaptureSubsystem, BlueprintType, Blueprintable)
class POCKETWORLDS_API UPocketCapture : public UObject
{
	GENERATED_BODY()

public:
	UPocketCapture();

	virtual void Initialize(UWorld* InWorld, int32 RendererIndex);
	virtual void Deinitialize();

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable)
	void SetRenderTargetSize(int32 Width, int32 Height);

	UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* GetOrCreateDiffuseRenderTarget();

	UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* GetOrCreateAlphaMaskRenderTarget();

	UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* GetOrCreateEffectsRenderTarget();

	UFUNCTION(BlueprintCallable)
	void SetCaptureTarget(AActor* InCaptureTarget);

	UFUNCTION(BlueprintCallable)
	void SetAlphaMaskedActors(const TArray<AActor*>& InCaptureTarget);

	UFUNCTION(BlueprintCallable)
	void CaptureDiffuse();

	UFUNCTION(BlueprintCallable)
	void CaptureAlphaMask();

	UFUNCTION(BlueprintCallable)
	void CaptureEffects();

	UFUNCTION(BlueprintCallable)
	virtual void ReleaseResources();

	UFUNCTION(BlueprintCallable)
	virtual void ReclaimResources();

	UFUNCTION(BlueprintCallable)
	int32 GetRendererIndex() const;
	
protected:
	AActor* GetCaptureTarget() const { return CaptureTargetPtr.Get(); }
	virtual void OnCaptureTargetChanged(AActor* InCaptureTarget) {}

	bool CaptureScene(UTextureRenderTarget2D* InRenderTarget, const TArray<AActor*>& InCaptureActors, ESceneCaptureSource CaptureSource, UMaterialInterface* OverrideMaterial);

protected:
	TArray<UPrimitiveComponent*> GatherPrimitivesForCapture(const TArray<AActor*>& InCaptureActors) const;
	
	UPocketCaptureSubsystem* GetThumbnailSystem() const;

protected:

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* AlphaMaskMaterial;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* EffectMaskMaterial;

protected:
	UPROPERTY(Transient)
	UWorld* PrivateWorld;

	UPROPERTY(Transient)
	int32 RendererIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere)
	int32 SurfaceWidth = 1;

	UPROPERTY(VisibleAnywhere)
	int32 SurfaceHeight = 1;

	UPROPERTY(VisibleAnywhere)
	UTextureRenderTarget2D* DiffuseRT;

	UPROPERTY(VisibleAnywhere)
	UTextureRenderTarget2D* AlphaMaskRT;

	UPROPERTY(VisibleAnywhere)
	UTextureRenderTarget2D* EffectsRT;

	UPROPERTY(VisibleAnywhere)
	USceneCaptureComponent2D* CaptureComponent;

	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<AActor> CaptureTargetPtr;

	UPROPERTY(VisibleAnywhere)
	TArray<TWeakObjectPtr<AActor>> AlphaMaskActorPtrs;
};
