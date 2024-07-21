/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

static struct block {
 unsigned int secnr;
} *blockhead;

void f() {
    unsigned int a = blockhead->secnr;
}

/* { dg-final { scan-tree-dump "static struct block {\n *unsigned int secnr;\n} *\*blockhead;" } } */
/* { dg-final { scan-tree-dump-not "^struct block {" } } */
