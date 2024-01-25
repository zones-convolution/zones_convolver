#pragma once

#include <functional>

/**
 * Implementation taken from
 * https://stackoverflow.com/questions/7852101/c-lambda-with-captures-as-a-function-pointer
 * needed for converting lambdas with captures to c style function pointers for use in testing.
 */
template <typename TLambda, typename TStdFunction, typename TRet, typename TCFunction>
struct LambdaPtrConverter
{
public:
    explicit LambdaPtrConverter (TLambda && pFun)
    {
        StdFun (&pFun);
    }
    explicit LambdaPtrConverter (TLambda & pFun)
    {
        StdFun (&pFun);
    }

protected:
    static TStdFunction StdFun (TLambda * pFun = nullptr)
    {
        static TStdFunction s_function;
        if (pFun)
            s_function = *pFun;
        return s_function;
    }
    template <typename... _Types>
    static TRet CFun (_Types... Args)
    {
        return StdFun () (std::forward<_Types> (Args)...);
    }

public:
    static TCFunction Ptr ()
    {
        return (TCFunction) (CFun);
    }
};