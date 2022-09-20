enum AA {
  const0 = 0,
  const1 = 1,
};

int var[const1];

int f()
{
  return var[0];
}
