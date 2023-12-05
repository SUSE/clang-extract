/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{
  template<typename _CharT>
    struct _Char_types
    {
      typedef unsigned long int_type;
#if 0
      typedef std::streampos pos_type;
      typedef std::streamoff off_type;
      typedef std::mbstate_t state_type;
#endif
    };
  template<typename _CharT>
    struct char_traits
    {
      typedef _CharT char_type;
      typedef typename _Char_types<_CharT>::int_type int_type;
#if 0
      typedef typename _Char_types<_CharT>::pos_type pos_type;
      typedef typename _Char_types<_CharT>::off_type off_type;
      typedef typename _Char_types<_CharT>::state_type state_type;
#endif
      static constexpr void
      assign(char_type& __c1, const char_type& __c2)
      {
        __c1 = __c2;
      }

      static constexpr bool
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static constexpr bool
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }
#if 0
      static constexpr int
      compare(const char_type* __s1, const char_type* __s2, std::size_t __n);

      static constexpr std::size_t
      length(const char_type* __s);

      static constexpr const char_type*
      find(const char_type* __s, std::size_t __n, const char_type& __a);

      static char_type*
      move(char_type* __s1, const char_type* __s2, std::size_t __n);

      static char_type*
      copy(char_type* __s1, const char_type* __s2, std::size_t __n);

      static char_type*
      assign(char_type* __s, std::size_t __n, char_type __a);
#endif
      static constexpr char_type
      to_char_type(const int_type& __c)
      { return static_cast<char_type>(__c); }

      static constexpr int_type
      to_int_type(const char_type& __c)
      { return static_cast<int_type>(__c); }

      static constexpr bool
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static constexpr int_type
      eof()
      { return static_cast<int_type>(-1); }

      static constexpr int_type
      not_eof(const int_type& __c)
      { return !eq_int_type(__c, eof()) ? __c : to_int_type(char_type()); }

    };
#if 0
  template<typename _CharT>
    constexpr int
    char_traits<_CharT>::
    compare(const char_type* __s1, const char_type* __s2, std::size_t __n)
    {
      for (std::size_t __i = 0; __i < __n; ++__i)
        if (lt(__s1[__i], __s2[__i]))
          return -1;
      else if (lt(__s2[__i], __s1[__i]))
        return 1;
      return 0;
    }

  template<typename _CharT>
    constexpr std::size_t
    char_traits<_CharT>::
    length(const char_type* __p)
    {
      std::size_t __i = 0;
      while (!eq(__p[__i], char_type()))
        ++__i;
      return __i;
    }

  template<typename _CharT>
    constexpr const typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    find(const char_type* __s, std::size_t __n, const char_type& __a)
    {
      for (std::size_t __i = 0; __i < __n; ++__i)
        if (eq(__s[__i], __a))
          return __s + __i;
      return 0;
    }

  template<typename _CharT>
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    move(char_type* __s1, const char_type* __s2, std::size_t __n)
    {
      if (__n == 0)
        return __s1;
      __builtin_memmove(__s1, __s2, __n * sizeof(char_type));
      return __s1;
    }

  template<typename _CharT>
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    copy(char_type* __s1, const char_type* __s2, std::size_t __n)
    {
      if (__n == 0)
        return __s1;
      __builtin_memcpy(__s1, __s2, __n * sizeof(char_type));
      return __s1;
    }

  template<typename _CharT>
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    assign(char_type* __s, std::size_t __n, char_type __a)
    {
      if constexpr (sizeof(_CharT) == 1 && __is_trivial(_CharT))
      {
        if (__n)
        {
          unsigned char __c;
          __builtin_memcpy(&__c, __builtin_addressof(__a), 1);
          __builtin_memset(__s, __c, __n);
        }
      }
      else
      {
        for (std::size_t __i = 0; __i < __n; ++__i)
        __s[__i] = __a;
      }
      return __s;
    }
#endif
}

template<typename _CharT>
  struct char_traits : public __gnu_cxx::char_traits<_CharT>
  { };

void* f(void)
{
  static struct char_traits<char> x;
  return (void *) &x;
}


/* { dg-final { scan-tree-dump "char_traits : public __gnu_cxx::char_traits<_CharT>" } } */
/* { dg-final { scan-tree-dump "namespace __gnu_cxx" } } */
/* { dg-final { scan-tree-dump "return \(void \*\) &x;" } } */
