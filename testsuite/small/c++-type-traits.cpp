/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

namespace std
{
  typedef long unsigned int size_t;
}

namespace std __attribute__ ((__visibility__ ("default")))
{
  template<typename _Tp, _Tp __v>
    struct integral_constant
    {
      static constexpr _Tp value = __v;
      typedef integral_constant<_Tp, __v> type;
    };

  using true_type = integral_constant<bool, true>;
  using false_type = integral_constant<bool, false>;

  template<typename>
    struct remove_cv;

  template<typename _Tp>
    using __remove_cv_t = typename remove_cv<_Tp>::type;

  template<typename>
    struct __is_integral_helper
    : public false_type { };


  template<>
    struct __is_integral_helper<wchar_t>
    : public true_type { };

  template<typename _Tp>
    struct is_integral
    : public __is_integral_helper<__remove_cv_t<_Tp>>::type
    { };

  template<typename _Tp>
    struct is_enum
    : public integral_constant<bool, __is_enum(_Tp)>
    { };

  template<typename>
    struct is_const
    : public false_type { };

  template<typename>
    struct is_volatile
    : public false_type { };

  template<typename _Tp>
    struct remove_cv
    { using type = __remove_cv(_Tp); };

  template<typename _Unqualified, bool _IsConst, bool _IsVol>
    struct __cv_selector;

  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, false, false>
    { typedef _Unqualified __type; };

  template<typename _Qualified, typename _Unqualified,
    bool _IsConst = is_const<_Qualified>::value,
    bool _IsVol = is_volatile<_Qualified>::value>
    class __match_cv_qualifiers
    {
      typedef __cv_selector<_Unqualified, _IsConst, _IsVol> __match;

    public:
      typedef typename __match::__type __type;
    };


  template<typename _Tp>
    struct __make_unsigned
    { typedef _Tp __type; };


  template<typename _Tp,
    bool _IsInt = is_integral<_Tp>::value,
    bool _IsEnum = is_enum<_Tp>::value>
    class __make_unsigned_selector;

  template<typename _Tp>
    class __make_unsigned_selector<_Tp, true, false>
    {
      using __unsigned_type
 = typename __make_unsigned<__remove_cv_t<_Tp>>::__type;

    public:
      using __type
 = typename __match_cv_qualifiers<_Tp, __unsigned_type>::__type;
    };

  template<typename _Tp>
    struct make_unsigned
    { typedef typename __make_unsigned_selector<_Tp>::__type type; };
}

int main()
{
  std::make_unsigned<wchar_t> x;
  return 0;
}

/* { dg-final { scan-tree-dump "std::make_unsigned" } } */
