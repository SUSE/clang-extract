#include "PrettyPrint.hh"

/** Public methods.  */

void PrettyPrint::Print_Decl(Decl *decl)
{
  /* When handling C code, we need to check if given declaration is a function
     with body.  If yes, we can simply print the declaration, but otherwise
     we need to manually insert the end of statement ';' token.  */

  FunctionDecl *f = dynamic_cast<FunctionDecl*>(decl);
  TagDecl *t = dynamic_cast<TagDecl*>(decl);
  TypedefNameDecl *td = dynamic_cast<TypedefDecl*>(decl);

  if (f && f->hasBody() && f->isThisDeclarationADefinition()) {
    f->print(Out, PPolicy);
  } else if (t && t->getName() == "") {
    /* If the RecordType doesn't have a name, then don't print it.  */
  } else if (td) {

    /* There is the special case 
       typedef struct {
         attrs
       } Type;

       in which the pretty printer hangs for some reason. We do it here
       manually if we detect this case.  

       What happens is that clang decompose such constructions in
       struct {
         attrs
       };

       typedef struct Type Type;

       but since the struct has no name, Type goes undefined.
       
       The way we fix that is output the typedef and the TagDecl
       body, and we avoid outputs of unamed structs.  */

    TagDecl *tagdecl = td->getAnonDeclWithTypedefName();
    StringRef tagname = "";

    if (tagdecl) {
      tagname = tagdecl->getName();
      if (tagname == "") {
        Out << "typedef "; 
        tagdecl->getDefinition()->print(Out, PPolicy);
        Out << " " << td->getName() << ";\n";
      }
    } else {
      decl->print(Out, PPolicy);
      Out << ";\n";
    }
  } else {
    decl->print(Out, PPolicy);
    Out << ";\n";
  }
}

void PrettyPrint::Print_Stmt(Stmt *stmt)
{
  /* Currently only used for debugging.  */
  stmt->printPretty(Out, nullptr, PPolicy);
  Out << "\n";
}

/** Private stuff.  */

/* See PrettyPrint.hh for what they do.  */
raw_ostream &PrettyPrint::Out = llvm::outs();
LangOptions PrettyPrint::LangOpts;
PrintingPolicy PrettyPrint::PPolicy(LangOpts);
