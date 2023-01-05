 #include "Transpiler.hpp"
#include "Passes.hpp"

#include <iostream>

using namespace clang::ast_matchers;

bool TranspilerMatchFinder::SourceFileCallbacks::handleBeginSource(clang::CompilerInstance &compiler)
{
    _rewriter.setSourceMgr(compiler.getSourceManager(), compiler.getLangOpts());
    return true;
}

void TranspilerMatchFinder::SourceFileCallbacks::handleEndSource(void)
{
    // _rewriter.overwriteChangedFiles();
    const auto buffer = _rewriter.getRewriteBufferFor(_rewriter.getSourceMgr().getMainFileID());
    if (buffer->size())
        std::cout << std::string(buffer->begin(), buffer->end()) << std::endl;
}

TranspilerMatchFinder::TranspilerMatchFinder(const bool reverse)
    : _sourceFileCallbacks(_rewriter)
{
    const auto &passes = reverse ? CppToKpp::Passes : KppToCpp::Passes;

    _matchCallbacks.reserve(std::size(passes));
    for (auto &pass : passes) {
        auto &matchCallback = _matchCallbacks.emplace_back(pass.callback, _rewriter);
        std::visit(
            [this, &matchCallback]<typename Type>(const Type &matcher)
            {
                addMatcher(matcher, &matchCallback);
            },
            pass.matcher
        );
    }
}