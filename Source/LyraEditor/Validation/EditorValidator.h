// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EditorValidatorBase.h"
#include "DataValidationModule.h"

#include "EditorValidator.generated.h"

class FLyraValidationMessageGatherer : public FOutputDevice
{
public:
	FLyraValidationMessageGatherer()
		: FOutputDevice()
	{
		GLog->AddOutputDevice(this);
	}

	virtual ~FLyraValidationMessageGatherer()
	{
		GLog->RemoveOutputDevice(this);
	}

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{
		if (Verbosity <= ELogVerbosity::Warning)
		{
			FString MessageString(V);
			bool bIgnored = false;
			for (const FString& IgnorePattern : IgnorePatterns)
			{
				if (MessageString.Contains(IgnorePattern))
				{
					bIgnored = true;
					break;
				}
			}

			if (!bIgnored)
			{
				AllWarningsAndErrors.Add(MessageString);
				if (Verbosity == ELogVerbosity::Warning)
				{
					AllWarnings.Add(MessageString);
				}
			}
		}
	}

	const TArray<FString>& GetAllWarningsAndErrors() const
	{
		return AllWarningsAndErrors;
	}

	const TArray<FString>& GetAllWarnings() const
	{
		return AllWarnings;
	}

	static void AddIgnorePatterns(const TArray<FString>& NewPatterns)
	{
		IgnorePatterns.Append(NewPatterns);
	}

	static void RemoveIgnorePatterns(const TArray<FString>& PatternsToRemove)
	{
		for (const FString& PatternToRemove : PatternsToRemove)
		{
			IgnorePatterns.RemoveSingleSwap(PatternToRemove);
		}
	}

private:
	TArray<FString> AllWarningsAndErrors;
	TArray<FString> AllWarnings;
	static TArray<FString> IgnorePatterns;
};

UCLASS(Abstract)
class UEditorValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

public:
	UEditorValidator();

	static void ValidateCheckedOutContent(bool bInteractive, const EDataValidationUsecase InValidationUsecase);
	static bool ValidatePackages(const TArray<FString>& ExistingPackageNames, const TArray<FString>& DeletedPackageNames, int32 MaxPackagesToLoad, TArray<FString>& OutAllWarningsAndErrors, const EDataValidationUsecase InValidationUsecase);
	static bool ValidateProjectSettings();

	static bool IsInUncookedFolder(const FString& PackageName, FString* OutUncookedFolderName = nullptr);
	static bool ShouldAllowFullValidation();

	static void GetChangedAssetsForCode(class IAssetRegistry& AssetRegistry, const FString& ChangedHeaderLocalFilename, TArray<FString>& OutChangedPackageNames);

protected:
	virtual bool CanValidateAsset_Implementation(UObject* InAsset) const override;

	static TArray<FString> TestMapsFolders;

private:
	/**
	 * Used by some validators to determine if it is okay to load referencing assets or other slow tasks. 
	 * This is not okay for fast operations like saving, but is fine for slower "check everything thoroughly" tests
	 */
	static bool bAllowFullValidationInEditor;
};