// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidgetPool.h"
#include "Engine/StreamableManager.h"
#include "GameSettingDetailView.generated.h"

class UGameSettingListView;
class UCommonRichTextBlock;
class UCommonTextBlock;
class UGameSetting;
class UGameSettingDetailExtension;
class UVerticalBox;
class UGameSettingVisualData;

/**
 * 
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingDetailView : public UUserWidget
{
	GENERATED_BODY()
public:
	UGameSettingDetailView(const FObjectInitializer& ObjectInitializer);

	void FillSettingDetails(UGameSetting* InSetting);

	//UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	void CreateDetailsExtension(UGameSetting* InSetting, TSubclassOf<UGameSettingDetailExtension> ExtensionClass);

protected:
	UPROPERTY(EditAnywhere)
	UGameSettingVisualData* VisualData;

	UPROPERTY(Transient)
	FUserWidgetPool ExtensionWidgetPool;

	UPROPERTY(Transient)
	UGameSetting* CurrentSetting;

	TSharedPtr<FStreamableHandle> StreamingHandle;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Text_SettingName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonRichTextBlock* RichText_Description;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonRichTextBlock* RichText_DynamicDetails;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonRichTextBlock* RichText_WarningDetails;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonRichTextBlock* RichText_DisabledDetails;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Box_DetailsExtension;
};
