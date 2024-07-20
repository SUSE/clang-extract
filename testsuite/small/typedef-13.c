/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef struct _fakestr {
  struct _fakestr *pNext;
  int type;
} FAKE, *PFAKE;

typedef struct {
  PFAKE pFakeThings;
} DPYINFO, *PDPYINFO;

int f(PDPYINFO pDpyInfo) {
    return pDpyInfo->pFakeThings->pNext->type;
}

/* { dg-final { scan-tree-dump "typedef struct _fakestr {\n *struct _fakestr \*pNext;\n *int type;\n} *FAKE, \*PFAKE;" } } */
/* { dg-final { scan-tree-dump "typedef struct {\n *PFAKE pFakeThings;\n} DPYINFO, \*PDPYINFO;" } } */

/* { dg-final { scan-tree-dump-not "struct _fakestr {\n *struct _fakestr \*pNext;\n *int type;\n};" } } */
