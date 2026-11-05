extern const struct names_t
{
#define DEF(x) char x;
#include "nested-include-1.def"
#undef DEF
} names;
