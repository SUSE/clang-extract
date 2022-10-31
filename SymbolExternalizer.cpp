#include "SymbolExternalizer.hh"
#include "PrettyPrint.hh"

#include <unordered_set>
#include <iostream>

/****** Begin hack: used to get a pointer to a private member of a class. *****/
struct ASTUnit_TopLevelDecls
{
  typedef std::vector<Decl*> ASTUnit::*type;
  friend type Get(ASTUnit_TopLevelDecls);
};

template<typename Tag, typename Tag::type M>
struct Rob {
  friend typename Tag::type Get(Tag) {
    return M;
  }
};

template struct Rob<ASTUnit_TopLevelDecls, &ASTUnit::TopLevelDecls>;

/** Get pointer to the TopLevelDecls vector in the ASTUnit.
 *
 * The TopLevelDecls attribute from the AST is private, but we need to
 * access that in order to remove nodes from the AST toplevel vector,
 * else we can't remove further declarations of the function we need
 * to externalize.
 */
static std::vector<Decl *>* Get_Pointer_To_Toplev(ASTUnit *obj)
{
  return &(obj->*Get(ASTUnit_TopLevelDecls()));
}
/****************************** End hack.  ***********************************/

static std::unordered_set<std::string *> StringPool;

void Free_String_Pool(void)
{
  for (auto it = StringPool.begin(); it != StringPool.end(); ++it) {
    std::string *elem = *it;
    delete elem;
  }

  StringPool.clear();
}

bool SymbolExternalizer::FunctionUpdater::Update_References_To_Symbol(Stmt *stmt)
{
  if (!stmt)
    return false;

  bool replaced = false;

  if (DeclRefExpr::classof(stmt)) {
    DeclRefExpr *expr = (DeclRefExpr *) stmt;
    ValueDecl *decl = expr->getDecl();

    if (decl->getName() == OldSymbolName) {
      /* Rewrite the source code.  */
      SourceLocation begin = expr->getBeginLoc();
      SourceLocation end = expr->getEndLoc();

      SourceRange range(begin, end);
      StringRef str = PrettyPrint::Get_Source_Text(range);

      /* Ensure that we indeed got the old symbol.  */
      if (str == OldSymbolName) {
        /* Prepare the text modification.  */
        std::string new_name;
        if (WasFunction) {
          /* In case the original declaration was a function, we only need to
             rewrite the new symbol name.  */
          new_name = NewSymbolDecl->getName().str();
        } else {
          /* In case the original declaration was a varaible, we need to indicate
             that we are now accessing a pointer variable.  */
          new_name = "(*" + NewSymbolDecl->getName().str() + ")";
        }

        /* Issue a text modification.  */
        RW.ReplaceText(range, new_name);
      } else {
        /* If we did not get the old symbol, it mostly means that the
           references comes from a macro.  */

        //std::cout << "WARNING: Unable to find location of symbol name: " << OldSymbolName << '\n';
      }

      /* Replace reference with the rewiten name.  */
      expr->setDecl(NewSymbolDecl);
      replaced = true;
    }
  }

  /* Repeat the process to child statements.  */
  clang::Stmt::child_iterator it, it_end;
  for (it = stmt->child_begin(), it_end = stmt->child_end();
      it != it_end; ++it) {

    Stmt *child = *it;
    replaced |= Update_References_To_Symbol(child);
  }

  return replaced;
}

bool SymbolExternalizer::FunctionUpdater::Update_References_To_Symbol(DeclaratorDecl *to_update)
{
  ToUpdate = to_update;
  if (to_update) {
    return Update_References_To_Symbol(to_update->getBody());
  }
  return false;
}

void SymbolExternalizer::Externalize_Symbol(DeclaratorDecl *to_externalize)
{
  assert(false && "To be implemented.");
}

