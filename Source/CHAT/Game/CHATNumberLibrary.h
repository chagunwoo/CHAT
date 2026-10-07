#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CHATNumberLibrary.generated.h"

USTRUCT(BlueprintType)
struct CHAT_API FCHATAnswerResult
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int32 Strikes = 0;
    UPROPERTY(BlueprintReadOnly) int32 Balls = 0;
    FString ToString() const;
};

UCLASS()
class CHAT_API UCHATNumberLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, Category="CHAT|Rules")
    static bool ValidateInput(const FString& Input, FString& Error);
    UFUNCTION(BlueprintCallable, Category="CHAT|Rules")
    static FString GenerateRandomNumbers();
    UFUNCTION(BlueprintPure, Category="CHAT|Rules")
    static FCHATAnswerResult CheckAnswer(const FString& Secret, const FString& Input);
};
