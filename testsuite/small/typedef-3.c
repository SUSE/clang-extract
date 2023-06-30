typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t;

struct Point {
  uint32_t x;
  uint32_t y;
};

uint32_t Get_X(struct Point *p)
{
  return p->x;
}
