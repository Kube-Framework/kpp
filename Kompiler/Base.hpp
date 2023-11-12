#pragma once

#include <cstdint>
#include <cstdlib>

/** @brief Align a variable / structure to cacheline size */
#define alignas_cacheline alignas(Kpp::CacheLineSize)
#define alignas_double_cacheline alignas(Kpp::CacheLineDoubleSize)
#define alignas_half_cacheline alignas(Kpp::CacheLineHalfSize)
#define alignas_quarter_cacheline alignas(Kpp::CacheLineQuarterSize)
#define alignas_eighth_cacheline alignas(Kpp::CacheLineEighthSize)


/** @brief Helpers used to assert alignment of a structure */
#define static_assert_alignof(Type, Alignment) static_assert(alignof(Type) == Alignment, #Type " must be aligned to " #Alignment)
#define static_assert_alignof_cacheline(Type) static_assert_alignof(Type, Kpp::CacheLineSize)
#define static_assert_alignof_double_cacheline(Type) static_assert_alignof(Type, Kpp::CacheLineDoubleSize)
#define static_assert_alignof_half_cacheline(Type) static_assert_alignof(Type, Kpp::CacheLineHalfSize)
#define static_assert_alignof_quarter_cacheline(Type) static_assert_alignof(Type, Kpp::CacheLineQuarterSize)
#define static_assert_alignof_eighth_cacheline(Type) static_assert_alignof(Type, Kpp::CacheLineEighthSize)

/** @brief Helpers used to assert size of a structure */
#define static_assert_sizeof(Type, Size) static_assert(sizeof(Type) == Size, #Type " must have a size of " #Size)
#define static_assert_sizeof_cacheline(Type) static_assert_sizeof(Type, Kpp::CacheLineSize)
#define static_assert_sizeof_double_cacheline(Type) static_assert_sizeof(Type, Kpp::CacheLineDoubleSize)
#define static_assert_sizeof_half_cacheline(Type) static_assert_sizeof(Type, Kpp::CacheLineHalfSize)
#define static_assert_sizeof_quarter_cacheline(Type) static_assert_sizeof(Type, Kpp::CacheLineQuarterSize)
#define static_assert_sizeof_eighth_cacheline(Type) static_assert_sizeof(Type, Kpp::CacheLineEighthSize)

/** @brief Helpers used to assert that the size and alignment of a structure are equal to themselves and a given value */
#define static_assert_fit(Type, Size) static_assert(sizeof(Type) == alignof(Type) && alignof(Type) == Size, #Type " must have a size of " #Size " and be aligned to " #Size)
#define static_assert_fit_cacheline(Type) static_assert_fit(Type, Kpp::CacheLineSize)
#define static_assert_fit_double_cacheline(Type) static_assert_fit(Type, Kpp::CacheLineDoubleSize)
#define static_assert_fit_half_cacheline(Type) static_assert_fit(Type, Kpp::CacheLineHalfSize)
#define static_assert_fit_quarter_cacheline(Type) static_assert_fit(Type, Kpp::CacheLineQuarterSize)
#define static_assert_fit_eighth_cacheline(Type) static_assert_fit(Type, Kpp::CacheLineEighthSize)


namespace Kpp
{
    /** @brief Theorical cacheline size */
    constexpr std::size_t CacheLineSize = sizeof(std::size_t) * 8;
    constexpr std::size_t CacheLineDoubleSize = CacheLineSize * 2;
    constexpr std::size_t CacheLineHalfSize = CacheLineSize / 2;
    constexpr std::size_t CacheLineQuarterSize = CacheLineSize / 4;
    constexpr std::size_t CacheLineEighthSize = CacheLineSize / 8;
}