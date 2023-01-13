enum
{
  CONST0,
  ANOTHER_CONST = CONST0,
};

int f()
{
  return ANOTHER_CONST;
}
