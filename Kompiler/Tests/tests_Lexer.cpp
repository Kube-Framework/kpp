#include <Kompiler/Lexer.hpp>

#include <gtest/gtest.h>

#include <codecvt>

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

/** @brief Compare two Tokens */
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
    Tokens Tokens;
    if (auto tokenError = lexer.tokenize(input, Tokens); tokenError.error != Lexer::Error::None) {
        std::cerr << "[TestTokenize] Error " << tokenError.error << ": " << tokenError.token << " | '" << input << '\'' << std::endl;
        return false;
    }
    if (Tokens.size() != expected.size()) {
        std::cerr << "[TestTokenize] Expected " << expected.size() << " Tokens, got " << Tokens.size() << " | '" << input << '\'' << std::endl;
        return false;
    }
    for (std::size_t index {}; index != Tokens.size(); ++index) {
        if (CompareTokens(Tokens[index], expected.begin()[index]))
            continue;
        std::cerr << "[TestTokenize] Expected token " << expected.begin()[index] << ", got " << Tokens[index] << " | '" << input << '\'' << std::endl;
        return false;
    }
    return true;
}

[[nodiscard]] static bool TestTokenizeError(const std::string_view &input, const Lexer::Error error, const Token &token) noexcept
{
    Lexer lexer;
    Tokens Tokens;
    if (const auto tokenError = lexer.tokenize(input, Tokens); tokenError.error != error) {
        std::cerr << "[TestTokenizeError] Expected error " << error << ", got " << tokenError.error << " | '" << input << '\'' << std::endl;
        return false;
    } else if (!CompareTokens(tokenError.token, token)) {
        std::cerr << "[TestTokenizeError] Expected token " << token << ", got " << tokenError.token << " | '" << input << '\'' << std::endl;
        return false;
    }
    return true;
}

/** @brief Space utility structure */
struct SpaceMetrics
{
    std::uint32_t line {};
    std::uint32_t column {};

    /** @brief Make a space */
    [[nodiscard]] static inline SpaceMetrics Make(const std::string &data) noexcept
    {
        SpaceMetrics spaceMetrics {};
        const auto wide = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(data);
        for (const auto c : wide) {
            if (c == '\n') {
                ++spaceMetrics.line;
                spaceMetrics.column = {};
            } else
                ++spaceMetrics.column;
        }
        ++spaceMetrics.line;
        ++spaceMetrics.column;
        return spaceMetrics;
    }
};

/** @brief Predefined spaces */
static const std::vector<std::string> Spaces {
    " ",
    "\t",
    "\n",
    " \t",
    "\n ",
    " \n",
    " \t\n",
    "\n\t\n",
    "\n\t   ",
    "\n \t \n",
    "\n\n\n \n",
    "\n\t\t  \n",
};

TEST(Lexer, Words)
{
    static const std::vector<std::string> Tokens {
        "Hɘllo",
        "World",
    };

    // Single word
    for (const auto &token : Tokens) {
        ASSERT_TRUE(TestTokenize(
            token,
            {
                MakeToken(Token::Type::Word, 1, 1, token),
            }
        ));
    }

    // Single word with spaces before & after
    for (std::size_t spaceIndex {}; spaceIndex + 1 < Spaces.size(); ++spaceIndex) {
        for (const auto &token : Tokens) {
            std::string input = Spaces[spaceIndex];
            const auto tokenSpaceMetrics = SpaceMetrics::Make(input);
            input += token + Spaces[spaceIndex + 1];
            ASSERT_TRUE(TestTokenize(
                input,
                {
                    MakeToken(Token::Type::Word, tokenSpaceMetrics.line, tokenSpaceMetrics.column, token),
                }
            ));
        }
    }

    // Two words with spaces before & after each
    for (std::size_t spaceIndex {}; spaceIndex + 2 < Spaces.size(); ++spaceIndex) {
        for (const auto &lhs : Tokens) {
            for (const auto &rhs : Tokens) {
                std::string input = Spaces[spaceIndex];
                const auto lhsSpaceMetrics = SpaceMetrics::Make(input);
                input += lhs + Spaces[spaceIndex + 1];
                const auto rhsSpaceMetrics = SpaceMetrics::Make(input);
                input += rhs + Spaces[spaceIndex + 2];
                ASSERT_TRUE(TestTokenize(
                    input,
                    {
                        MakeToken(Token::Type::Word, lhsSpaceMetrics.line, lhsSpaceMetrics.column, lhs),
                        MakeToken(Token::Type::Word, rhsSpaceMetrics.line, rhsSpaceMetrics.column, rhs),
                    }
                ));
            }
        }
    }
}

