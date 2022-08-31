// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"

#include "MaterialProgressBar.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UWidgetAnimation;

UCLASS(Abstract, meta = (DisableNativeTick))
class UMaterialProgressBar : public UCommonUserWidget
{
	GENERATED_BODY()

protected:

	virtual void SynchronizeProperties() override;

#if WITH_EDITOR
	virtual void OnWidgetRebuilt() override;
#endif

	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

public:

	UFUNCTION(BlueprintCallable)
	void SetProgress(float Progress);

	UFUNCTION(BlueprintCallable)
	void SetStartProgress(float StartProgress);

	UFUNCTION(BlueprintCallable)
	void SetColorA(FLinearColor ColorA);

	UFUNCTION(BlueprintCallable)
	void SetColorB(FLinearColor ColorB);

	UFUNCTION(BlueprintCallable)
	void SetColorBackground(FLinearColor ColorBackground);

	UFUNCTION(BlueprintCallable)
	void AnimateProgressFromStart(float Start, float End, float AnimSpeed = 1.0f);

	UFUNCTION(BlueprintCallable)
	void AnimateProgressFromCurrent(float End, float AnimSpeed = 1.0f);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFillAnimationFinished);

	UPROPERTY(BlueprintAssignable)
	FOnFillAnimationFinished OnFillAnimationFinished;

private:

	void SetProgress_Internal(float Progress);
	void SetStartProgress_Internal(float StartProgress);
	void SetColorA_Internal(FLinearColor ColorA);
	void SetColorB_Internal(FLinearColor ColorB);
	void SetColorBackground_Internal(FLinearColor ColorBackground);

	UMaterialInstanceDynamic* GetBarDynamicMaterial() const;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "CachedColorA"))
	bool bOverrideDefaultColorA = false;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Color A", EditCondition = "bOverrideDefaultColorA"))
	FLinearColor CachedColorA;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "CachedColorB"))
	bool bOverrideDefaultColorB = false;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Color B", EditCondition = "bOverrideDefaultColorB"))
	FLinearColor CachedColorB;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "CachedColorBackground"))
	bool bOverrideDefaultColorBackground;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Color Background", EditCondition = "bOverrideDefaultColorBackground"))
	FLinearColor CachedColorBackground;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "Segments"))
	bool bOverrideDefaultSegments = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultSegments"))
	int32 Segments;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "SegmentEdge"))
	bool bOverrideDefaultSegmentEdge = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultSegmentEdge"))
	float SegmentEdge;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "FillEdgeSoftness"))
	bool bOverrideDefaultFillEdgeSoftness;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultFillEdgeSoftness"))
	float FillEdgeSoftness;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "GlowEdge"))
	bool bOverrideDefaultGlowEdge = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultGlowEdge"))
	float GlowEdge;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "GlowSoftness"))
	bool bOverrideDefaultGlowSoftness = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultGlowSoftness"))
	float GlowSoftness;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle = "OutlineScale"))
	bool bOverrideDefaultOutlineScale = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideDefaultOutlineScale"))
	float OutlineScale;

	UPROPERTY(EditAnywhere)
	bool bUseStroke = true;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* StrokeMaterial;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* NoStrokeMaterial;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Design Time Progress"))
	float DesignTime_Progress = 1.0f;
#endif

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess))
	UImage* Image_Bar;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim, AllowPrivateAccess))
	UWidgetAnimation* BoundAnim_FillBar;

	UPROPERTY(Transient)
	mutable UMaterialInstanceDynamic* CachedMID;

	float CachedProgress = -1.0f;
	float CachedStartProgress = -1.0f;
};