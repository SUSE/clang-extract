enum
{
  CONST0,
};

enum
{
  ANOTHER_CONST = 1 << CONST0,
};

int f()
{
  return ANOTHER_CONST;
}
