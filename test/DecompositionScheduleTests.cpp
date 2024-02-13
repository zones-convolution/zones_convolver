#include "LambdaPtrConverter.h"

#include <catch2/catch_test_macros.hpp>
#include <zones_convolver/util/DecompositionSchedule.h>

// template <typename Lambda>
// DecompositionSchedule::DecompositionFunction LambdaToDecompositionFunction (Lambda && pFun)
//{
//     return LambdaPtrConverter<
//                Lambda,
//                std::function<void (std::complex<float> *, std::size_t, std::size_t,
//                std::size_t)>, void, DecompositionSchedule::DecompositionFunction> (pFun)
//         .Ptr ();
// }
