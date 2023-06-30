typedef struct {
  int x;
} X;

typedef struct {
  X x;
  int y;
} Y;

int f(Y *y)
{
  return y->x.x;
}