VarDecl *SymbolExternalizer::Create_Externalized_Var(DeclaratorDecl *decl, const std::string &name)
{

  /* Hack a new Variable Declaration node in which holds the address of our
     externalized symbol.  We use the information of the old function to
     build it. For example, assume we want to externalize:

     int f(int);

     then we need to emit:

     static int (*f)(int);

     instead of the first function delcaration.  Therefore we need:
      - The new node's type must be a pointer to the original function type.
      - The source location of the new node should somewhat match the original
        symbol.
      - We should append a `static` to it so it doesn't clash with other
        livepatches.  */

  /* Create a new identifier with name `name`.  */
  IdentifierInfo *id = AST->getPreprocessor().getIdentifierInfo(name);

  /* Get clang's ASTContext, needed to allocate the node.  */
  ASTContext &astctx = AST->getASTContext();

  /* Create a type that is a pointer to the externalized object's type.  */
  QualType pointer_to = astctx.getPointerType(decl->getType());
  TypeSourceInfo *tsi = astctx.CreateTypeSourceInfo(pointer_to);

  /* Get context of decl.  */
  DeclContext *decl_ctx;

  /* Create node.  */
  if (dynamic_cast<FunctionDecl *>(decl)) {
    decl_ctx = dynamic_cast<FunctionDecl *>(decl)->getParent();
  } else {
    decl_ctx = decl->getDeclContext();
  }

  VarDecl *ret = VarDecl::Create(astctx, decl_ctx,
    decl->getBeginLoc(),
    decl->getEndLoc(),
    id,
    pointer_to,
    tsi,
    SC_Static
  );

  /* return node.  */
  return ret;
}

bool SymbolExternalizer::Commit_Changes_To_Source(void)
{
  SourceManager &sm = AST->getSourceManager();
  clang::SourceManager::fileinfo_iterator it;
  bool modified = false;

  /* Iterate into all files we may have opened, most probably headers that are
     #include'd.  */
  for (it = sm.fileinfo_begin(); it != sm.fileinfo_end(); ++it) {
    const FileEntry *fentry = it->getFirst();

    /* Our updated file buffer.  */
    const RewriteBuffer *rewritebuf = RW.getRewriteBufferFor(sm.translateFile(fentry));

    /* If we have modifications, then update the buffer.  */
    if (rewritebuf) {
      /* The RewriteBuffer object contains a sequence of deltas of the original
         buffer.  Clearly a faster way would be apply those deltas into the
         target buffer directly, but clang doesn't seems to provide such
         interface, so we expand them into a temporary string and them pass
         it to the SourceManager.  */
      std::string *modified_str = new std::string(rewritebuf->begin(), rewritebuf->end());
      StringPool.insert(modified_str);
      auto new_membuff = llvm::MemoryBuffer::getMemBuffer(*modified_str);
      sm.overrideFileContents(fentry, std::move(new_membuff));

      modified = true;
    }
  }

  return modified;
}

std::string SymbolExternalizer::Get_Modifications_To_Main_File(void)
{

  SourceManager &sm = AST->getSourceManager();

  /* Our updated file buffer for main file.  */
  FileID main_id = sm.getMainFileID();
  RewriteBuffer &main_buf = RW.getEditBuffer(main_id);

  return std::string(main_buf.begin(), main_buf.end());
}

