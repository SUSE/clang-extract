/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

static const struct mount_opts
{
  int token;
  int mount_opts;
  int flags;
} ext4_mount_opts[] = {
  {1, 1, 1}
};

int f(void)
{
  return ext4_mount_opts[0].token;
}

/* { dg-final { scan-tree-dump "static const struct mount_opts\n{\n *int token;\n *int mount_opts;\n *int flags;\n} ext4_mount_opts\[\] = {\n *{1, 1, 1}\n};" } } */
/* { dg-final { scan-tree-dump "return ext4_mount_opts\[0\]\.token;" } } */
