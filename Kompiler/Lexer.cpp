#include "Lexer.hpp"

#include <cctype>

using namespace Kpp;

template<typename Iterator>
static constexpr std::uint32_t GetNextCharByteCount(const Iterator from, const Iterator to) noexcept
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
static constexpr std::uint32_t Decode(const Iterator from, const std::uint32_t byteCount) noexcept
{
    const auto c = std::uint32_t(reinterpret_cast<const std::uint8_t &>(*from));
    if (byteCount == 1u)
        return c;
    auto it = from + 1;
    auto res = (c & (0b11111111u >> (byteCount + 1u))) << 6u * (byteCount - 1);
    auto count = byteCount;
    while (--count) {
        const auto c2 = (std::uint32_t(reinterpret_cast<const std::uint8_t &>(*it)) & 0b00111111u) << 6u * (count - 1);
        res |= c2;
        ++it;
    }
    return res;
}


static constexpr bool IsSpace(const std::uint32_t unicode) noexcept
{
    return (unicode == ' ') | (unicode == '\t') | (unicode == '\n');
}

static constexpr bool IsStringLiteralDelimiter(const std::uint32_t unicode) noexcept
{
    return (unicode == '\'') | (unicode == '"');
}

static constexpr bool IsDigit(const std::uint32_t unicode) noexcept
{
    return (unicode >= '0') & (unicode <= '9');
}

static constexpr std::uint32_t ConcatenateOperator(const std::uint32_t lhs, const std::uint32_t rhs) noexcept
{
    return (lhs << 8) | rhs;
}

static constexpr bool IsOperator(const std::uint32_t unicode) noexcept
{
    switch (unicode) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case ';':
    case ',':
    case '?':
    case ':':
    case '#':
    case '@':
    case '~':
    case '.':
    case '+':
    case ConcatenateOperator('+', '='):
    case '-':
    case ConcatenateOperator('-', '='):
    case ConcatenateOperator('-', '>'):
    case '*':
    case ConcatenateOperator('*', '='):
    case '/':
    case ConcatenateOperator('/', '='):
    case '%':
    case ConcatenateOperator('%', '='):
    case '^':
    case ConcatenateOperator('^', '='):
    case '!':
    case ConcatenateOperator('!', '='):
    case '=':
    case ConcatenateOperator('=', '='):
    case '&':
    case ConcatenateOperator('&', '&'):
    case '|':
    case ConcatenateOperator('|', '|'):
    case '<':
    case ConcatenateOperator('<', '='):
    case ConcatenateOperator('<', '<'):
    case ConcatenateOperator(ConcatenateOperator('<', '='), '>'):
    case '>':
    case ConcatenateOperator('>', '='):
    case ConcatenateOperator('>', '>'):
        return true;
    default:
        return false;
    }
}

Lexer::TokenError Lexer::tokenize(const std::string_view &input, Tokens &output) noexcept
{
    // If the input is empty we return Exhausted error
    if (input.empty())
        return TokenError { .error = Error::Exhausted };

    // Prepare input cache
    _head = input.data();
    _tail = input.data() + input.size();
    _line = { 1 };
    _column = { 1 };

    // Exhaust input
    Token token {};
    Error error {};
    while (true) {
        error = tokenize(token);
        if (error == Error::None) [[likely]]
            output.emplace_back(std::move(token));
        else
            break;
    }
    return TokenError {
        .token = token,
        .error = error == Error::Exhausted ? Error::None : error,
    };
}

Lexer::Error Lexer::tokenize(Token &output) noexcept
{
    // Store the first decoded unicode character
    std::uint32_t firstUnicode {};

    // Skip all spaces
    traverseCharacters<true>([this, &output, &firstUnicode](const std::uint32_t unicode) {
        if (IsSpace(unicode))
            return true;
        // Store first unicode & token location
        firstUnicode = unicode;
        output.data = _head;
        output.line = _line;
        output.column = _column;
        return false;
    });
    if (!firstUnicode)
        return Error::Exhausted;

    // Detect a string literal token
    if (IsStringLiteralDelimiter(firstUnicode))
        return tokenizeStringLiteral(output, firstUnicode);
    // Detect a numeric literal token
    else if (IsDigit(firstUnicode))
        return tokenizeNumericLiteral(output, firstUnicode);
    // Detect an operator token
    else if (IsOperator(firstUnicode))
        return tokenizeOperator(output, firstUnicode);
    // No special token detect, consider it as a word
    else
        return tokenizeWord(output, firstUnicode);
}

