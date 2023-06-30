#define __stringify_1(x...) #x
#define __stringify(x...)        __stringify_1(x)

const char *f(void)
{
  return __stringify(1);
}
