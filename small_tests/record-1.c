struct Point;

struct Point
{
  int x, y;
};

int Get_X(struct Point *p)
{
  return p->x;
}

int getX(struct Point p)
{
  return p.x;
}
