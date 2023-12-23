#pragma once

#include "Lexer.hpp"

#include <variant>

namespace Kpp
{
    class Parser;

    /** @brief A literal */
    struct Literal
    {

    };

    /** @brief An identifier */
    struct Identifier
    {
        struct Type
        {

        };
    };

    /** @brief A statement */
    struct Statement
    {
        struct Qualified

    };

    /** @brief An expression */
    struct Expression
    {

    };

    /** @brief A declaration */
    struct Declaration
    {

    };

    // Undefined,
    // Expression,
    // Identifier,
    // Declaration,
    // InspectExpression,
    // Literal,

    /** @brief Parser node */
    struct PrimaryNode
    {
        /** @brief Node type */
        enum class Type
        {
            Undefined,
            Expression,
            Identifier,
            Declaration,
            InspectExpression,
            Literal,
        };
    };
}

/** @brief Parser build an abstract syntax tree (Node) from a list of tokens */
class Kpp::Parser
{
public:
    /** @brief Parser specific errors */
    enum class Error
    {
        None, // No error
    };

    /** @brief A helpful structure that concatenates a node and an error */
    struct NodeError
    {
        Node *node {};
        Error error {};
    };

    /** @brief Parse token list input into the output AST root node parameter
     *  @warning All nodes are tied to the input tokens
     *  @note The output will get overwrited */
    [[nodiscard]] NodeError parse(const Tokens &input, Node &output) noexcept;

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
std::ostream &operator<<(std::ostream &os, const Kpp::Node &node) noexcept;
std::ostream &operator<<(std::ostream &os, const Kpp::Node::Type &type) noexcept;
std::ostream &operator<<(std::ostream &os, const Kpp::Lexer::Error &error) noexcept;