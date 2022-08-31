// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditor.h"

#include "AssetRegistryModule.h"

#include "CollectionManagerTypes.h"
#include "ICollectionManager.h"
#include "CollectionManagerModule.h"

#include "HAL/IConsoleManager.h"
#include "Algo/Transform.h"

//////////////////////////////////////////////////////////////////////////

FAutoConsoleCommandWithWorldArgsAndOutputDevice GDiffCollectionReferenceSupport(
	TEXT("Lyra.DiffCollectionReferenceSupport"),
	TEXT("Usage:\n")
	TEXT("  Lyra.DiffCollectionReferenceSupport OldCollectionName NewCollectionName [Deduplicate]\n")
	TEXT("\n")
	TEXT("It will list the assets in Old that 'support' assets introduced in New (are referencers directly/indirectly)\n")
	TEXT("as well as any loose unsupported assets.\n")
	TEXT("\n")
	TEXT("The optional third argument controls whether or not multi-supported assets will be de-duplicated (true) or not (false)"),
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

	if (AssetRegistry.IsLoadingAssets())
	{
		Ar.Log(TEXT("Asset registry is still scanning, try again later"));
		return;
	}

	const bool bExcludeSecondInstanceOfMultiSupported = (Params.Num() >= 3) ? Params[2].ToBool() : true;

	TArray<FName> OldPaths;
	if (!CollectionManager.GetAssetsInCollection(FName(*Params[0]), ECollectionShareType::CST_All, /*out*/ OldPaths))
	{
		Ar.Log(FString::Printf(TEXT("Failed to find collection %s"), *Params[0]));
		return;
	}

	TArray<FName> NewPaths;
	if (!CollectionManager.GetAssetsInCollection(FName(*Params[1]), ECollectionShareType::CST_All, /*out*/ NewPaths))
	{
		Ar.Log(FString::Printf(TEXT("Failed to find collection %s"), *Params[1]));
		return;
	}

	auto ToPackageName = [](FName ObjectName) { return FName(*FPackageName::ObjectPathToPackageName(ObjectName.ToString())); };

	TSet<FName> OldPathSet;
	Algo::Transform(OldPaths, OldPathSet, ToPackageName);

	TSet<FName> NewPathSet;
	Algo::Transform(NewPaths, NewPathSet, ToPackageName);

	TSet<FName> IntroducedAssetSet = NewPathSet.Difference(OldPathSet);

	// Map from added asset to list of assets in old paths that supports it (if any)
	TMap<FName, TSet<FName>> SupportMap;
	TSet<FName> VisitedAssets;

	// Map of count of newly added assets directly/indirectly supported by each supporter asset in the old paths
	TMap<FName, int32> SupporterCountMap;
	TMap<FName, TSet<FName>> SupporterToAddedMap;

	TFunction<void(const FName)> RecursivelyBuildSupport = [&](const FName IntroducedAssetPath)
	{
		// Someone else may have already processed me as part of their dependencies
		if (!VisitedAssets.Contains(IntroducedAssetPath))
		{
			VisitedAssets.Add(IntroducedAssetPath);

			TArray<FName> Referencers;
			AssetRegistry.GetReferencers(IntroducedAssetPath, /*out*/ Referencers);

			for (const FName& Referencer : Referencers)
			{
				if (OldPathSet.Contains(Referencer))
				{
					// Direct support
					SupportMap.FindOrAdd(IntroducedAssetPath).Add(Referencer);
				}
				else
				{
					// Indirect, need to process recursively
					RecursivelyBuildSupport(Referencer);

					// Can now use the supports it indicated to build into our own
					TSet<FName>& MySupports = SupportMap.FindOrAdd(IntroducedAssetPath);
					if (TSet<FName>* pRecuriveReferencers = SupportMap.Find(Referencer))
					{
						MySupports.Append(*pRecuriveReferencers);
					}
				}
			}
		}
	};

	// Find the supporters
	for (const FName& IntroducedAssetPath : IntroducedAssetSet)
	{
		RecursivelyBuildSupport(IntroducedAssetPath);
	}

	// Count the strongest supporters
	for (const auto& KVP : SupportMap)
	{
		const FName SupportedAsset = KVP.Key;
		for (const FName& Supporter : KVP.Value)
		{
			SupporterToAddedMap.FindOrAdd(Supporter).Add(SupportedAsset);
		}
	}

	TSet<FName> AlreadyPrintedOut;

	Ar.Log(TEXT("List of supporters, sorted by count of newly added assets being supported"));
	SupporterCountMap.ValueSort(TGreater<int32>());
	for (const auto& KVP : SupporterToAddedMap)
	{
		const FName Supporter = KVP.Key;

		// Filter to added assets (and exclude ones already printed if we were asked to)
		TArray<FName> AddedAssetsBeingSupported(KVP.Value.Array());
		AddedAssetsBeingSupported = AddedAssetsBeingSupported.FilterByPredicate([&](FName Test) { return IntroducedAssetSet.Contains(Test); });

		int32 IncludingMultisupportCount = AddedAssetsBeingSupported.Num();
		if (bExcludeSecondInstanceOfMultiSupported)
		{
			AddedAssetsBeingSupported = AddedAssetsBeingSupported.FilterByPredicate([&](FName Test)	{ return !AlreadyPrintedOut.Contains(Test); });
		}

		Algo::Sort(AddedAssetsBeingSupported, [](const FName LHS, const FName RHS) { return LHS.LexicalLess(RHS); });

		// Print out the list
		Ar.Log(FString::Printf(TEXT("%s supports %d new assets:"), *Supporter.ToString(), IncludingMultisupportCount));
		for (const FName& AddedAsset : AddedAssetsBeingSupported)
		{
			const int32 AddedAssetSupportCount = SupportMap.FindChecked(AddedAsset).Num();
			Ar.Log(FString::Printf(TEXT("\t%s%s"), *AddedAsset.ToString(), (AddedAssetSupportCount > 1) ? TEXT(" [multi-supported]") : TEXT("")));
			AlreadyPrintedOut.Add(AddedAsset);
		}

		const int32 NumExcludedDueToMultiSupport = IncludingMultisupportCount - AddedAssetsBeingSupported.Num();
		if (NumExcludedDueToMultiSupport > 0)
		{
			Ar.Log(FString::Printf(TEXT("\tAnd %d more that were also supported by a previous supporter"), NumExcludedDueToMultiSupport));
		}
	}

	Ar.Log(TEXT("\n"));
	Ar.Log(TEXT("List of unsupported assets:"));
	for (const FName& AssetName : IntroducedAssetSet)
	{
		if (!SupportMap.Contains(AssetName) || (SupportMap.FindRef(AssetName).Num() == 0))
		{
			Ar.Log(FString::Printf(TEXT("\t%s"), *AssetName.ToString()));
			//@TODO: Check to see if this is instead supported by a primary asset maybe?
		}
	}
}));
