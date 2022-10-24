char CRYPTO_strdup(const char *, const char *, int);

#define OPENSSL_strdup(str) \
        CRYPTO_strdup(str, "aaaaa", __LINE__)

int f()
{
  return OPENSSL_strdup("aaa");
}
