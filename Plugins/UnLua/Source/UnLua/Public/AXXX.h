#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AXXX.generated.h"

UCLASS()
class UAXXX : public UObject
{
public:
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = LevelOfDetail)
    int32 LODInfo;

    UPROPERTY(EditAnywhere, Category = LevelOfDetail)
    int32 _X_XXX_;
};
