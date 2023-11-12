#include <Kompiler/Lexer.hpp>

#include <gtest/gtest.h>

using namespace Kpp;

static std::ostream &operator<<(std::ostream &os, const Token &token) noexcept
{
    return os << '\'' << token.string() << "' (" << token.type << " l" << token.line << ':' << token.column << ':' << token.length << ')';
}

[[nodiscard]] static constexpr Token MakeToken(const Token::Type type, const std::uint32_t line, const std::uint32_t column, const std::string_view data) noexcept
{
    return Token {
        .type = type,
        .line = line,
        .column = column,
        .length = std::uint32_t(data.length()),
        .data = data.data(),
    };
}

/** @brief Compare two tokens */
[[nodiscard]] static constexpr bool CompareTokens(const Token &lhs, const Token &rhs) noexcept
{
    return lhs.type == rhs.type
        && lhs.line == rhs.line
        && lhs.column == rhs.column
        && lhs.string() == rhs.string();
}

[[nodiscard]] static bool TestTokenize(const std::string_view &input, const std::initializer_list<Token> &expected) noexcept
{
    Lexer lexer;
    Tokens tokens;
    if (auto tokenError = lexer.tokenize(input, tokens); tokenError.error != Lexer::Error::None) {
        std::cerr << "[TestTokenize] Error " << tokenError.error << ": " << tokenError.token << std::endl;
        return false;
    }
    if (tokens.size() != expected.size()) {
        std::cerr << "[TestTokenize] Expected " << expected.size() << " tokens, got " << tokens.size() << std::endl;
        return false;
    }
    for (std::size_t index {}; index != tokens.size(); ++index) {
        if (CompareTokens(tokens[index], expected.begin()[index]))
            continue;
        std::cerr << "[TestTokenize] Expected token " << expected.begin()[index] << ", got " << tokens[index] << std::endl;
        return false;
    }
    return true;
}


[[nodiscard]] static bool TestTokenizeError(const std::string_view &input, const Lexer::Error error, const Token &token) noexcept
{
    Lexer lexer;
    Tokens tokens;
    if (const auto tokenError = lexer.tokenize(input, tokens); tokenError.error != error) {
        std::cerr << "[TestTokenizeError] Expected error " << error << ", got " << tokenError.error << std::endl;
        return false;
    } else if (!CompareTokens(tokenError.token, token)) {
        std::cerr << "[TestTokenizeError] Expected token " << token << ", got " << tokenError.token << std::endl;
        return false;
    }
    return true;
}

TEST(Lexer, Words)
{
    const std::string token1 = "Hɘllo";
    const std::string token2 = "World";

    // Single word
    ASSERT_TRUE(TestTokenize(
        token1,
        {
            MakeToken(Token::Type::Word, 1, 1, token1),
        }
    ));

    // Single word with spaces before & after
    ASSERT_TRUE(TestTokenize(
        " \n\t\n  \n\t" + token1 + "\n\t \t\n  \n",
        {
            MakeToken(Token::Type::Word, 4, 2, token1),
        }
    ));

    // Two words separated by newline
    ASSERT_TRUE(TestTokenize(
        token1 + "\n" + token2,
        {
            MakeToken(Token::Type::Word, 1, 1, token1),
            MakeToken(Token::Type::Word, 2, 1, token2),
        }
    ));

    // Two words separated by space
    ASSERT_TRUE(TestTokenize(
        token1 + " " + token2,
        {
            MakeToken(Token::Type::Word, 1, 1, token1),
            MakeToken(Token::Type::Word, 1, 7, token2),
        }
    ));

    // Two words with spaces before & after each
    ASSERT_TRUE(TestTokenize(
        "\n\t" + token1 + "\n \t" + token2 + "\n\t \n",
        {
            MakeToken(Token::Type::Word, 2, 2, token1),
            MakeToken(Token::Type::Word, 3, 3, token2),
        }
    ));
}

TEST(Lexer, TokenizeStringLiteral)
{
    const std::string stringLiteral1 = "This is a string literal";
    const std::string token1 = '"' + stringLiteral1 + '"';

    // Single string literal
    ASSERT_TRUE(TestTokenize(
        token1,
        {
            MakeToken(Token::Type::Literal, 1, 1, token1),
        }
    ));

    // Single string literal with spaces before & after
    ASSERT_TRUE(TestTokenize(
        " \n\t\n  \n\t" + token1 + "\n\t \t\n  \n",
        {
            MakeToken(Token::Type::Literal, 4, 2, token1),
        }
    ));

    // Two consecutive string literals
    ASSERT_TRUE(TestTokenize(
        token1 + token1,
        {
            MakeToken(Token::Type::Literal, 1, 1, token1),
            MakeToken(Token::Type::Literal, 1, token1.size() + 1, token1),
        }
    ));

    // Two string literals with spaces before & after each
    ASSERT_TRUE(TestTokenize(
        "\n\t" + token1 + "\n \t" + token1 + "\n\t \n",
        {
            MakeToken(Token::Type::Literal, 2, 2, token1),
            MakeToken(Token::Type::Literal, 3, 3, token1),
        }
    ));

    // Infinite string literal
    for (const auto &infinite : { "\"", "\" \n\t", "\"Hello\n\tWorld" }) {
        ASSERT_TRUE(TestTokenizeError(
            infinite,
            Lexer::Error::InfiniteStringLiteral,
            MakeToken(Token::Type::Literal, 1, 1, std::string_view {})
        ));
    }
}