#pragma once

#include "Transpiler.hpp"

#include <variant>

namespace TranspilationUtils
{
    /** @brief Mutable K++ annotation */
    constexpr auto MutableAnnotation = "kpp-mut";

    /** @brief Get end attribute insert location from a FunctionDecl */
    [[nodiscard]] clang::SourceLocation GetAttributeInsertLocation(
        const clang::SourceManager &sourceManager,
        const clang::LangOptions &langOptions,
        const clang::FunctionDecl &func
    );

    /** @brief Get end attribute insert location from a LambdaExpr */
    [[nodiscard]] clang::SourceLocation GetAttributeInsertLocation(
        const clang::SourceManager &sourceManager,
        const clang::LangOptions &langOptions,
        const clang::LambdaExpr &lambda
    );
}

struct TranspilationPass
{
    using Matcher = std::variant<
        clang::ast_matchers::DeclarationMatcher,
        clang::ast_matchers::TypeMatcher,
        clang::ast_matchers::StatementMatcher,
        clang::ast_matchers::NestedNameSpecifierMatcher,
        clang::ast_matchers::NestedNameSpecifierLocMatcher,
        clang::ast_matchers::TypeLocMatcher,
        clang::ast_matchers::CXXCtorInitializerMatcher,
        clang::ast_matchers::TemplateArgumentLocMatcher,
        clang::ast_matchers::AttrMatcher
    >;

    Matcher matcher;
    TranspilerMatchCallback::Callback callback;
};


struct KppToCpp
{
    /** @brief Get Kpp to Cpp transpiler passes */
    static const std::vector<TranspilationPass> Passes;
};

struct CppToKpp
{
    /** @brief Get Cpp to Kpp transpiler passes */
    static const std::vector<TranspilationPass> Passes;
};