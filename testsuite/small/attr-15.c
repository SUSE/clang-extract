/* { dg-options "-Wno-everything -DCE_EXTRACT_FUNCTIONS=_libc_intl_domainname -DCE_NO_EXTERNALIZATION" }*/

extern const char _libc_intl_domainname[];
extern typeof (_libc_intl_domainname) _libc_intl_domainname asm("__gi__libc_intl_domainname") __attribute__((visibility("hidden")));


/* { dg-final { scan-tree-dump "extern const char _libc_intl_domainname\[\];" } } */
/* { dg-final { scan-tree-dump "extern typeof \(_libc_intl_domainname\) _libc_intl_domainname asm\(\"__gi__libc_intl_domainname\"\) __attribute__\(\(visibility\(\"hidden\"\)\)\);" } } */
