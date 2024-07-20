/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

extern int strncasecmp (const char *__s1, const char *__s2, unsigned long __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern unsigned long strlen (const char *__s)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

struct config {
 enum sched_prio {
  SCHED_ERR = -1,
  SCHED_HIGH,
  SCHED_DEFAULT,
  SCHED_LOW
 } prio;
 unsigned int verbose;
};

/** clang-extract: from source.c:1511:1 */
enum sched_prio f(const char *str)
{
 if (strncasecmp("high", str, strlen(str)) == 0)
  return SCHED_HIGH;
 else if (strncasecmp("default", str, strlen(str)) == 0)
  return SCHED_DEFAULT;
 else if (strncasecmp("low", str, strlen(str)) == 0)
  return SCHED_LOW;
 else
  return SCHED_ERR;
}

/* { dg-final { scan-tree-dump "struct config *{" } } */
/* { dg-final { scan-tree-dump " *enum sched_prio *{" } } */
