/* { dg-options "-DCE_EXTRACT_FUNCTIONS=___dlsym -DCE_NO_EXTERNALIZATION" }*/

extern void *_dl_sym(void *handle, const char *name, void *who) __attribute__((visibility("hidden")));
extern int _dlerror_run(void (*operate)(void *), void *args) __attribute__((visibility("hidden")));

struct dlsym_args
{
  /* The arguments to dlsym_doit.  */
  void *handle;
  const char *name;
  void *who;

  /* The return value of dlsym_doit.  */
  void *sym;
};

static void
dlsym_doit (void *a)
{
  struct dlsym_args *args = (struct dlsym_args *) a;

  args->sym = _dl_sym (args->handle, args->name, args->who);
}

static void *dlsym_implementation(void *handle, const char *name, void *dl_caller) {
    struct dlsym_args args;
    args.who = dl_caller;
    args.handle = handle;
    args.name = name;
    //__pthread_mutex_lock(&(_dl_load_lock).mutex);
    void *result = (_dlerror_run(dlsym_doit, &args) ? ((void *)0) : args.sym);
    //__pthread_mutex_unlock(&(_dl_load_lock).mutex);
    return result;
}

void *__dlsym(void *handle, const char *name, void *dl_caller) {
    return dlsym_implementation(handle, name, dl_caller);
}

void *___dlsym(void *handle, const char *name) {
    return __dlsym(handle, name, __builtin_extract_return_addr(__builtin_return_address(0)));
}

/* { dg-final { scan-tree-dump "extern void \*_dl_sym" } } */
