#include "Game/CHATNumberLibrary.h"

FString FCHATAnswerResult::ToString() const
{
    return Strikes == 0 && Balls == 0 ? TEXT("OUT") : FString::Printf(TEXT("%dS%dB"), Strikes, Balls);
}

bool UCHATNumberLibrary::ValidateInput(const FString& Input, FString& Error)
{
    Error.Reset();
    if (Input.Len() != 3)
    {
        Error = TEXT("3자리 숫자를 입력해주세요. 다시 입력하세요.");
        return false;
    }
    TSet<TCHAR> Digits;
    for (TCHAR Digit : Input)
    {
        if (Digit < TEXT('1') || Digit > TEXT('9'))
        {
            Error = TEXT("1~9의 숫자만 입력해주세요. 다시 입력하세요.");
            return false;
        }
        if (Digits.Contains(Digit))
        {
            Error = TEXT("중복되지 않은 숫자를 입력해주세요. 다시 입력하세요.");
            return false;
        }
        Digits.Add(Digit);
    }
    return true;
}

FString UCHATNumberLibrary::GenerateRandomNumbers()
{
    TArray<TCHAR> Digits = {TEXT('1'), TEXT('2'), TEXT('3'), TEXT('4'), TEXT('5'), TEXT('6'), TEXT('7'), TEXT('8'), TEXT('9')};
    FString Result;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        const int32 Pick = FMath::RandRange(0, Digits.Num() - 1);
        Result.AppendChar(Digits[Pick]);
        Digits.RemoveAtSwap(Pick);
    }
    return Result;
}

FCHATAnswerResult UCHATNumberLibrary::CheckAnswer(const FString& Secret, const FString& Input)
{
    FCHATAnswerResult Result;
    FString Error;
    if (!ValidateInput(Secret, Error) || !ValidateInput(Input, Error)) return Result;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (Input[Index] == Secret[Index]) ++Result.Strikes;
        else if (Secret.Contains(FString::Chr(Input[Index]))) ++Result.Balls;
    }
    return Result;
}
