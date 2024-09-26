/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;

typedef __mbstate_t mbstate_t;

namespace std
{
  using ::mbstate_t;
}

int main(void)
{
  std::mbstate_t state;
  return 0;
}

/* { dg-final { scan-tree-dump "std::mbstate_t" } } */
