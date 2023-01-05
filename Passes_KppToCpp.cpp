#include <iostream>

#include "Passes.hpp"

using namespace clang::ast_matchers;

template<typename NodeType, typename UnderlyingType = std::remove_cvref_t<NodeType>>
    requires std::same_as<UnderlyingType, clang::FunctionDecl> || std::same_as<UnderlyingType, clang::LambdaExpr>
static void TransformFunction(
    clang::Rewriter &rewriter,
    const clang::SourceManager &sourceManager,
    const clang::LangOptions &langOptions,
    const NodeType &node
)
{
    // Query underlying function decl
    const clang::FunctionDecl &func = [](auto &node) -> const clang::FunctionDecl &{
        if constexpr (std::is_same_v<UnderlyingType, clang::LambdaExpr>)
            return *node.getCallOperator();
        else
            return node;
    }(node);

    if (func.isImplicit())
        return;

    // Insert [[nodiscard]] if not a lambda
    if constexpr (!std::is_same_v<UnderlyingType, clang::LambdaExpr>) {
        if (const auto returnType = func.getReturnType(); !returnType.isNull()) {
            const bool isUnknown = returnType->isTemplateTypeParmType() && returnType->isInstantiationDependentType();
            const bool isVoid = returnType->isVoidType();
            const bool alreadyNodiscard = func.hasAttr<clang::WarnUnusedResultAttr>();
            if (!(isUnknown | isVoid | alreadyNodiscard))
                rewriter.InsertTextBefore(func.getBeginLoc(), "[[nodiscard]] ");
        }
    }

    // Special rules for member functions
    if (func.isCXXClassMember()) {
        const auto &method = reinterpret_cast<const clang::CXXMethodDecl &>(func);
        // Insert const
        if (!method.isConst()) {
            llvm::outs() << "BEFORE " << func.getNameAsString() << "\n";
            rewriter.InsertTextAfter(
                TranspilationUtils::GetAttributeInsertLocation(sourceManager, langOptions, node),
                " const"
            );
            llvm::outs() << "AFTER " << func.getNameAsString() << "\n";
        }
    }

    // Insert noexcept
    if (func.getExceptionSpecType() == clang::ExceptionSpecificationType::EST_None) {
        llvm::outs() << "BEFORE2 " << func.getNameAsString() << "\n";
        rewriter.InsertTextAfter(
            TranspilationUtils::GetAttributeInsertLocation(sourceManager, langOptions, node),
            " noexcept"
        );
        llvm::outs() << "AFTER2 " << func.getNameAsString() << "\n";
    }
}

static void TransformVar(
    clang::Rewriter &rewriter,
    const clang::SourceManager &sourceManager,
    const clang::LangOptions &langOptions,
    const clang::VarDecl &node
)
{
    const auto attr = node.getAttr<clang::AnnotateAttr>();
    if (!(attr && attr->getAnnotation() == TranspilationUtils::MutableAnnotation) && !node.getType().isConstQualified()) {
        rewriter.InsertTextAfter(
            node.getBeginLoc(),
            "const "
        );
    }
}

const std::vector<TranspilationPass> KppToCpp::Passes {
    // Transform functions
    TranspilationPass {
        DeclarationMatcher(
            functionDecl(
                allOf(
                    unless(isImplicit()),
                    unless(hasAncestor(cxxRecordDecl(isLambda())))
                )
            ).bind("func")
        ),
        [](const clang::ast_matchers::MatchFinder::MatchResult &result, clang::Rewriter &rewriter)
        {
            TransformFunction(
                rewriter,
                result.Context->getSourceManager(),
                result.Context->getLangOpts(),
                *result.Nodes.getNodeAs<clang::FunctionDecl>("func")
            );
        }
    },
    // Transform lambdas
    TranspilationPass {
        StatementMatcher(
            lambdaExpr(
                hasDescendant(cxxMethodDecl(unless(isNoThrow())))
            ).bind("lambda")
        ),
        [](const clang::ast_matchers::MatchFinder::MatchResult &result, clang::Rewriter &rewriter)
        {
            TransformFunction(
                rewriter,
                result.Context->getSourceManager(),
                result.Context->getLangOpts(),
                *result.Nodes.getNodeAs<clang::LambdaExpr>("lambda")
            );
        }
    },
    // Transform local variables
    TranspilationPass {
        DeclarationMatcher(
            traverse(
                clang::TraversalKind::TK_IgnoreUnlessSpelledInSource,
                varDecl().bind("var")
            )
        ),
        [](const clang::ast_matchers::MatchFinder::MatchResult &result, clang::Rewriter &rewriter)
        {
            TransformVar(
                rewriter,
                result.Context->getSourceManager(),
                result.Context->getLangOpts(),
                *result.Nodes.getNodeAs<clang::VarDecl>("var")
            );
        }
    }
};