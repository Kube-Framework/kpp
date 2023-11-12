#pragma once

#include "Base.hpp"

#include <string_view>
#include <vector>

namespace Kpp
{
    class Lexer;
    class Token;
    class TokenAllocator;

    /** @brief A token is the result of Lexer tokenize function
     *  @warning Do not use a token's data if the input used to build it has been destroyed */
    struct Token
    {
        /** @brief Token's type */
        enum class Type
        {
            Undefined,
            Word,
            Literal,
            Operator,
            Comment
        };

        Type type;
        std::uint32_t line;
        std::uint32_t column;
        std::uint32_t length;
        const char *data;

        /** @brief Get token string */
        [[nodiscard]] constexpr std::string_view string(void) const noexcept { return std::string_view(data, length); }
    };
    static_assert_sizeof(Token, sizeof(std::size_t) * 3);

    /** @brief A list of tokens */
    using Tokens = std::vector<Token>;
}

/** @brief Lexer parses a string into an array of tokens */
class Kpp::Lexer
{
public:
    /** @brief Lexer specific errors */
    enum class Error
    {
        None, // No error
        Exhausted, // No more data in input stream
        InfiniteStringLiteral, // String literal is not valid
    };

    /** @brief A helpful structure that concatenates a token and an error */
    struct TokenError
    {
        Token token {};
        Error error {};
    };

    /** @brief Tokenize input into parameter token list
     *  @warning All tokens are tied to the input */
    [[nodiscard]] TokenError tokenize(const std::string_view &input, Tokens &output) noexcept;

private:
    /** @brief Tokenize input cache into parameter token */
    [[nodiscard]] Error tokenize(Token &output) noexcept;

    /** @brief Tokenize string literal
     *  @param firstUnicode is the unicode character beginning and thus ending the string literal */
    [[nodiscard]] Error tokenizeStringLiteral(Token &output, const std::uint32_t firstUnicode) noexcept;

    /** @brief Tokenize numeric literal */
    [[nodiscard]] Error tokenizeNumericLiteral(Token &output, const std::uint32_t firstUnicode) noexcept;

    /** @brief Tokenize operator */
    [[nodiscard]] Error tokenizeOperator(Token &output, const std::uint32_t firstUnicode) noexcept;

    /** @brief Tokenize word */
    [[nodiscard]] Error tokenizeWord(Token &output, const std::uint32_t firstUnicode) noexcept;


    /** @brief Traverse decoded characters until parameter callback returns false
     *  @param ConsumeLastChar If true, the traverse will consume the last character refused by the callback */
    template<bool ConsumeLastChar = false, std::invocable<std::uint32_t> Callback>
    void traverseCharacters(Callback &&callback) noexcept;


    /** @brief Get the distance from parameter data to current head */
    [[nodiscard]] inline std::uint32_t distanceToHead(const char * const data) noexcept { return std::uint32_t(std::distance(data, _head)); }


    const char *_head {};
    const char *_tail {};
    std::uint32_t _line {};
    std::uint32_t _column {};
};


#include <iosfwd>

/** @brief Ostream compatibility */
std::ostream &operator<<(std::ostream &os, const Kpp::Token &token) noexcept;
std::ostream &operator<<(std::ostream &os, const Kpp::Token::Type &token) noexcept;
std::ostream &operator<<(std::ostream &os, const Kpp::Lexer::Error &error) noexcept;