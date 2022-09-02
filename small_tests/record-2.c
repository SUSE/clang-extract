
struct Point
{
  int x, y;
};

typedef struct Point point_t;

int Get_X(point_t *p)
{
  return p->x;
}

int getX(struct Point p)
{
  return p.x;
}
