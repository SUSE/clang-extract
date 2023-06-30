/* { dg-options "-DCE_EXTRACT_FUNCTIONS=function -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail } Tests fails because we currently can't track that the endif
   comes from #ifdef AAA.  */


//#define AA

#ifdef AA
int function(void) {
  int x = 3;
#else
int function(int x) {
#endif
  return x;
}

/* { dg-final { scan-tree-dump "int function\(int x\) {" } } */
