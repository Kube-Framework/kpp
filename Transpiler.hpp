#pragma once

#include "ClangTooling.hpp"

#include <vector>

class TranspilerMatchCallback;
class TranspilerMatchFinder;
class TranspilationPass;

class TranspilerMatchCallback final : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    using Callback = void(*)(const clang::ast_matchers::MatchFinder::MatchResult &, clang::Rewriter &);

    TranspilerMatchCallback(const Callback callback, clang::Rewriter &rewriter)
        : _callback(callback), _rewriter(rewriter) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) final
        { (*_callback)(result, _rewriter); }

private:
    Callback _callback;
    clang::Rewriter &_rewriter;
};

class TranspilerMatchFinder final : public clang::ast_matchers::MatchFinder
{
public:
    class SourceFileCallbacks final : public clang::tooling::SourceFileCallbacks
    {
    public:
        ~SourceFileCallbacks(void) final = default;
        SourceFileCallbacks(clang::Rewriter &rewriter) : _rewriter(rewriter) {}

        bool handleBeginSource(clang::CompilerInstance &compiler) final;

        void handleEndSource(void) final;

    private:
        clang::Rewriter &_rewriter;
    };

    /** @brief Constructor */
    TranspilerMatchFinder(const bool reverse);

    [[nodiscard]] inline SourceFileCallbacks &sourceFileCallbacks(void) noexcept { return _sourceFileCallbacks; }

private:
    clang::Rewriter _rewriter;
    std::vector<TranspilerMatchCallback> _matchCallbacks {};
    SourceFileCallbacks _sourceFileCallbacks;
};
