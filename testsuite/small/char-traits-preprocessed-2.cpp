/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

typedef unsigned int wint_t;

namespace std {
  /** clang-extract: from /usr/bin/../lib64/gcc/x86_64-suse-linux/13/../../../../include/c++/13/x86_64-suse-linux/bits/c++config.h:2605:3 */
typedef long unsigned int size_t;

}
namespace std {
  /** clang-extract: from /usr/bin/../lib64/gcc/x86_64-suse-linux/13/../../../../include/c++/13/x86_64-suse-linux/bits/c++config.h:2836:3 */
constexpr inline bool
  __is_constant_evaluated() noexcept
  {
    return __builtin_is_constant_evaluated();
  }

}
/** clang-extract: from /usr/lib64/clang/18/include/__stddef_size_t.h:18:1 */
typedef long unsigned int size_t;

/** clang-extract: from /usr/include/bits/types/__mbstate_t.h:13:1 */
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;

/** clang-extract: from /usr/include/bits/types/mbstate_t.h:6:1 */
typedef __mbstate_t mbstate_t;

extern "C" {
/** clang-extract: from /usr/include/wchar.h:247:1 */
extern size_t wcslen (const wchar_t *__s) noexcept (true) __attribute__ ((__pure__));

/** clang-extract: from /usr/include/wchar.h:278:1 */
extern wchar_t *wmemchr (const wchar_t *__s, wchar_t __c, size_t __n)
     noexcept (true) __attribute__ ((__pure__));

/** clang-extract: from /usr/include/wchar.h:283:1 */
extern int wmemcmp (const wchar_t *__s1, const wchar_t *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__));

/** clang-extract: from /usr/include/wchar.h:287:1 */
extern wchar_t *wmemcpy (wchar_t *__restrict __s1,
    const wchar_t *__restrict __s2, size_t __n) noexcept (true);

/** clang-extract: from /usr/include/wchar.h:292:1 */
extern wchar_t *wmemmove (wchar_t *__s1, const wchar_t *__s2, size_t __n)
     noexcept (true);

/** clang-extract: from /usr/include/wchar.h:296:1 */
extern wchar_t *wmemset (wchar_t *__s, wchar_t __c, size_t __n) noexcept (true);

};
namespace std {
  /** clang-extract: from /usr/bin/../lib64/gcc/x86_64-suse-linux/13/../../../../include/c++/13/cwchar:64:3 */
using ::mbstate_t;

}
namespace std {
  /** clang-extract: from /usr/bin/../lib64/gcc/x86_64-suse-linux/13/../../../../include/c++/13/bits/postypes.h:62:3 */
typedef long int streamoff;

/** clang-extract: from /usr/bin/../lib64/gcc/x86_64-suse-linux/13/../../../../include/c++/13/bits/postypes.h:81:3 */
template<typename _StateT>
    class fpos
    {
    private:
      streamoff _M_off;
      _StateT _M_state;

    public:




      fpos()
      : _M_off(0), _M_state() { }
      fpos(streamoff __off)
      : _M_off(__off), _M_state() { }


      fpos(const fpos&) = default;
      fpos& operator=(const fpos&) = default;
      ~fpos() = default;



      operator streamoff() const { return _M_off; }


      void
      state(_StateT __st)
      { _M_state = __st; }


      _StateT
      state() const
      { return _M_state; }





      fpos&
      operator+=(streamoff __off)
      {
 _M_off += __off;
 return *this;
      }





      fpos&
      operator-=(streamoff __off)
      {
 _M_off -= __off;
 return *this;
      }







      fpos
      operator+(streamoff __off) const
      {
 fpos __pos(*this);
 __pos += __off;
 return __pos;
      }







      fpos
      operator-(streamoff __off) const
      {
 fpos __pos(*this);
 __pos -= __off;
 return __pos;
      }






