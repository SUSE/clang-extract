/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;
typedef __builtin_va_list va_list;

void f() {
    va_list args;
}

/* { dg-final { scan-tree-dump "typedef __builtin_va_list va_list;" } } */
