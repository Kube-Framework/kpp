#include <Kompiler/Lexer.hpp>

#include <gtest/gtest.h>

using namespace Kpp;

TEST(Lexer, TokenizeSingleToken)
{
    constexpr std::string_view TokenData = "ꓘudɘ";

    Lexer lexer;
    Tokens tokens;
    ASSERT_EQ(lexer.tokenize(TokenData, tokens).error, Lexer::Error::None);
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens.front().data, TokenData);
    ASSERT_EQ(tokens.front().line, 1);
    ASSERT_EQ(tokens.front().column, 1);
    ASSERT_EQ(tokens.front().length, TokenData.length());
}