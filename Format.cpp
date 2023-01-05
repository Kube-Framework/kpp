#include "Format.hpp"

void FormatConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
    // const auto *comments = context.Comments.getCommentsInFile(context.getSourceManager().getMainFileID());
    // if (comments) {
    //     for (auto it = comments->begin(); it != comments->end(); it++) {
    //         clang::RawComment* comment = it->second;
    //         std::string source = comment->getFormattedText(context.getSourceManager(), context.getDiagnostics());
    //         llvm::outs() << "Comment: " << source << "\n";
    //     }
    //     llvm::outs() << "----\n";
    // }

    _visitor.TraverseDecl(context.getTranslationUnitDecl());
}

bool FormatVisitor::TraverseDecl(clang::Decl *node)
{
    // if (const clang::RawComment* comment = node->getASTContext().getRawCommentForDeclNoCache(node); comment)
    //     _out << _cache.indent << comment->getRawText(_sourceManager).operator std::string_view() << '\n';
    return RecursiveASTVisitor::TraverseDecl(node);
}

bool FormatVisitor::TraverseFunctionDecl(clang::FunctionDecl *node)
{
    _out
    // Attributes
        // << node->getA
    // Return
        << node->getReturnType().getAsString() << ' '
    // Name
        << getTokenSpelling(node->getLocation());
    { // Arguments
        _out << '(';
        const auto count = node->getNumParams();
        if (!count) {
            _out << "void";
        } else {
            for (auto i = 0u; i != count; ++i) {
                if (i)
                    _out << ", ";
                clang::ParmVarDecl *paramNode = node->getParamDecl(i);
                _out << paramNode->getType().getAsString() << ' ' << paramNode->getName().operator std::string_view();
            }
        }
        _out << ')';
    }
    { // Specifiers
        // Const specifier
        if (node->isCXXClassMember()) {
            clang::CXXMethodDecl *memberNode = reinterpret_cast<clang::CXXMethodDecl *>(node);
            if (memberNode->isConst())
                _out << " const";
        }
        // Exception specifier
        switch (node->getExceptionSpecType()) {
        case clang::ExceptionSpecificationType::EST_None:
            break;
        case clang::ExceptionSpecificationType::EST_DynamicNone:
            _out << " throw()";
            break;
        case clang::ExceptionSpecificationType::EST_MSAny:
            _out << " throw(...)";
            break;
        case clang::ExceptionSpecificationType::EST_NoThrow:
            _out << " __declspec(nothrow)";
            break;
        case clang::ExceptionSpecificationType::EST_BasicNoexcept:
            _out << " noexcept";
            break;
        default:
            const auto range = node->getExceptionSpecSourceRange();
            const auto sourceText = clang::Lexer::getSourceText(clang::CharSourceRange::getCharRange(range), _sourceManager, _langOptions);
            _out << ' ' << sourceText.operator std::string_view() << ')';
            break;
        }
    }

    // Function body
    _out << '\n';

    if (node->isThisDeclarationADefinition() && !node->isDefaulted()) {
        for (auto *usingShadowDecl : node->decls()) {
            if (llvm::isa<clang::UsingShadowDecl>(usingShadowDecl)) {
                FormatVisitor::TraverseDecl(usingShadowDecl);
            }
        }
        FormatVisitor::TraverseStmt(node->getBody());
    }

    return true;
}

bool FormatVisitor::TraverseStmt(clang::Stmt *node)
{
    switch (node->getStmtClass()) {
    case clang::Stmt::CompoundStmtClass:
    {
        _out << _cache.indent << "{\n";
        _cache.indent.push_back('\t');
        RecursiveASTVisitor::TraverseCompoundStmt(reinterpret_cast<clang::CompoundStmt *>(node));
        _cache.indent.pop_back();
        _out << _cache.indent << "}\n";
        break;
    }
    case clang::Stmt::StmtClass::ReturnStmtClass:
        _out << _cache.indent << "return ";
        RecursiveASTVisitor::TraverseReturnStmt(reinterpret_cast<clang::ReturnStmt *>(node));
        _out << ";\n";
        break;
    case clang::Stmt::IntegerLiteralClass:
        _out << getTokenSpelling(node->getBeginLoc());
        break;
    case clang::Stmt::BinaryOperatorClass:
    {
        const auto binaryNode = reinterpret_cast<clang::BinaryOperator *>(node);
        FormatVisitor::TraverseStmt(binaryNode->getLHS());
        _out << ' ' << binaryNode->getOpcodeStr().operator std::string_view() << ' ';
        FormatVisitor::TraverseStmt(binaryNode->getRHS());
        break;
    }
    case clang::Stmt::ParenExprClass:
    {
        const auto parenNode = reinterpret_cast<clang::ParenExpr *>(node);
        _out << '(';
        FormatVisitor::TraverseStmt(parenNode->getSubExpr());
        _out << ')';
        break;
    }
    case clang::Stmt::RecoveryExprClass:
        RecursiveASTVisitor::TraverseRecoveryExpr(reinterpret_cast<clang::RecoveryExpr *>(node));
        break;
    case clang::Stmt::UnresolvedLookupExprClass:
        RecursiveASTVisitor::TraverseUnresolvedLookupExpr(reinterpret_cast<clang::UnresolvedLookupExpr *>(node));
        break;
    default:
        _out << _cache.indent << "[" << node->getStmtClassName() << "]" << std::endl;
        break;
    }
    return true;
}

bool FormatVisitor::TraverseType(clang::QualType node)
{
    _out << node.getAsString()  << std::endl;
    return RecursiveASTVisitor::TraverseType(node);
}

std::string FormatVisitor::getTokenSpelling(const clang::SourceLocation location)
{
    std::string spelling;
    clang::Token token;

    if (!clang::Lexer::getRawToken(location, token, _sourceManager, _langOptions))
        spelling = clang::Lexer::getSpelling(token, _sourceManager, _langOptions, nullptr);
    return spelling;
}

// void FormatVisitor::processComment()
// {
//     if (const clang::RawComment* comment = node->getASTContext().getRawCommentForDeclNoCache(node); comment)
//         _out << _cache.indent << comment->getRawText(_sourceManager).operator std::string_view() << '\n';
// }