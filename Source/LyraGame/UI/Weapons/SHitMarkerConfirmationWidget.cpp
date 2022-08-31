// Copyright Epic Games, Inc. All Rights Reserved.

#include "SHitMarkerConfirmationWidget.h"
#include "Weapons/LyraWeaponStateComponent.h"

SHitMarkerConfirmationWidget::SHitMarkerConfirmationWidget()
{
}

void SHitMarkerConfirmationWidget::Construct(const FArguments& InArgs, const FLocalPlayerContext& InContext, const TMap<FGameplayTag, FSlateBrush>& ZoneOverrideImages)
{
	PerHitMarkerImage = InArgs._PerHitMarkerImage;
	PerHitMarkerZoneOverrideImages = ZoneOverrideImages;
	AnyHitsMarkerImage = InArgs._AnyHitsMarkerImage;
	bColorAndOpacitySet = InArgs._ColorAndOpacity.IsSet();
	ColorAndOpacity = InArgs._ColorAndOpacity;

	MyContext = InContext;
}

int32 SHitMarkerConfirmationWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	const FVector2D LocalCenter = AllottedGeometry.GetLocalPositionAtCoordinates(FVector2D(0.5f, 0.5f));

	const bool bDrawMarkers = (HitNotifyOpacity > KINDA_SMALL_NUMBER);

	if (bDrawMarkers)
	{
		// Check if we should use screen-space damage location hit notifies
		TArray<FLyraScreenSpaceHitLocation> LastWeaponDamageScreenLocations;
		if (APlayerController* PC = MyContext.IsInitialized() ? MyContext.GetPlayerController() : nullptr)
		{
			if (ULyraWeaponStateComponent* WeaponStateComponent = PC->FindComponentByClass<ULyraWeaponStateComponent>())
			{
				WeaponStateComponent->GetLastWeaponDamageScreenLocations(/*out*/ LastWeaponDamageScreenLocations);
			}
		}

		if ((LastWeaponDamageScreenLocations.Num() > 0) && (PerHitMarkerImage != nullptr))
		{
			const FVector2D HalfAbsoluteSize = AllottedGeometry.GetAbsoluteSize() * 0.5f;

			for (const FLyraScreenSpaceHitLocation& Hit : LastWeaponDamageScreenLocations)
			{
				const FSlateBrush* LocationMarkerImage = PerHitMarkerZoneOverrideImages.Find(Hit.HitZone);
				if (LocationMarkerImage == nullptr)
				{
					LocationMarkerImage = PerHitMarkerImage;
				}

				FLinearColor MarkerColor = bColorAndOpacitySet ?
					ColorAndOpacity.Get().GetColor(InWidgetStyle) :
					(InWidgetStyle.GetColorAndOpacityTint() * LocationMarkerImage->GetTint(InWidgetStyle));
				MarkerColor.A *= HitNotifyOpacity;

				const FVector2D WindowSSLocation = Hit.Location + MyCullingRect.GetTopLeft(); // Accounting for window trim when not in fullscreen mode
				const FSlateRenderTransform DrawPos(AllottedGeometry.AbsoluteToLocal(WindowSSLocation));

				const FPaintGeometry Geometry(AllottedGeometry.ToPaintGeometry(LocationMarkerImage->ImageSize, FSlateLayoutTransform(-(LocationMarkerImage->ImageSize * 0.5f)), DrawPos));
				FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geometry, LocationMarkerImage, DrawEffects, MarkerColor);
			}
		}
		
		if (AnyHitsMarkerImage != nullptr)
		{
			FLinearColor MarkerColor = bColorAndOpacitySet ?
				ColorAndOpacity.Get().GetColor(InWidgetStyle) :
				(InWidgetStyle.GetColorAndOpacityTint() * AnyHitsMarkerImage->GetTint(InWidgetStyle));
			MarkerColor.A *= HitNotifyOpacity;

			// Otherwise show the hit notify in the center of the reticle
			const FPaintGeometry Geometry(AllottedGeometry.ToPaintGeometry(AnyHitsMarkerImage->ImageSize, FSlateLayoutTransform(LocalCenter - (AnyHitsMarkerImage->ImageSize * 0.5f))));
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geometry, AnyHitsMarkerImage, DrawEffects, MarkerColor);
		}
	}

	return LayerId;
}

FVector2D SHitMarkerConfirmationWidget::ComputeDesiredSize(float) const
{
	return FVector2D(100.0f, 100.0f);
}

void SHitMarkerConfirmationWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	HitNotifyOpacity = 0.0f;

	if (APlayerController* PC = MyContext.IsInitialized() ? MyContext.GetPlayerController() : nullptr)
	{
		if (ULyraWeaponStateComponent* DamageMarkerComponent = PC->FindComponentByClass<ULyraWeaponStateComponent>())
		{
			const double TimeSinceLastHitNotification = DamageMarkerComponent->GetTimeSinceLastHitNotification();
			if (TimeSinceLastHitNotification < HitNotifyDuration)
			{
				HitNotifyOpacity = FMath::Clamp(1.0f - (float)(TimeSinceLastHitNotification / HitNotifyDuration), 0.0f, 1.0f);
			}
		}
	}
}
