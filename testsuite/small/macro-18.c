#define inline inline __gnu_inline __inline_maybe_unused notrace
#define __inline__ inline
#define __inline_maybe_unused

#if defined(CC_USING_HOTPATCH)
#define notrace			__attribute__((hotpatch(0, 0)))
#elif defined(CC_USING_PATCHABLE_FUNCTION_ENTRY)
#define notrace			__attribute__((patchable_function_entry(0, 0)))
#else
#define notrace
#endif

#define __maybe_unused __attribute__((unused))

int __inline__ g(int x)
{
  return x + 3;
}

int f(int x)
{
  return g(x);
}
