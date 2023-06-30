/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
#define AA int f(void){ return 3; }

AA;

/* { dg-final { scan-tree-dump "#define AA int f\(void\){ return 3; }" } } */
/* { dg-final { scan-tree-dump "AA[;]?" } } */
