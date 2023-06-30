struct Z {
  int z;
};

typedef struct Z Z;

struct Point
{
  int x, y;
  Z *z;
};

int Get_X(struct Point *p)
{
  return p->x;
}
