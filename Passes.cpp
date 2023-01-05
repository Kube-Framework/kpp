#include "Passes.hpp"

clang::SourceLocation TranspilationUtils::GetAttributeInsertLocation(
    const clang::SourceManager &sourceManager,
    const clang::LangOptions &langOptions,
    const clang::FunctionDecl &node
)
{
    // Get last argument location
    const clang::SourceLocation previousTokenLoc = node.getTypeSpecEndLoc();

    // Token query success
    if (clang::Token token; !clang::Lexer::getRawToken(previousTokenLoc, token, sourceManager, langOptions, true)) [[likely]]
        return token.getEndLoc();
    // Must be a wrong match
    else
        throw std::logic_error("Transpiler::GetAttributeInsertLocation(FunctionDecl): Critical error");
}

clang::SourceLocation TranspilationUtils::GetAttributeInsertLocation(
    const clang::SourceManager &sourceManager,
    const clang::LangOptions &langOptions,
    const clang::LambdaExpr &node
)
{
    const clang::SourceLocation previousTokenLoc = [](const auto &node) {
        // Find the right insert location upon implicit/explicit parameters
        if (node.hasExplicitParameters())
            return node.getCallOperator()->getTypeSpecEndLoc();
        else
            return node.getIntroducerRange().getEnd();
    }(node);


    // Token query success
    if (clang::Token token; !clang::Lexer::getRawToken(previousTokenLoc, token, sourceManager, langOptions, true)) [[likely]]
        return token.getEndLoc();
    // Must be a wrong match
    else
        throw std::logic_error("Transpiler::GetAttributeInsertLocation(LambdaExpr): Critical error");
}