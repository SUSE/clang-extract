typedef enum {
  LEFT,
  RIGHT
} Hand;

struct Player {
  Hand mainhand;
};

int f(struct Player *p)
{
  return p->mainhand;
}

