#pragma once

#include "Base.hpp"

namespace Kpp::Utf8
{
    /** @brief Get the byte count of the next unicode character */
    template<typename Iterator>
    [[nodiscard]] constexpr std::uint32_t GetNextCharByteCount(const Iterator from, const Iterator to) noexcept;

    /** @brief Get the next unicode character in utf8 string */
    template<typename Iterator>
    [[nodiscard]] constexpr std::uint32_t GetNextChar(Iterator &from, const Iterator to) noexcept;

    /** @brief Decode an utf8 character */
    template<typename Iterator>
    [[nodiscard]] constexpr std::uint32_t Decode(const Iterator from, const Iterator to) noexcept;
}

template<typename Iterator>
constexpr std::uint32_t Kpp::Utf8::GetNextCharByteCount(const Iterator from, const Iterator to) noexcept
{
    if (from == to)
        return 0u;
    const auto c = std::uint32_t(reinterpret_cast<const std::uint8_t &>(*from));
    return std::min<std::uint32_t>(
        1u + std::uint32_t(c >= 0b11000000u) + std::uint32_t(c >= 0b11100000u) + std::uint32_t(c >= 0b11110000u),
        std::uint32_t(std::distance(from, to))
    );
}

template<typename Iterator>
constexpr std::uint32_t Kpp::Utf8::GetNextChar(Iterator &from, const Iterator to) noexcept
{
    const auto byteCount = GetNextCharByteCount(from, to);
    const auto unicode = Decode(from, from + byteCount);
    from += byteCount;
    return unicode;
}

template<typename Iterator>
constexpr std::uint32_t Kpp::Utf8::Decode(const Iterator from, const Iterator to) noexcept
{
    auto byteCount = std::uint32_t(std::distance(from, to));
    if (!byteCount)
        return 0u;
    const auto c = std::uint32_t(reinterpret_cast<const std::uint8_t &>(*from));
    if (byteCount == 1u)
        return c;
    auto it = from + 1;
    auto res = (c & (0b11111111u >> (byteCount + 1u))) << 6u * (byteCount - 1);
    while (--byteCount) {
        const auto c2 = (std::uint32_t(reinterpret_cast<const std::uint8_t &>(*it)) & 0b00111111u) << 6u * (byteCount - 1);
        res |= c2;
        ++it;
    }
    return res;
}
