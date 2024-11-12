/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

namespace std
{
  template<typename>
    struct remove_cv;

  template<typename _Tp>
    using __remove_cv_t = typename remove_cv<_Tp>::type;

  template<typename _Tp>
    struct remove_cv
    { using type = __remove_cv(_Tp); };
}

int main()
{
  std::__remove_cv_t<wchar_t> x;
  return 0;
}

/* { dg-final { scan-tree-dump "std::__remove_cv_t" } } */
