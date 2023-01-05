#pragma once

#include <iostream>

#include "ClangTooling.hpp"

class FormatVisitor : public clang::RecursiveASTVisitor<FormatVisitor>
{
public:
    struct Cache
    {
        std::string indent {};
    };

    FormatVisitor(clang::ASTContext *context)
        : _context(*context)
        , _sourceManager(context->getSourceManager())
        , _langOptions(context->getLangOpts())
        , _out(std::cout) {}

    bool TraverseDecl(clang::Decl *node);
    bool TraverseFunctionDecl(clang::FunctionDecl *node);

    bool TraverseStmt(clang::Stmt *node);
    bool TraverseType(clang::QualType node);

private:
    /** @brief Get the spelling of a token at source location */
    [[nodiscard]] std::string getTokenSpelling(const clang::SourceLocation location);


    clang::ASTContext &_context;
    const clang::SourceManager &_sourceManager;
    const clang::LangOptions &_langOptions;
    std::ostream &_out;
    Cache _cache {};
};

class FormatConsumer : public clang::ASTConsumer
{
public:
    FormatConsumer(clang::ASTContext *context) : _visitor(context) {}

    virtual void HandleTranslationUnit(clang::ASTContext &context);

private:
    FormatVisitor _visitor;
};

class FormatAction : public clang::SyntaxOnlyAction
{
public:
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile)
        { return std::make_unique<FormatConsumer>(&Compiler.getASTContext()); }
};