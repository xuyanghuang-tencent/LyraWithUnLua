// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraTabButtonBase.h"
#include "CommonLazyImage.h"
#include "CommonTextBlock.h"

void ULyraTabButtonBase::SetIconFromLazyObject(TSoftObjectPtr<UObject> LazyObject)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrushFromLazyDisplayAsset(LazyObject);
	}
}

void ULyraTabButtonBase::SetIconBrush(const FSlateBrush& Brush)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrush(Brush);
	}
}

void ULyraTabButtonBase::SetTabLabelInfo_Implementation(const FLyraTabDescriptor& TabLabelInfo)
{
	SetButtonText(TabLabelInfo.TabText);
	SetIconBrush(TabLabelInfo.IconBrush);
}