TEST(Lexer, TokenizeStringLiteral)
{
    static const std::vector<std::string> Tokens {
        "\"This is a string literal\"",
        "\"This\nis\na\nmulti-line\nstring\nliteral\"",
        "\"0x12CCFF\"_color",
    };
    static const std::vector<std::string> Infinites {
        "\"",
        "\" \n\t",
        "\"Hello\n\tWorld"
    };

    // Single string literal
    for (const auto &token : Tokens) {
        ASSERT_TRUE(TestTokenize(
            token,
            {
                MakeToken(Token::Type::Literal, 1, 1, token),
            }
        ));
    }

    // Single string literal with spaces before & after
    for (std::size_t spaceIndex {}; spaceIndex + 1 < Spaces.size(); ++spaceIndex) {
        for (const auto &token : Tokens) {
            std::string input = Spaces[spaceIndex];
            const auto tokenSpaceMetrics = SpaceMetrics::Make(input);
            input += token + Spaces[spaceIndex + 1];
            ASSERT_TRUE(TestTokenize(
                input,
                {
                    MakeToken(Token::Type::Literal, tokenSpaceMetrics.line, tokenSpaceMetrics.column, token),
                }
            ));
        }
    }

    // Two consecutive string literals
    for (const auto &lhs : Tokens) {
        for (const auto &rhs : Tokens) {
            const auto rhsSpaceMetrics = SpaceMetrics::Make(lhs);
            ASSERT_TRUE(TestTokenize(
                lhs + rhs,
                {
                    MakeToken(Token::Type::Literal, 1, 1, lhs),
                    MakeToken(Token::Type::Literal, rhsSpaceMetrics.line, rhsSpaceMetrics.column, rhs),
                }
            ));
        }
    }

    // Two string literals with spaces before & after each
    for (std::size_t spaceIndex {}; spaceIndex + 2 < Spaces.size(); ++spaceIndex) {
        for (const auto &lhs : Tokens) {
            for (const auto &rhs : Tokens) {
                std::string input = Spaces[spaceIndex];
                const auto lhsSpaceMetrics = SpaceMetrics::Make(input);
                input += lhs + Spaces[spaceIndex + 1];
                const auto rhsSpaceMetrics = SpaceMetrics::Make(input);
                input += rhs + Spaces[spaceIndex + 2];
                ASSERT_TRUE(TestTokenize(
                    input,
                    {
                        MakeToken(Token::Type::Literal, lhsSpaceMetrics.line, lhsSpaceMetrics.column, lhs),
                        MakeToken(Token::Type::Literal, rhsSpaceMetrics.line, rhsSpaceMetrics.column, rhs),
                    }
                ));
            }
        }
    }

    // Infinite string literal
    for (const auto &infinite : Infinites) {
        ASSERT_TRUE(TestTokenizeError(
            infinite,
            Lexer::Error::InfiniteStringLiteral,
            MakeToken(Token::Type::Literal, 1, 1, std::string_view {})
        ));
    }
}

