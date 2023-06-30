/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f" }*/

char CRYPTO_strdup(const char *, const char *, int);

#define OPENSSL_strdup(str) \
        CRYPTO_strdup(str, "aaaaa", __LINE__)

int f()
{
  return OPENSSL_strdup("aaa");
}

/* { dg-final { scan-tree-dump "static char \(\*klp_CRYPTO_strdup\)\(const char \*, const char \*, int\);" } } */
/* { dg-final { scan-tree-dump "klp_CRYPTO_strdup\(str," } } */
