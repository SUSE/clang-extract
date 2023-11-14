#include "TopLevelASTIterator.hh"

TopLevelASTIterator::TopLevelASTIterator(ASTUnit *ast, bool skip_macros_in_decls)
  : AST(ast),
    SM(AST->getSourceManager()),
    PrepRec(*AST->getPreprocessor().getPreprocessingRecord()),
    DeclIt(AST->top_level_begin()),
    MacroIt(PrepRec.begin()),
    UndefIt(0),
    NeedsUndef({}),
    BeforeClass(SM),
    MW(AST->getPreprocessor()),
    SkipMacrosInDecls(skip_macros_in_decls),
    Ended(false),
    EndLocOfLastDecl(PrepRec.begin()->getSourceRange().getBegin())
{
  Populate_Needs_Undef();
  Advance();
}

SourceLocation TopLevelASTIterator::Return::Get_Location(void)
{
  switch (Type)
  {
    case ReturnType::TYPE_INVALID:
      return SourceLocation();
      break;

    case ReturnType::TYPE_DECL:
      return AsDecl->getLocation();
      break;

    case ReturnType::TYPE_PREPROCESSED_ENTITY:
      return AsPrep->getSourceRange().getBegin();
      break;

    case ReturnType::TYPE_MACRO_UNDEF:
      return AsUndef->getLocation();
      break;
  }
  __builtin_unreachable();
}

void TopLevelASTIterator::Populate_Needs_Undef(void)
{
  PreprocessingRecord *rec = AST->getPreprocessor().getPreprocessingRecord();

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {

      MacroDirective *directive = MW.Get_Macro_Directive(def);

      /* If there is no history, then doesn't bother analyzing.  */
      if (directive == nullptr)
        continue;

      SourceLocation undef_loc =
          directive->getDefinition().getUndefLocation();

      /* In case the macro isn't undefined then its location is invalid.  */
      if (undef_loc.isValid()) {
        /* If we somehow lost the undef location, then mark it to output.  */
        NeedsUndef.push_back(directive);
      }
    }
  }

  /* Sort macros by location where they should be undefined.  */
  class CompareMacroUndefLoc
  {
    public:
    CompareMacroUndefLoc(BeforeThanCompare<SourceLocation> &beforeclass)
      : BeforeClassRef(beforeclass)
    { }

    bool operator()(const MacroDirective *a, const MacroDirective *b)
    {
      return BeforeClassRef(a->getDefinition().getUndefLocation(),
                            b->getDefinition().getUndefLocation());
    }

    BeforeThanCompare<SourceLocation> &BeforeClassRef;
  };
  std::sort(NeedsUndef.begin(), NeedsUndef.end(), CompareMacroUndefLoc(BeforeClass));
}

bool TopLevelASTIterator::Advance(void)
{
  SourceManager &sm = AST->getSourceManager();
  const SourceLocation &end = sm.getLocForEndOfFile(sm.getMainFileID());

#define NEXT_DECL_IF_EXISTS(DeclIt, end) ((DeclIt) != AST->top_level_end()) \
                                         ? (*(DeclIt))->getLocation() \
                                         : (end)
#define NEXT_PREP_IF_EXISTS(MacroIt, end) ((MacroIt) != PrepRec.end()) \
                                         ? (*(MacroIt))->getSourceRange().getBegin() \
                                         : (end)
#define NEXT_UNDEF_IF_EXISTS(UndefIt, end) (UndefIt < NeedsUndef.size()) \
                                           ? (NeedsUndef)[UndefIt]->getLocation() \
                                           : (end);

  SourceLocation end_of_last_decl = EndLocOfLastDecl;

  do {
    const SourceLocation &next_decl_loc = NEXT_DECL_IF_EXISTS(DeclIt, end);
    const SourceLocation &next_prep_loc = NEXT_PREP_IF_EXISTS(MacroIt, end);
    const SourceLocation &next_undef_loc = NEXT_UNDEF_IF_EXISTS(UndefIt, end);

    // Find out what comes first.
    std::vector<SourceLocation> vec = {
      next_decl_loc,
      next_prep_loc,
      next_undef_loc,
    };

    class CompareLocWithAssertion
    {
      public:
      CompareLocWithAssertion(BeforeThanCompare<SourceLocation> &beforeclass)
        : BeforeClassRef(beforeclass)
      { }

      bool operator()(const SourceLocation &a, const SourceLocation &b)
      {
        assert(a.isValid());
        assert(b.isValid());
        return BeforeClassRef(a, b);
      }

      BeforeThanCompare<SourceLocation> &BeforeClassRef;
    };

    std::sort(vec.begin(), vec.end(), CompareLocWithAssertion(BeforeClass));

    /* Check which comes next.  */
    const SourceLocation &next = vec[0];
    if (next == end) {
      /* We reached the end.  */
      Ended = true;
      return false;
    }

    if (next == next_decl_loc) {
      EndLocOfLastDecl = (*DeclIt)->getEndLoc();
      Current = Return(*DeclIt);
      ++DeclIt;

    } else if (next == next_prep_loc) {
      Current = Return(*MacroIt);
      ++MacroIt;
    }
    else if (next == next_undef_loc) {
      Current = Return(NeedsUndef[UndefIt]);
      ++UndefIt;
    }
  } while (SkipMacrosInDecls && Is_Before(Current.Get_Location(), end_of_last_decl));

  return true;
}

bool TopLevelASTIterator::Skip_Until(const SourceLocation &loc)
{
  bool valid = Current.Type != ReturnType::TYPE_INVALID;
  SourceLocation current = Current.Get_Location();
  while (valid && Is_Before(current, loc)) {
    valid = Advance();
    current = Current.Get_Location();
  }

  return valid;
}

#include "PrettyPrint.hh"

void Debug_TopLevelASTWalker(ASTUnit *ast)
{
  TopLevelASTIterator it(ast);
  for (; !it.End(); ++it) {
    TopLevelASTIterator::Return &Current = *it;

    switch (Current.Type) {
      case TopLevelASTIterator::ReturnType::TYPE_INVALID:
        assert(0 && "Invalid type.");
        break;
      case TopLevelASTIterator::ReturnType::TYPE_DECL:
        PrettyPrint::Debug_Decl(Current.AsDecl);
        break;
      case TopLevelASTIterator::ReturnType::TYPE_PREPROCESSED_ENTITY:
        if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(Current.AsPrep))
        {
          PrettyPrint::Debug_Macro_Def(def);
        } else {
          llvm::outs() << PrettyPrint::Get_Source_Text(Current.AsPrep->getSourceRange());
          llvm::outs() << '\n';
        }
        break;
      case TopLevelASTIterator::ReturnType::TYPE_MACRO_UNDEF:
        PrettyPrint::Debug_Macro_Undef(Current.AsUndef);
        break;
    }
  }
}
