/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail }*/

typedef unsigned long size_t;
typedef int unused_t;

extern "C++" {
namespace std
{
  enum class align_val_t: size_t {};
  unsigned int unused;

};
};

std::align_val_t f(void)
{
  std::align_val_t r;
  return r;
}

/* { dg-final { scan-tree-dump "enum class align_val_t: size_t" } } */
/* { dg-final { scan-tree-dump-not "unused_t" } } */
/* { dg-final { scan-tree-dump-not "unused" } } */
