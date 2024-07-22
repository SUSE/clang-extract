/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

void b() {
}

void a() {
    b();
}

void f() {
    extern void a();
    a();
}

/* { dg-final { scan-tree-dump "void b\(\) {" } } */
/* { dg-final { scan-tree-dump "void a\(\) {" } } */
