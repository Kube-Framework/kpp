// Declares clang::SyntaxOnlyAction.
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
// Declares llvm::cl::extrahelp.
#include <llvm/Support/CommandLine.h>

// Declares Kube's Transpiler classes
#include "Transpiler.hpp"

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory ToolCategory("Kube Transpiler Options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static llvm::cl::extrahelp MoreHelp("\nKube transpiler converts k++ source code to c++ (and vice-versa)\n");

// Reverse option
static llvm::cl::opt<bool> ReverseOption(
    "reverse", llvm::cl::desc("Reverse transpiler (Cpp to Kpp)"), llvm::cl::init(false)
);

int main(int argc, const char **argv)
{
    auto expectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, ToolCategory);
    if (!expectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << expectedParser.takeError();
        return 1;
    }

    clang::tooling::CommonOptionsParser &optionsParser = expectedParser.get();
    clang::tooling::ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());
    const auto beginArgs = argv;
    const auto endArgs = argv + argc;
    TranspilerMatchFinder matchFinder(ReverseOption.getValue());

    return tool.run(
        clang::tooling::newFrontendActionFactory(
            &matchFinder,
            &matchFinder.sourceFileCallbacks()
        ).get()
    );
}