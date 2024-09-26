/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

template <typename _Tp>
  inline constexpr bool is_trivial_v = __is_trivial(_Tp);


int main(void)
{
  static_assert(is_trivial_v<char>);
  return 0;
}

/* { dg-final { scan-tree-dump "static_assert\(is_trivial_v" } } */
