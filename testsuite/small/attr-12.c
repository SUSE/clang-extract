/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef struct _GFile GFile;
typedef GFile *GFile_autoptr;
static __attribute__((__unused__)) inline void
glib_autoptr_cleanup_GFile(GFile **_ptr) {
  return;
}

void f() {
    GFile** ptr;
    __attribute__((cleanup(glib_autoptr_cleanup_GFile))) GFile_autoptr
      socket_dir = ((void *)0);
}

/* { dg-final { scan-tree-dump "glib_autoptr_cleanup_GFile\(GFile \*\*_ptr\)" } } */
