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
            Word,
            Operator,
            Literal,
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
        InvalidStringLiteral, // String literal is not valid
        InvalidNumericLiteral, // Numeric literal is not valid
        InvalidOperator, // Operator is not valid
        InvalidWord, // Word is not valid
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
     *  @param delimiter is the unicode character ending the string literal
    */
    [[nodiscard]] Error tokenizeStringLiteral(Token &output, const std::uint32_t delimiter) noexcept;

    /** @brief Tokenize numeric literal */
    [[nodiscard]] Error tokenizeNumericLiteral(Token &output, const std::uint32_t firstUnicode) noexcept;

    /** @brief Tokenize operator */
    [[nodiscard]] Error tokenizeOperator(Token &output, const std::uint32_t firstUnicode) noexcept;

    /** @brief Tokenize word */
    [[nodiscard]] Error tokenizeWord(Token &output, const std::uint32_t firstUnicode) noexcept;


    /** @brief Get next decoded character from input cache */
    [[nodiscard]] std::uint32_t getNextDecodedChar(void) noexcept;

    /** @brief Get next raw character from input cache */
    [[nodiscard]] inline char peekNextRawChar(void) const noexcept { return *_head; }


    /** @brief Check if unicode is a space */
    [[nodiscard]] bool isSpace(const std::uint32_t unicode) const noexcept;

    /** @brief Check if unicode is a digit */
    [[nodiscard]] bool isDigit(const std::uint32_t unicode) const noexcept;

    /** @brief Check if unicode is a literal delimiter */
    [[nodiscard]] bool isLiteralDelimiter(const std::uint32_t unicode) const noexcept;

    /** @brief Check if unicode is an operator */
    [[nodiscard]] bool isOperator(const std::uint32_t unicode) const noexcept;


    const char *_head {};
    const char *_tail {};
    std::uint32_t _line {};
    std::uint32_t _column {};
};