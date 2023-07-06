/* { dg-options "-DCE_EXTRACT_FUNCTIONS=dlsym_implementation -DCE_NO_EXTERNALIZATION" }*/

extern void *_dl_sym(void);
extern int _dlerror_run(void *);

static void *
dlsym_doit (void *a)
{
  return _dl_sym ();
}

int dlsym_implementation(void) {
    return _dlerror_run(dlsym_doit);
}

/* { dg-final { scan-tree-dump "extern void \*_dl_sym" } } */
