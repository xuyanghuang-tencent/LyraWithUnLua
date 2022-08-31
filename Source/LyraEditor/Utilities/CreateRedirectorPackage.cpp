// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditor.h"

#include "AssetRegistryModule.h"

#include "CollectionManagerTypes.h"
#include "ICollectionManager.h"
#include "CollectionManagerModule.h"

#include "HAL/IConsoleManager.h"
#include "Algo/Transform.h"

//////////////////////////////////////////////////////////////////////////

FAutoConsoleCommandWithWorldArgsAndOutputDevice GCreateRedirectorPackage(
	TEXT("Lyra.CreateRedirectorPackage"),
	TEXT("Usage:\n")
	TEXT("  Lyra.CreateRedirectorPackage RedirectorName TargetPackage"),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(
		[](const TArray<FString>& Params, UWorld* World, FOutputDevice& Ar)
{
	IAssetRegistry& AssetRegistry = IAssetRegistry::GetChecked();;
	ICollectionManager& CollectionManager = FCollectionManagerModule::GetModule().Get();

	if (Params.Num() < 2)
	{
		Ar.Log(TEXT("Expected two parameters"));
		return;
	}

	const FString RedirectorPackageName = Params[0];
	const FString TargetPackageName = Params[1];

	if (!FPackageName::IsValidLongPackageName(RedirectorPackageName))
	{
		Ar.Log(FString::Printf(TEXT("RedirectorPackageName '%s' is not a valid long package name"), *RedirectorPackageName));
		return;
	}

	if (!FPackageName::IsValidLongPackageName(TargetPackageName))
	{
		Ar.Log(FString::Printf(TEXT("TargetPackageName '%s' is not a valid long package name"), *TargetPackageName));
		return;
	}


	UObject* TargetAsset = StaticLoadObject(UObject::StaticClass(), nullptr, *TargetPackageName);

	if (!FPackageName::IsValidLongPackageName(TargetPackageName))
	{
		Ar.Log(FString::Printf(TEXT("Could not find an asset to point to at '%s'"), *TargetPackageName));
		return;
	}

	// Create the package for the redirector
	const FString RedirectorObjectName = FPackageName::GetLongPackageAssetName(RedirectorPackageName);
	UPackage* RedirectorPackage = CreatePackage(*RedirectorPackageName);

	// Create the redirector itself
	UObjectRedirector* Redirector = NewObject<UObjectRedirector>(RedirectorPackage, FName(*RedirectorObjectName), RF_Standalone | RF_Public);
	Redirector->DestinationObject = TargetAsset;


	FAssetRegistryModule::AssetCreated(Redirector);
	
}));
