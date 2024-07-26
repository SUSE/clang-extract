/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef void (*XtErrorHandler)(char *);
extern void XtSetErrorHandler(XtErrorHandler __attribute((noreturn)));

void f() {
    XtSetErrorHandler(((void*)0));
}

/* { dg-final { scan-tree-dump "typedef void \(\*XtErrorHandler\)\(char \*\);" } } */