Lexer::Error Lexer::tokenizeStringLiteral(Token &output, const std::uint32_t firstUnicode) noexcept
{
    // Set output token type
    output.type = Token::Type::Literal;

    // Get string part
    bool delimiterFound {};
    traverseCharacters<true>([this, firstUnicode, &delimiterFound](const std::uint32_t unicode) {
        if (unicode != firstUnicode) [[likely]]
            return true;
        delimiterFound = true;
        return false;
    });

    // Delimiter not found means its an infinite string
    if (!delimiterFound)
        return Error::InfiniteStringLiteral;

    // Get word part
    traverseCharacters([](const auto unicode) {
        return !IsSpace(unicode) && !IsStringLiteralDelimiter(unicode) && !IsOperator(unicode);
    });

    // Store literal length
    output.length = distanceToHead(output.data);
    return Error::None;
}

Lexer::Error Lexer::tokenizeNumericLiteral(Token &output, const std::uint32_t firstUnicode) noexcept
{
    // Set output token type
    output.type = Token::Type::Literal;

    // Get numeric part
    traverseCharacters([hasDot = false](const std::uint32_t unicode) mutable {
        if (!hasDot & (unicode == '.')) [[unlikely]] {
            hasDot = true;
            return true;
        } else if (IsDigit(unicode) | (unicode == '\''))
            return true;
        return false;
    });

    // Get word part
    traverseCharacters([](const auto unicode) {
        return !IsSpace(unicode) && !IsStringLiteralDelimiter(unicode) && !IsOperator(unicode);
    });

    // Store literal length
    output.length = distanceToHead(output.data);
    return Error::None;
}

Lexer::Error Lexer::tokenizeOperator(Token &output, const std::uint32_t firstUnicode) noexcept
{
    // Set output token type
    output.type = Token::Type::Operator;

    // Concatenate until operator doesn't match
    traverseCharacters([op = firstUnicode](const auto unicode) mutable {
        op = ConcatenateOperator(op, unicode);
        return IsOperator(op);
    });

    // Store literal length
    output.length = distanceToHead(output.data);
    return Error::None;
}

Lexer::Error Lexer::tokenizeWord(Token &output, const std::uint32_t) noexcept
{
    // Set output token type
    output.type = Token::Type::Word;

    // Get word
    traverseCharacters([](const auto unicode) {
        return !IsSpace(unicode) && !IsStringLiteralDelimiter(unicode) && !IsOperator(unicode);
    });

    // Store literal length
    output.length = distanceToHead(output.data);
    return Error::None;
}

template<bool ConsumeLastChar, std::invocable<std::uint32_t> Callback>
inline void Lexer::traverseCharacters(Callback &&callback) noexcept
{
    while (true) {
        const auto byteCount = GetNextCharByteCount(_head, _tail);
        if (!byteCount)
            return;
        const auto unicode = Decode(_head, byteCount);
        const bool keepGoing = callback(unicode);
        if (!ConsumeLastChar && !keepGoing)
            return;
        _head += byteCount;
        const auto oldLine = _line;
        _line += unicode == '\n';
        _column = 1u + _column * (_line == oldLine);
        if (ConsumeLastChar && !keepGoing)
            return;
    }
}

#include <ostream>

std::ostream &operator<<(std::ostream &os, const Token::Type &type) noexcept
{
    static constexpr std::string_view Table[] {
        "Undefined",
        "Word",
        "Literal",
        "Operator",
        "Comment",
    };
    return os << Table[std::to_underlying(type)];
}

std::ostream &operator<<(std::ostream &os, const Lexer::Error &error) noexcept
{
    static constexpr std::string_view Table[] {
        "None",
        "Exhausted",
        "InfiniteStringLiteral",
    };
    return os << Table[std::to_underlying(error)];
}