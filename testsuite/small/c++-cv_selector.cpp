/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

namespace std {

  template<typename _Unqualified, bool _IsConst, bool _IsVol>
    struct __cv_selector;

  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, false, false>
    { typedef _Unqualified __type; };

  template<typename _Qualified, typename _Unqualified,
    bool _IsConst = false,
    bool _IsVol = false>
    class __match_cv_qualifiers
    {
      typedef __cv_selector<_Unqualified, _IsConst, _IsVol> __match;

    public:
      typedef typename __match::__type __type;
    };
}


int main()
{
  std::__match_cv_qualifiers<wchar_t, wchar_t> x;
  return 0;
}

/* { dg-final { scan-tree-dump "std::__match_cv_qualifiers" } } */
