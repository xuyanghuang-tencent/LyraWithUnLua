// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonLocalPlayer.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Teams/LyraTeamAgentInterface.h"
#include "LyraLocalPlayer.generated.h"

class ULyraSettingsLocal;
class ULyraSettingsShared;
class UInputMappingContext;

/**
 * ULyraLocalPlayer
 */
UCLASS()
class LYRAGAME_API ULyraLocalPlayer : public UCommonLocalPlayer, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:

	ULyraLocalPlayer();

	//~UObject interface
	virtual void PostInitProperties() override;
	//~End of UObject interface

	//~UPlayer interface
	virtual void SwitchController(class APlayerController* PC) override;
	//~End of UPlayer interface

	//~ULocalPlayer interface
	virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;
	virtual void InitOnlineSession() override;
	//~End of ULocalPlayer interface

	//~ILyraTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface

public:
	UFUNCTION()
	ULyraSettingsLocal* GetLocalSettings() const;

	UFUNCTION()
	ULyraSettingsShared* GetSharedSettings() const;

protected:
	void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);
	
	UFUNCTION()
	void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

private:
	void OnPlayerControllerChanged(APlayerController* NewController);

	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	UPROPERTY(Transient)
	mutable ULyraSettingsShared* SharedSettings;

	UPROPERTY(Transient)
	mutable const UInputMappingContext* InputMappingContext;

	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
};
