// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorValidator_MaterialFunctions.h"
#include "Materials/MaterialFunction.h"
#include "Materials/Material.h"
#include "AssetRegistryModule.h"
#include "Validation/EditorValidator_Load.h"
#include "LyraEditor.h"

#define LOCTEXT_NAMESPACE "EditorValidator"

UEditorValidator_MaterialFunctions::UEditorValidator_MaterialFunctions()
	: Super()
{
}

bool UEditorValidator_MaterialFunctions::CanValidateAsset_Implementation(UObject* InAsset) const
{
	return Super::CanValidateAsset_Implementation(InAsset) && (InAsset ? InAsset->IsA(UMaterialFunction::StaticClass()) : false);
}

EDataValidationResult UEditorValidator_MaterialFunctions::ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors)
{
	UMaterialFunction* MaterialFunction = Cast<UMaterialFunction>(InAsset);
	check(MaterialFunction);

	if (UEditorValidator::ShouldAllowFullValidation())
	{
		// Also load and check all directly referencing materials, as changes may have caused them to fail to compile
		IAssetRegistry& AssetRegistry = IAssetRegistry::GetChecked();

		TSet<FName> AllHardReferencers;

		TArray<FName> PackagesToProcess;
		PackagesToProcess.Add(MaterialFunction->GetOutermost()->GetFName());

		do
		{
			TArray<FName> NextPackagesToProcess;
			for (FName PackageToProcess : PackagesToProcess)
			{
				TArray<FName> HardReferencers;
				AssetRegistry.GetReferencers(PackageToProcess, HardReferencers, UE::AssetRegistry::EDependencyCategory::Package, UE::AssetRegistry::EDependencyQuery::Hard);
				for (FName HardReferencer : HardReferencers)
				{
					if (!AllHardReferencers.Contains(HardReferencer))
					{
						AllHardReferencers.Add(HardReferencer);
						TArray<FAssetData> RefAssets;
						AssetRegistry.GetAssetsByPackageName(HardReferencer, RefAssets, true);
						for (const FAssetData& RefData : RefAssets)
						{
							if (RefData.IsRedirector())
							{
								NextPackagesToProcess.Add(RefData.PackageName);
								break;
							}
						}
					}
				}
			}
			PackagesToProcess = MoveTemp(NextPackagesToProcess);
		} while (PackagesToProcess.Num() > 0);

		for (FName HardReferencer : AllHardReferencers)
		{
			FString HardReferencerStr = HardReferencer.ToString();
			if (!IsInUncookedFolder(HardReferencerStr))
			{
				TArray<FAssetData> ReferencerAssets;
				AssetRegistry.GetAssetsByPackageName(HardReferencer, ReferencerAssets, true);
				for (const FAssetData& ReferencerAssetData : ReferencerAssets)
				{
					if (ReferencerAssetData.AssetClass == UMaterial::StaticClass()->GetFName())
					{
						UE_LOG(LogLyraEditor, Display, TEXT("    Loading referencing material %s"), *HardReferencerStr);

						TArray<FString> WarningsAndErrors;
						if (UEditorValidator_Load::GetLoadWarningsAndErrorsForPackage(HardReferencerStr, WarningsAndErrors))
						{
							for (const FString& WarningOrError : WarningsAndErrors)
							{
								AssetFails(InAsset, FText::FromString(WarningOrError), ValidationErrors);
							}
						}
						break;
					}
				}
			}
		}
	}

	if (GetValidationResult() != EDataValidationResult::Invalid)
	{
		AssetPasses(InAsset);
	}

	return GetValidationResult();
}

#undef LOCTEXT_NAMESPACE