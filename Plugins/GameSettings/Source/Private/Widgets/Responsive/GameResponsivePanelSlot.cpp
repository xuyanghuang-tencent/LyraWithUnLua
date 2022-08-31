// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameResponsivePanelSlot.h"
#include "GameResponsivePanel.h"
#include "Components/Widget.h"

/////////////////////////////////////////////////////
// UGameResponsivePanelSlot

UGameResponsivePanelSlot::UGameResponsivePanelSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Slot = nullptr;
}

void UGameResponsivePanelSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	Slot = nullptr;
}

void UGameResponsivePanelSlot::BuildSlot(TSharedRef<SGameResponsivePanel> GameResponsivePanel)
{
	Slot = &GameResponsivePanel->AddSlot()
	[
		Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
	];
}

void UGameResponsivePanelSlot::SynchronizeProperties()
{
}
