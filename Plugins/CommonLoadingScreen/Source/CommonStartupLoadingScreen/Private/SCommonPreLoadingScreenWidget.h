// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "UObject/GCObject.h"

#include "Animation/CurveHandle.h"
#include "Animation/CurveSequence.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UFont;
class UMaterialInstance;
class UTexture2D;

class SCommonPreLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SCommonPreLoadingScreenWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End FGCObject interface

private:

};