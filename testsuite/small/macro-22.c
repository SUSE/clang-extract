/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_RENAME_SYMBOLS" }*/

// Same as macro-21.c but with rename symbols
//
typedef __builtin_va_list __gnuc_va_list;
extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);
typedef __builtin_va_list va_list;

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define __printf(a, b)                  __attribute__((__format__(printf, a, b)))

__printf(1, 2)
static void f(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf (fmt, args);
	va_end(args);
}

/* { dg-final { scan-tree-dump-not "static" } } */
