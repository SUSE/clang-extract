struct Z {
  int z;
};

struct Point
{
  int x, y;
  struct Z z;
};

int Get_X(struct Point *p)
{
  return p->x;
}

int getX(struct Point p)
{
  return p.x;
}
