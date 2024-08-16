#define __DEFINE_FUNCTION(i) \
  int function_ ## i (void) { \
    return 0;\
  }

#define _DEFINE_FUNCTION(i) \
  __DEFINE_FUNCTION(i)

#define DEFINE_FUNCTION \
  _DEFINE_FUNCTION(__COUNTER__)

DEFINE_FUNCTION;
