/** clang-extract: from /usr/include/bits/types/__mbstate_t.h:13:1  */
typedef struct
{
  int __count;
  union
  {
    int __wch;
    char __wchb[4];
  } __value;/* Value so far.  */
} __mbstate_t;

/** clang-extract: from /usr/include/bits/types/mbstate_t.h:6:1  */
typedef __mbstate_t mbstate_t;

typedef long ptrdiff_t;

namespace std {
  /** clang-extract: from /usr/lib64/gcc/x86_64-suse-linux/7/../../../../include/c++/7/bits/postypes.h:88:3  */
  typedef long          streamoff;

  /** clang-extract: from /usr/lib64/gcc/x86_64-suse-linux/7/../../../../include/c++/7/bits/postypes.h:98:3  */
  typedef ptrdiff_t	streamsize;

  /** clang-extract: from /usr/lib64/gcc/x86_64-suse-linux/7/../../../../include/c++/7/bits/postypes.h:111:3  */
  template<typename _StateT>
    class fpos
    {
      private:
        streamoff	                _M_off;
        _StateT			_M_state;

      public:
        // The standard doesn't require that fpos objects can be default
        // constructed. This implementation provides a default
        // constructor that initializes the offset to 0 and default
        // constructs the state.
        fpos()
          : _M_off(0), _M_state() { }

        // The standard requires that fpos objects can be constructed
        // from streamoff objects using the constructor syntax, and
        // fails to give any meaningful semantics. In this
        // implementation implicit conversion is also allowed, and this
        // constructor stores the streamoff as the offset and default
        // constructs the state.
        /// Construct position from offset.
        fpos(streamoff __off)
          : _M_off(__off), _M_state() { }

        /// Convert to streamoff.
        operator streamoff() const { return _M_off; }

        /// Remember the value of @a st.
        void
          state(_StateT __st)
          { _M_state = __st; }

        /// Return the last set value of @a st.
        _StateT
          state() const
          { return _M_state; }

        // The standard requires that this operator must be defined, but
        // gives no semantics. In this implementation it just adds its
        // argument to the stored offset and returns *this.
        /// Add offset to this position.
        fpos&
          operator+=(streamoff __off)
          {
            _M_off += __off;
            return *this;
          }

        // The standard requires that this operator must be defined, but
        // gives no semantics. In this implementation it just subtracts
        // its argument from the stored offset and returns *this.
        /// Subtract offset from this position.
        fpos&
          operator-=(streamoff __off)
          {
            _M_off -= __off;
            return *this;
          }

        // The standard requires that this operator must be defined, but
        // defines its semantics only in terms of operator-. In this
        // implementation it constructs a copy of *this, adds the
        // argument to that copy using operator+= and then returns the
        // copy.
        /// Add position and offset.
        fpos
          operator+(streamoff __off) const
          {
            fpos __pos(*this);
            __pos += __off;
            return __pos;
          }

        // The standard requires that this operator must be defined, but
        // defines its semantics only in terms of operator+. In this
        // implementation it constructs a copy of *this, subtracts the
        // argument from that copy using operator-= and then returns the
        // copy.
        /// Subtract offset from position.
        fpos
          operator-(streamoff __off) const
          {
            fpos __pos(*this);
            __pos -= __off;
            return __pos;
          }

        // The standard requires that this operator must be defined, but
        // defines its semantics only in terms of operator+. In this
        // implementation it returns the difference between the offset
        // stored in *this and in the argument.
        /// Subtract position to return offset.
        streamoff
          operator-(const fpos& __other) const
          { return _M_off - __other._M_off; }
    };

  /** clang-extract: from /usr/lib64/gcc/x86_64-suse-linux/7/../../../../include/c++/7/bits/postypes.h:228:3  */
  typedef fpos<mbstate_t> streampos;

  /** clang-extract: from /usr/lib64/gcc/x86_64-suse-linux/7/../../../../include/c++/7/bits/postypes.h:230:3  */
  typedef fpos<mbstate_t> wstreampos;

}

int f(void)
{
  std::wstreampos w;
  return 0;
}
