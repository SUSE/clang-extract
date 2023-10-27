/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

template<typename _CharT>
  struct _Char_types
  {
    typedef unsigned long int_type;
  };

template<typename _CharT>
struct char_traits
{
  typedef typename _Char_types<_CharT>::int_type int_type;
};

void* f(void)
{
  static struct char_traits<char> x;
  return (void *) &x;
}

/* { dg-final { scan-tree-dump "typedef typename _Char_types<_CharT>::int_type int_type;" } } */
/* { dg-final { scan-tree-dump "typedef unsigned long int_type;" } } */
/* { dg-final { scan-tree-dump "return \(void \*\) &x;" } } */
