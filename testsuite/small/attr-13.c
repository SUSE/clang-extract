/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail } */

enum MHD_Result {
    MHD_NO = 0,
    MHD_YES = 1
}__attribute__((enum_extensibility(closed)));

enum MHD_Result f() {
    return MHD_NO;
}


/* { dg-final { scan-tree-dump "__attribute__\(\(enum_externsibility\(closed\)\)\)" } } */
