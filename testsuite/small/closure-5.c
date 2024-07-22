/* { dg-options "-DCE_EXTRACT_FUNCTIONS=ext2fs_casefold_cmp -DCE_NO_EXTERNALIZATION" }*/

typedef long unsigned int size_t;

struct struct_ext2_filsys {
 const struct ext2fs_nls_table *encoding;
};

extern const struct ext2fs_nls_table *ext2fs_load_nls_table(int encoding);
extern int ext2fs_casefold_cmp(const struct ext2fs_nls_table *table,
          const unsigned char *str1, size_t len1,
          const unsigned char *str2, size_t len2);

struct ext2fs_nls_table {
 int version;
 const struct ext2fs_nls_ops *ops;
};

int ext2fs_casefold_cmp(const struct ext2fs_nls_table *table,
          const unsigned char *str1, size_t len1,
          const unsigned char *str2, size_t len2)
{
  (void) *table;
  return 0;
}

/* { dg-final { scan-tree-dump "struct struct_ext2_filsys {\n *const struct ext2fs_nls_table \*encoding;\n};" } } */