void SymbolExternalizer::_Externalize_Symbol(const std::string &to_externalize)
{
  ASTUnit::top_level_iterator it;
  VarDecl *new_decl = nullptr;
  bool must_update = false;
  bool was_function = false;

  std::vector<Decl *> *topleveldecls = nullptr;

  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    DeclaratorDecl *decl = dynamic_cast<DeclaratorDecl *>(*it);

    /* If we externalized some function, then we must start analyzing for further
       functions in order to find if there is a reference to the function we
       externalized.  */
    if (must_update) {
      FunctionUpdater(RW, new_decl, to_externalize, was_function)
        .Update_References_To_Symbol(decl);
    }

    if (decl && decl->getName() == to_externalize) {

      /* If we externalized this function, then all further delcarations of
         this function shall be discarded.  */
      if (must_update) {

        /* Get source location of old function declaration.  */
        SourceLocation decl_start = decl->getSourceRange().getBegin();
        SourceLocation decl_end = Lexer::getLocForEndOfToken(
            decl->getSourceRange().getEnd(),
            0,
            AST->getSourceManager(),
            AST->getLangOpts());

        SourceRange decl_range(decl_start, decl_end);

        /* Remove the text.  */
        RW.RemoveText(decl_range);

        topleveldecls->erase(it);
        /* We must decrease the iterator because we deleted an element from the
           vector.  */
        it--;
      }

      /* If we found the first instance of the function we want to externalize,
         then proceed to create and replace the function declaration node with
         a variable declaration node of proper type.  */
      else if (!new_decl) {
        /* The TopLevelDecls attribute from the AST is private, but we need to
           access that in order to remove nodes from the AST toplevel vector,
           else we can't remove further declarations of the function we need
           to externalize.  */
        topleveldecls = Get_Pointer_To_Toplev(AST);

        std::string new_name = "klp_" + decl->getName().str();
        new_decl = Create_Externalized_Var(decl, new_name);

        /* Create a string with the new variable type and name.  */
        std::string o;
        llvm::raw_string_ostream outstr(o);
        new_decl->print(outstr, AST->getLangOpts());
        outstr << ";\n";

        /* Get source location of old function declaration.  */
        SourceLocation decl_start;
        SourceLocation decl_end;

        decl_start = Lexer::GetBeginningOfToken(
            decl->getSourceRange().getBegin(),
            AST->getSourceManager(),
            AST->getLangOpts());

        decl_end = Lexer::getLocForEndOfToken(
            decl->getSourceRange().getEnd(),
            0,
            AST->getSourceManager(),
            AST->getLangOpts());

        SourceRange decl_range(decl_start, decl_end);

        /* Replace text content of old declaration.  */
        assert(RW.ReplaceText(decl_range, outstr.str()) == false && "Location not RW.");

        must_update = true;
        was_function = dynamic_cast<FunctionDecl*>(decl) ? true : false;

        /* Update any macros that may reference the symbol.  */
        std::string replacement = was_function ? new_decl->getName().str() : "*(" + new_decl->getName().str() + ")";

        Rewrite_Macros(to_externalize, replacement);

        /* Slaps the new node into the position of where was the function
           to be externalized.  */
        *it = new_decl;
      }
    }
  }
}

void SymbolExternalizer::Rewrite_Macros(std::string const &to_look_for, std::string const &replace_with)
{
  PreprocessingRecord *rec = AST->getPreprocessor().getPreprocessingRecord();

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {
      MacroInfo *info = MW.Get_Macro_Info(def);

      if (!info)
        continue;

      for (const Token tok : info->tokens()) {
        IdentifierInfo *id_info = tok.getIdentifierInfo();
        if (!id_info)
          continue;

        MacroInfo *maybe_macro = MW.Get_Macro_Info(id_info, def->getLocation());

        if (!maybe_macro && !MacroWalker::Is_Identifier_Macro_Argument(info, id_info)) {
          if (id_info->getName() == to_look_for) {
            SourceRange range(tok.getLocation(), tok.getLastLoc());
            RW.ReplaceText(range, replace_with);
          }
        }
      }
    }
  }
}

void SymbolExternalizer::Externalize_Symbol(const std::string &to_externalize)
{
  _Externalize_Symbol(to_externalize);
}

void SymbolExternalizer::Externalize_Symbols(std::vector<std::string> const &to_externalize_array)
{

  for (const std::string &to_externalize : to_externalize_array) {
    _Externalize_Symbol(to_externalize);
  }

  /* Update the source file buffer, else when we output based on the original
     source we would still get references to the old symbol.  */
  //Commit_Changes_To_Source();
}
