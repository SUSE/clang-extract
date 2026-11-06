/* Mimics glibc's locale/localeinfo.h, which includes categories.def inside
   the body of a struct declared together with a variable.  */
#define CATNAMEMF(line) CATNAMEMF1 (line)
#define CATNAMEMF1(line) str##line
extern const struct catnamestr_t
{
#define DEFINE_CATEGORY(category, category_name) \
  char CATNAMEMF (__LINE__)[sizeof (category_name)];
#include "nested-include-2.def"
#undef DEFINE_CATEGORY
} _nl_category_names __attribute__ ((visibility ("hidden")));

typedef struct
{
#define DEFINE_CATEGORY(category, category_name) int category;
#include "nested-include-2.def"
#undef DEFINE_CATEGORY
} anon_t;
