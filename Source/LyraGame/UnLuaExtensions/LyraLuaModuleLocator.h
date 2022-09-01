#pragma once

#include "LuaModuleLocator.h"
#include "LyraLuaModuleLocator.generated.h"

UCLASS()
class ULyraLuaModuleLocator : public ULuaModuleLocator
{
	GENERATED_BODY()

public:
	virtual FString Locate(const UObject* Object) override;

private:
	TMap<FName, FString> Cache;
};
