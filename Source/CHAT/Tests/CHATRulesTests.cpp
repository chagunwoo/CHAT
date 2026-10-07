#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Game/CHATNumberLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCHATValidationTest, "CHAT.Rules.Validation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCHATValidationTest::RunTest(const FString& Parameters)
{
    FString Error;
    for (const FString& Input : {FString(TEXT("")), FString(TEXT("12")), FString(TEXT("1234")), FString(TEXT("abc")), FString(TEXT("112")), FString(TEXT("012")), FString(TEXT("1 2")), FString(TEXT("１２３")), FString(TEXT("123\n"))})
    {
        TestFalse(*FString::Printf(TEXT("Reject %s"), *Input), UCHATNumberLibrary::ValidateInput(Input, Error));
        TestFalse(TEXT("Explanation is not empty"), Error.IsEmpty());
    }
    TestTrue(TEXT("Accept 386"), UCHATNumberLibrary::ValidateInput(TEXT("386"), Error));
    TestTrue(TEXT("Valid input clears error"), Error.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCHATScoringTest, "CHAT.Rules.Scoring", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCHATScoringTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("OUT"), UCHATNumberLibrary::CheckAnswer(TEXT("386"), TEXT("127")).ToString(), FString(TEXT("OUT")));
    TestEqual(TEXT("Ball"), UCHATNumberLibrary::CheckAnswer(TEXT("386"), TEXT("167")).ToString(), FString(TEXT("0S1B")));
    TestEqual(TEXT("Win"), UCHATNumberLibrary::CheckAnswer(TEXT("386"), TEXT("386")).ToString(), FString(TEXT("3S0B")));
    TestEqual(TEXT("Mixed"), UCHATNumberLibrary::CheckAnswer(TEXT("386"), TEXT("368")).ToString(), FString(TEXT("1S2B")));
    TestEqual(TEXT("All balls"), UCHATNumberLibrary::CheckAnswer(TEXT("386"), TEXT("863")).ToString(), FString(TEXT("0S3B")));
    TArray<FString> Values;
    for (int32 A = 1; A <= 9; ++A)
        for (int32 B = 1; B <= 9; ++B)
            for (int32 C = 1; C <= 9; ++C)
                if (A != B && A != C && B != C) Values.Add(FString::Printf(TEXT("%d%d%d"), A, B, C));
    for (const FString& Secret : Values)
        for (const FString& Guess : Values)
        {
            int32 Exact = 0, Common = 0;
            for (int32 I = 0; I < 3; ++I)
            {
                Exact += Secret[I] == Guess[I] ? 1 : 0;
                for (int32 J = 0; J < 3; ++J) Common += Secret[I] == Guess[J] ? 1 : 0;
            }
            const FCHATAnswerResult Actual = UCHATNumberLibrary::CheckAnswer(Secret, Guess);
            if (Actual.Strikes != Exact || Actual.Balls != Common - Exact)
            {
                AddError(Secret + TEXT(" vs ") + Guess);
                return false;
            }
        }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCHATRandomTest, "CHAT.Rules.RandomNumbers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCHATRandomTest::RunTest(const FString& Parameters)
{
    FString Error;
    for (int32 I = 0; I < 1000; ++I)
        if (!UCHATNumberLibrary::ValidateInput(UCHATNumberLibrary::GenerateRandomNumbers(), Error))
        { AddError(Error); return false; }
    return true;
}
#endif
