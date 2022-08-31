// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ListView.h"
#include "GameSettingListEntry.h"

#include "GameSettingListView.generated.h"

class UGameSettingCollection;
class ULocalPlayer;
class UGameSettingVisualData;

UCLASS()
class GAMESETTINGS_API UGameSettingListView : public UListView
{
	GENERATED_BODY()

public:
	UGameSettingListView(const FObjectInitializer& ObjectInitializer);

	void AddNameOverride(const FName& DevName, const FText& OverrideName);

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const override;
#endif

protected:
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;
	virtual bool OnIsSelectableOrNavigableInternal(UObject* SelectedItem) override;

protected:
	UPROPERTY(EditAnywhere)
	UGameSettingVisualData* VisualData;

private:
	TMap<FName, FText> NameOverrides;
};