      streamoff
      operator-(const fpos& __other) const
      { return _M_off - __other._M_off; }
    };

/** clang-extract: from /usr/bin/../lib64/gcc/x86_64-suse-linux/13/../../../../include/c++/13/bits/postypes.h:204:3 */
typedef fpos<mbstate_t> streampos;

/** clang-extract: from /usr/bin/../lib64/gcc/x86_64-suse-linux/13/../../../../include/c++/13/bits/postypes.h:206:3 */
typedef fpos<mbstate_t> wstreampos;

}
namespace __gnu_cxx {
  /** clang-extract: from /usr/include/c++/13/bits/char_traits.h:83:3 */
template<typename _CharT>
    struct _Char_types
    {
      typedef unsigned long int_type;

      typedef std::streampos pos_type;
      typedef std::streamoff off_type;
      typedef std::mbstate_t state_type;

    };

/** clang-extract: from /usr/include/c++/13/bits/char_traits.h:110:3 */
template<typename _CharT>
    struct char_traits
    {
      typedef _CharT char_type;
      typedef typename _Char_types<_CharT>::int_type int_type;

      typedef typename _Char_types<_CharT>::pos_type pos_type;
      typedef typename _Char_types<_CharT>::off_type off_type;
      typedef typename _Char_types<_CharT>::state_type state_type;





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

/** clang-extract: from /usr/include/c++/13/bits/char_traits.h:184:3 */
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

/** clang-extract: from /usr/include/c++/13/bits/char_traits.h:197:3 */
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

/** clang-extract: from /usr/include/c++/13/bits/char_traits.h:208:3 */
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

/** clang-extract: from /usr/include/c++/13/bits/char_traits.h:219:3 */
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

/** clang-extract: from /usr/include/c++/13/bits/char_traits.h:250:3 */
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

/** clang-extract: from /usr/include/c++/13/bits/char_traits.h:270:3 */
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

}
namespace std {
  /** clang-extract: from /usr/include/c++/13/bits/char_traits.h:330:3 */
template<typename _CharT>
    struct char_traits : public __gnu_cxx::char_traits<_CharT>
    { };

/** clang-extract: from /usr/include/c++/13/bits/char_traits.h:478:3 */
template<>
    struct char_traits<wchar_t>
    {
      typedef wchar_t char_type;
      typedef wint_t int_type;

      typedef streamoff off_type;
      typedef wstreampos pos_type;
      typedef mbstate_t state_type;





      static constexpr void
      assign(char_type& __c1, const char_type& __c2) noexcept
      {





 __c1 = __c2;
      }

      static constexpr bool
      eq(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr bool
      lt(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 < __c2; }

      static constexpr int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return 0;

 if (std::__is_constant_evaluated())
   return __gnu_cxx::char_traits<char_type>::compare(__s1, __s2, __n);

 return wmemcmp(__s1, __s2, __n);
      }

      static constexpr size_t
      length(const char_type* __s)
      {

 if (std::__is_constant_evaluated())
   return __gnu_cxx::char_traits<char_type>::length(__s);

 return wcslen(__s);
      }

      static constexpr const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      {
 if (__n == 0)
   return 0;

 if (std::__is_constant_evaluated())
   return __gnu_cxx::char_traits<char_type>::find(__s, __n, __a);

 return wmemchr(__s, __a, __n);
      }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return wmemmove(__s1, __s2, __n);
      }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return wmemcpy(__s1, __s2, __n);
      }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      {
 if (__n == 0)
   return __s;




 return wmemset(__s, __a, __n);
      }

      static constexpr char_type
      to_char_type(const int_type& __c) noexcept
      { return char_type(__c); }

      static constexpr int_type
      to_int_type(const char_type& __c) noexcept
      { return int_type(__c); }

      static constexpr bool
      eq_int_type(const int_type& __c1, const int_type& __c2) noexcept
      { return __c1 == __c2; }


      static constexpr int_type
      eof() noexcept
      { return static_cast<int_type>((0xffffffffu)); }

      static constexpr int_type
      not_eof(const int_type& __c) noexcept
      { return eq_int_type(__c, eof()) ? 0 : __c; }

  };

}
namespace std {
  }
/** clang-extract: from char-traits.cpp:3:1 */
int main(void)
{
  struct std::char_traits<wchar_t> x;
  return 0;
}

/* { dg-final { scan-tree-dump "struct std::char_traits<wchar_t>" } } */