TEST(Lexer, TokenizeNumericLiteral)
{
    static const std::vector<std::string> Tokens {
        "0",
        "1.0f",
        "123456.789",
        "0b0101",
        "0xFFCCAA",
        "42_hash",
    };

    // Single numeric literal
    for (const auto &token : Tokens) {
        ASSERT_TRUE(TestTokenize(
            token,
            {
                MakeToken(Token::Type::Literal, 1, 1, token),
            }
        ));
    }

    // Single string literal with spaces before & after
    for (std::size_t spaceIndex {}; spaceIndex + 1 < Spaces.size(); ++spaceIndex) {
        for (const auto &token : Tokens) {
            std::string input = Spaces[spaceIndex];
            const auto tokenSpaceMetrics = SpaceMetrics::Make(input);
            input += token + Spaces[spaceIndex + 1];
            ASSERT_TRUE(TestTokenize(
                input,
                {
                    MakeToken(Token::Type::Literal, tokenSpaceMetrics.line, tokenSpaceMetrics.column, token),
                }
            ));
        }
    }

    // Two numeric literals with spaces before & after each
    for (std::size_t spaceIndex {}; spaceIndex + 2 < Spaces.size(); ++spaceIndex) {
        for (const auto &lhs : Tokens) {
            for (const auto &rhs : Tokens) {
                std::string input = Spaces[spaceIndex];
                const auto lhsSpaceMetrics = SpaceMetrics::Make(input);
                input += lhs + Spaces[spaceIndex + 1];
                const auto rhsSpaceMetrics = SpaceMetrics::Make(input);
                input += rhs + Spaces[spaceIndex + 2];
                ASSERT_TRUE(TestTokenize(
                    input,
                    {
                        MakeToken(Token::Type::Literal, lhsSpaceMetrics.line, lhsSpaceMetrics.column, lhs),
                        MakeToken(Token::Type::Literal, rhsSpaceMetrics.line, rhsSpaceMetrics.column, rhs),
                    }
                ));
            }
        }
    }
}

TEST(Lexer, TokenizeOperator)
{
    static const std::vector<std::string> Tokens {
        "(",
        ")",
        "[",
        "]",
        "{",
        "}",
        ";",
        ",",
        "?",
        ":",
        "#",
        "@",
        "~",
        ".",
        "+",
        "+=",
        "-",
        "-=",
        "->",
        "*",
        "*=",
        "/",
        "/=",
        "%",
        "%=",
        "^",
        "^=",
        "!",
        "!=",
        "=",
        "==",
        "&",
        "&&",
        "|",
        "||",
        "<",
        "<=",
        "<<",
        "<=>",
        ">",
        ">=",
        ">>",
    };

    // Single operator
    for (const auto &token : Tokens) {
        ASSERT_TRUE(TestTokenize(
            token,
            {
                MakeToken(Token::Type::Operator, 1, 1, token),
            }
        ));
    }

    // Single operator with spaces before & after
    for (std::size_t spaceIndex {}; spaceIndex + 1 < Spaces.size(); ++spaceIndex) {
        for (const auto &token : Tokens) {
            std::string input = Spaces[spaceIndex];
            const auto tokenSpaceMetrics = SpaceMetrics::Make(input);
            input += token + Spaces[spaceIndex + 1];
            ASSERT_TRUE(TestTokenize(
                input,
                {
                    MakeToken(Token::Type::Operator, tokenSpaceMetrics.line, tokenSpaceMetrics.column, token),
                }
            ));
        }
    }

    // Two operators with spaces before & after each
    for (std::size_t spaceIndex {}; spaceIndex + 2 < Spaces.size(); ++spaceIndex) {
        for (const auto &lhs : Tokens) {
            for (const auto &rhs : Tokens) {
                std::string input = Spaces[spaceIndex];
                const auto lhsSpaceMetrics = SpaceMetrics::Make(input);
                input += lhs + Spaces[spaceIndex + 1];
                const auto rhsSpaceMetrics = SpaceMetrics::Make(input);
                input += rhs + Spaces[spaceIndex + 2];
                ASSERT_TRUE(TestTokenize(
                    input,
                    {
                        MakeToken(Token::Type::Operator, lhsSpaceMetrics.line, lhsSpaceMetrics.column, lhs),
                        MakeToken(Token::Type::Operator, rhsSpaceMetrics.line, rhsSpaceMetrics.column, rhs),
                    }
                ));
            }
        }
    }
}