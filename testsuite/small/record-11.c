/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct files {
 const char *fname;
 int mode;
} *files;

void f(void)
{
 int a = files[0].mode;
}

/* { dg-final { scan-tree-dump "struct files {" } } */
