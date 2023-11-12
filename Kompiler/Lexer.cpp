#include "Lexer.hpp"

#include "Utf8.hpp"

#include <cctype>

using namespace Kpp;

Lexer::TokenError Lexer::tokenize(const std::string_view &input, Tokens &output) noexcept
{
    // If the input is empty we return Exhausted error
    if (input.empty())
        return TokenError { .error = Error::Exhausted };

    // Prepare input cache
    _head = input.data();
    _tail = input.data() + input.size();
    _line = { 1 };
    _column = {};

    // Exhaust input
    Token token;
    while (true) {
        const auto error = tokenize(token);
        if (error == Error::None) {
            output.emplace_back(std::move(token));
            continue;
        }
        return TokenError {
            .token = token,
            .error = error == Error::Exhausted ? Error::None : error,
        };
    }
    return TokenError { .error = Error::None };
}

Lexer::Error Lexer::tokenize(Token &output) noexcept
{
    // Cache of the last decoded unicode character
    std::uint32_t unicode {};

    // Skip all spaces
    auto lastHead = _head;
    while (true) {
        unicode = getNextDecodedChar();
        // Detect end of file
        if (!unicode)
            return Error::Exhausted;
        // Detect spaces
        else if (!isSpace(unicode))
            break;
        lastHead = _head;
    }

    // Store token location
    output.data = lastHead;
    output.line = _line;
    output.column = _column;

    // Detect a string literal token
    if (isLiteralDelimiter(unicode))
        return tokenizeStringLiteral(output, unicode);
    // Detect a numeric literal token
    else if (isDigit(unicode))
        return tokenizeNumericLiteral(output, unicode);
    // Detect an operator token
    else if (isOperator(unicode))
        return tokenizeOperator(output, unicode);
    // No special token detect, consider it as a word
    else
        return tokenizeWord(output, unicode);
}

Lexer::Error Lexer::tokenizeStringLiteral(Token &output, const std::uint32_t delimiter) noexcept
{
    while (true) {
        const auto unicode = getNextDecodedChar();
        // End character
        if (!unicode) [[unlikely]]
            break;
        // Delimiter not matched
        else if (unicode != delimiter) [[likely]]
            continue;
        // Delimiter matched
        output.length = std::uint32_t(std::distance(output.data, _head));
        return Error::None;
    }
    return Error::InvalidStringLiteral;
}

Lexer::Error Lexer::tokenizeNumericLiteral(Token &output, const std::uint32_t firstUnicode) noexcept
{
    return Error::InvalidNumericLiteral;
}

Lexer::Error Lexer::tokenizeOperator(Token &output, const std::uint32_t firstUnicode) noexcept
{
    return Error::InvalidOperator;
}

Lexer::Error Lexer::tokenizeWord(Token &output, const std::uint32_t firstUnicode) noexcept
{
    while (true) {
        const auto unicode = getNextDecodedChar();
        // Continue when reading word characters
        if (unicode && !isSpace(unicode) && !isLiteralDelimiter(unicode) && !isOperator(unicode)) [[likely]]
            continue;
        // Non word character matched
        output.length = std::uint32_t(std::distance(output.data, _head));
        return Error::None;
    }
    return Error::InvalidWord;
}

inline std::uint32_t Lexer::getNextDecodedChar(void) noexcept
{
    const auto unicode = Utf8::GetNextChar(_head, _tail);
    const auto oldLine = _line;
    _line += unicode == '\n';
    _column = 1u + _column * (_line == oldLine);
    return unicode;
}

inline bool Lexer::isSpace(const std::uint32_t unicode) const noexcept
{
    return std::isspace(std::uint8_t(unicode));
}

inline bool Lexer::isDigit(const std::uint32_t unicode) const noexcept
{
    return std::isdigit(std::uint8_t(unicode));
}

inline bool Lexer::isLiteralDelimiter(const std::uint32_t unicode) const noexcept
{
    return unicode == '\'' | unicode == '"';
}

inline bool Lexer::isOperator(const std::uint32_t unicode) const noexcept
{
    switch (unicode) {
    case '-':
    case ';':
    case ':':
    case '!':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '*':
    case '/':
    case '&':
    case '#':
    case '%':
    case '^':
    case '+':
    case '<':
    case '=':
    case '>':
    case '|':
    case '~':
        return true;
    default:
        return false;
    }
}