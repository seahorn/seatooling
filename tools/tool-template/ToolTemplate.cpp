//===---- tools/extra/ToolTemplate.cpp - Template for refactoring tool ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements an empty refactoring tool using the clang tooling.
//  The goal is to lower the "barrier to entry" for writing refactoring tools.
//
//  Usage:
//  tool-template <cmake-output-dir> <file1> <file2> ...
//
//  Where <cmake-output-dir> is a CMake build directory in which a file named
//  compile_commands.json exists (enable -DCMAKE_EXPORT_COMPILE_COMMANDS in
//  CMake to get this output).
//
//  <file1> ... specify the paths of files in the CMake source tree. This path
//  is looked up in the compile command database. If the path of a file is
//  absolute, it needs to point into CMake's source tree. If the path is
//  relative, the current working directory needs to be in the CMake source
//  tree and the file must be in a subdirectory of the current working
//  directory. "./" prefixes in the relative files will be automatically
//  removed, but the rest of a relative path must be a suffix of a path in
//  the compile command line database.
//
//  For example, to use tool-template on all files in a subtree of the
//  source tree, use:
//
//    /path/in/subtree $ find . -name '*.cpp'|
//        xargs tool-template /path/to/build
//
//===----------------------------------------------------------------------===//

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Signals.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

namespace {
class ToolTemplateCallback : public MatchFinder::MatchCallback {
 public:
  ToolTemplateCallback(Replacements *Replace) : Replace(Replace) {}

  void run(const MatchFinder::MatchResult &Result) override {
    // TODO: This routine will get called for each thing that the matchers find.
    // At this point, you can examine the match, and do whatever you want,
    // including replacing the matched text with other text

    //ers () << "In callback\n";
    
    if (const RecordDecl *decl = Result.Nodes.getNodeAs<RecordDecl> ("rec"))
    {
      errs () << "Found a decl\n";
      decl->dump ();
      return;
    }
    
    
    const FieldDecl *fld = Result.Nodes.getNodeAs<FieldDecl> ("fld");
    if (!fld) return;
    if (!fld->isBitField ()) return;
    if (!fld->getDeclName ().isIdentifier ()) return;
    
    //fld->dump ();
    
    
    // if (TypeSourceInfo *TSI = fld->getTypeSourceInfo ())
    // {
    //   TypeLoc TL = TSI->getTypeLoc ();
    //   TL.getSourceRange ().getBegin ().print (errs (), *Result.SourceManager);
    //   errs () << " ";
    //   TL.getSourceRange ().getEnd ().print (errs (), *Result.SourceManager);
    //   errs () << "\n";

    //   if (fld->getBitWidthValue (fld->getASTContext ()) == 1)
    //     Replace->insert
    //       (Replacement (*Result.SourceManager,
    //                     CharSourceRange::getTokenRange
    //                     (TL.getSourceRange ().getBegin (),
    //                      TL.getSourceRange ().getEnd ()),
    //                     "bool"));
    // }
    
    // -- name + bit-field declaration
    CharSourceRange rng =
      CharSourceRange::getTokenRange (fld->getLocation (),
                                      fld->getBitWidth ()->getLocStart ());
    
    // -- replace rng by name only
    Replace->insert (Replacement (*Result.SourceManager, rng, fld->getName ()));
    
    // Replace->insert (Replacement (*Result.SourceManager, fld->getBitWidth (), "32"));
    // Replace->insert (Replacement (*Result.SourceManager, ..., ""));
    
    (void)Replace; // This to prevent an "unused member variable" warning;
  }

 private:
  Replacements *Replace;
};
} // end anonymous namespace

// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory ToolTemplateCategory("tool-template options");

int main(int argc, const char **argv) {
  
  llvm::sys::PrintStackTraceOnErrorSignal();
  CommonOptionsParser OptionsParser(argc, argv, ToolTemplateCategory);
  RefactoringTool Tool(OptionsParser.getCompilations(),
                       OptionsParser.getSourcePathList());
  ast_matchers::MatchFinder Finder;
  ToolTemplateCallback Callback(&Tool.getReplacements());

  errs () << "Setting up matchers\n";
  
  // TODO: Put your matchers here.
  // Use Finder.addMatcher(...) to define the patterns in the AST that you
  // want to match against. You are not limited to just one matcher!
  Finder.addMatcher
    (recordDecl ().bind ("rec"), &Callback);
   Finder.addMatcher
    (fieldDecl (decl().bind ("fld")),
     &Callback);
 
  return Tool.runAndSave(newFrontendActionFactory(&Finder).get());
}
