/* { dg-options "-DCE_EXTRACT_FUNCTIONS=ossl_cms_RecipientInfo_kari_init -DCE_MACRO_EXTERNALIZATION -DCE_EXPORT_SYMBOLS=CMS_RecipientEncryptedKey_it,CMS_RecipientKeyIdentifier_it,CMS_KeyAgreeRecipientInfo_it" }*/

/** clang-extract: from /usr/include/bits/types.h:41:1 */
typedef signed int __int32_t;

/** clang-extract: from /usr/include/bits/stdint-intn.h:26:1 */
typedef __int32_t int32_t;

/** clang-extract: from include/openssl/stack.h:23:1 */
typedef struct stack_st OPENSSL_STACK;

/** clang-extract: from include/openssl/stack.h:49:1 */
int OPENSSL_sk_push(OPENSSL_STACK *st, const void *data);

/** clang-extract: from include/openssl/safestack.h:31:10 */
#define STACK_OF(type) struct stack_st_##type

/** clang-extract: from crypto/cms/cms_kari.c:1263:9 */
struct asn1_string_st;/* Full definition was removed.  */

/** clang-extract: from include/openssl/types.h:66:1 */
typedef struct asn1_string_st ASN1_OCTET_STRING;

/** clang-extract: from include/openssl/types.h:75:1 */
typedef struct asn1_string_st ASN1_GENERALIZEDTIME;

/** clang-extract: from include/openssl/types.h:87:1 */
typedef struct ASN1_ITEM_st ASN1_ITEM;

/** clang-extract: from include/openssl/types.h:111:1 */
typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

/** clang-extract: from include/openssl/types.h:116:1 */
typedef struct evp_pkey_st EVP_PKEY;

/** clang-extract: from include/openssl/types.h:122:1 */
typedef struct evp_pkey_ctx_st EVP_PKEY_CTX;

/** clang-extract: from include/openssl/types.h:169:1 */
typedef struct x509_st X509;

/** clang-extract: from include/openssl/types.h:170:1 */
typedef struct X509_algor_st X509_ALGOR;

/** clang-extract: from include/openssl/types.h:227:1 */
typedef struct ossl_lib_ctx_st OSSL_LIB_CTX;

/** clang-extract: from /usr/lib64/clang/23/include/__stddef_null.h:26:9 */
#define NULL ((void*)0)

/** clang-extract: from include/openssl/asn1.h:279:1 */
typedef struct ASN1_VALUE_st ASN1_VALUE;

/** clang-extract: from include/openssl/asn1.h:363:10 */
#define CHECKED_PTR_OF(type, p) \
    ((void*) (1 ? p : (type*)0))

/** clang-extract: from include/openssl/asn1.h:427:10 */
#define ASN1_ITEM_rptr(ref) (ref##_it())

/** clang-extract: from include/openssl/asn1.h:756:1 */
extern ASN1_OCTET_STRING *ASN1_OCTET_STRING_new(void);

/** clang-extract: from include/openssl/asn1.h:928:10 */
#define M_ASN1_new_of(type) (type *)ASN1_item_new(ASN1_ITEM_rptr(type))

/** clang-extract: from include/openssl/asn1.h:929:10 */
#define M_ASN1_free_of(x, type) \
                ASN1_item_free(CHECKED_PTR_OF(type, x), ASN1_ITEM_rptr(type))

/** clang-extract: from include/openssl/asn1.h:1028:1 */
ASN1_VALUE *ASN1_item_new(const ASN1_ITEM *it);

/** clang-extract: from include/openssl/asn1.h:1031:1 */
void ASN1_item_free(ASN1_VALUE *val, const ASN1_ITEM *it);

/** clang-extract: from include/openssl/evp.h:1438:1 */
int EVP_PKEY_up_ref(EVP_PKEY *pkey);

/** clang-extract: from include/openssl/evp.h:1440:1 */
void EVP_PKEY_free(EVP_PKEY *pkey);

/** clang-extract: from include/openssl/evp.h:1870:1 */
EVP_PKEY_CTX *EVP_PKEY_CTX_new_from_pkey(OSSL_LIB_CTX *libctx,
                                         EVP_PKEY *pkey, const char *propquery);

/** clang-extract: from include/openssl/evp.h:1873:1 */
void EVP_PKEY_CTX_free(EVP_PKEY_CTX *ctx);

/** clang-extract: from include/openssl/evp.h:2034:1 */
int EVP_PKEY_derive_init(EVP_PKEY_CTX *ctx);

/** clang-extract: from include/openssl/evp.h:2096:1 */
int EVP_PKEY_keygen_init(EVP_PKEY_CTX *ctx);

/** clang-extract: from include/openssl/evp.h:2097:1 */
int EVP_PKEY_keygen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey);

/** clang-extract: from include/openssl/cms.h:40:1 */
typedef struct CMS_RecipientInfo_st CMS_RecipientInfo;

/** clang-extract: from include/openssl/cms.h:43:1 */
typedef struct CMS_RecipientEncryptedKey_st CMS_RecipientEncryptedKey;

/** clang-extract: from include/openssl/cms.h:44:1 */
typedef struct CMS_OtherKeyAttribute_st CMS_OtherKeyAttribute;

/** clang-extract: from include/openssl/cms.h:72:1 */
struct stack_st_CMS_RecipientEncryptedKey;/* Full definition was removed.  */

/** clang-extract: from include/openssl/cms.h:72:1 */
__attribute__((unused)) static inline CMS_RecipientEncryptedKey *ossl_check_CMS_RecipientEncryptedKey_type(CMS_RecipientEncryptedKey *ptr) {
    return ptr;
}

/** clang-extract: from include/openssl/cms.h:72:1 */
__attribute__((unused)) static inline OPENSSL_STACK *ossl_check_CMS_RecipientEncryptedKey_sk_type(struct stack_st_CMS_RecipientEncryptedKey *sk) {
    return (OPENSSL_STACK *)sk;
}

/** clang-extract: from include/openssl/cms.h:83:9 */
#define sk_CMS_RecipientEncryptedKey_push(sk, ptr) OPENSSL_sk_push(ossl_check_CMS_RecipientEncryptedKey_sk_type(sk), ossl_check_CMS_RecipientEncryptedKey_type(ptr))

/** clang-extract: from include/openssl/cms.h:167:10 */
#define CMS_RECIPINFO_AGREE             1

/** clang-extract: from include/openssl/cms.h:193:10 */
#define CMS_USE_KEYID                   0x10000

/** clang-extract: from include/openssl/cms.h:198:10 */
#define CMS_USE_ORIGINATOR_KEYID        0x200000

/** clang-extract: from crypto/cms/cms_local.h:21:1 */
typedef struct CMS_IssuerAndSerialNumber_st CMS_IssuerAndSerialNumber;

/** clang-extract: from crypto/cms/cms_local.h:33:1 */
typedef struct CMS_KeyTransRecipientInfo_st CMS_KeyTransRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:34:1 */
typedef struct CMS_OriginatorPublicKey_st CMS_OriginatorPublicKey;

/** clang-extract: from crypto/cms/cms_local.h:35:1 */
typedef struct CMS_OriginatorIdentifierOrKey_st CMS_OriginatorIdentifierOrKey;

/** clang-extract: from crypto/cms/cms_local.h:36:1 */
typedef struct CMS_KeyAgreeRecipientInfo_st CMS_KeyAgreeRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:37:1 */
typedef struct CMS_RecipientKeyIdentifier_st CMS_RecipientKeyIdentifier;

/** clang-extract: from crypto/cms/cms_local.h:38:1 */
typedef struct CMS_KeyAgreeRecipientIdentifier_st
    CMS_KeyAgreeRecipientIdentifier;

/** clang-extract: from crypto/cms/cms_local.h:41:1 */
typedef struct CMS_KEKRecipientInfo_st CMS_KEKRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:42:1 */
typedef struct CMS_PasswordRecipientInfo_st CMS_PasswordRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:43:1 */
typedef struct CMS_OtherRecipientInfo_st CMS_OtherRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:45:1 */
typedef struct CMS_CTX_st CMS_CTX;

/** clang-extract: from crypto/cms/cms_local.h:144:1 */
struct CMS_RecipientInfo_st {
    int type;
    union {
        CMS_KeyTransRecipientInfo *ktri;
        CMS_KeyAgreeRecipientInfo *kari;
        CMS_KEKRecipientInfo *kekri;
        CMS_PasswordRecipientInfo *pwri;
        CMS_OtherRecipientInfo *ori;
    } d;
};

/** clang-extract: from crypto/cms/cms_local.h:170:1 */
struct CMS_KeyAgreeRecipientInfo_st {
    int32_t version;
    CMS_OriginatorIdentifierOrKey *originator;
    ASN1_OCTET_STRING *ukm;
    X509_ALGOR *keyEncryptionAlgorithm;
    STACK_OF(CMS_RecipientEncryptedKey) *recipientEncryptedKeys;
    /* Public key context associated with current operation */
    EVP_PKEY_CTX *pctx;
    /* Cipher context for CEK wrapping */
    EVP_CIPHER_CTX *ctx;
    const CMS_CTX *cms_ctx;
};

/** clang-extract: from crypto/cms/cms_local.h:183:1 */
struct CMS_OriginatorIdentifierOrKey_st {
    int type;
    union {
        CMS_IssuerAndSerialNumber *issuerAndSerialNumber;
        ASN1_OCTET_STRING *subjectKeyIdentifier;
        CMS_OriginatorPublicKey *originatorKey;
    } d;
};

/** clang-extract: from crypto/cms/cms_local.h:197:1 */
struct CMS_RecipientEncryptedKey_st {
    CMS_KeyAgreeRecipientIdentifier *rid;
    ASN1_OCTET_STRING *encryptedKey;
    /* Public key associated with this recipient */
    EVP_PKEY *pkey;
};

/** clang-extract: from crypto/cms/cms_local.h:204:1 */
struct CMS_KeyAgreeRecipientIdentifier_st {
    int type;
    union {
        CMS_IssuerAndSerialNumber *issuerAndSerialNumber;
        CMS_RecipientKeyIdentifier *rKeyId;
    } d;
};

/** clang-extract: from crypto/cms/cms_local.h:212:1 */
struct CMS_RecipientKeyIdentifier_st {
    ASN1_OCTET_STRING *subjectKeyIdentifier;
    ASN1_GENERALIZEDTIME *date;
    CMS_OtherKeyAttribute *other;
};

/** clang-extract: from crypto/cms/cms_local.h:385:10 */
#define CMS_REK_ISSUER_SERIAL           0

/** clang-extract: from crypto/cms/cms_local.h:386:10 */
#define CMS_REK_KEYIDENTIFIER           1

/** clang-extract: from crypto/cms/cms_local.h:389:10 */
#define CMS_OIK_KEYIDENTIFIER           1

/** clang-extract: from crypto/cms/cms_local.h:394:1 */
__attribute__((used)) static OSSL_LIB_CTX *(*klpe_ossl_cms_ctx_get0_libctx)(const CMS_CTX *);

/** clang-extract: from crypto/cms/cms_local.h:395:1 */
__attribute__((used)) static const char *(*klpe_ossl_cms_ctx_get0_propq)(const CMS_CTX *);

/** clang-extract: from crypto/cms/cms_local.h:434:1 */
__attribute__((used)) static int (*klpe_ossl_cms_set1_ias)(CMS_IssuerAndSerialNumber **, X509 *);

/** clang-extract: from crypto/cms/cms_local.h:435:1 */
__attribute__((used)) static int (*klpe_ossl_cms_set1_keyid)(ASN1_OCTET_STRING **, X509 *);

/** clang-extract: from crypto/cms/cms_local.h:465:1 */
int ossl_cms_RecipientInfo_kari_init(CMS_RecipientInfo *ri, X509 *recip,
                                     EVP_PKEY *recipPubKey, X509 *originator,
                                     EVP_PKEY *originatorPrivKey,
                                     unsigned int flags,
                                     const CMS_CTX *ctx);

/** clang-extract: from crypto/cms/cms_local.h:499:1 */
extern const ASN1_ITEM *CMS_KeyAgreeRecipientInfo_it(void);

/** clang-extract: from crypto/cms/cms_local.h:505:1 */
extern const ASN1_ITEM *CMS_RecipientEncryptedKey_it(void);

/** clang-extract: from crypto/cms/cms_local.h:506:1 */
extern const ASN1_ITEM *CMS_RecipientKeyIdentifier_it(void);

/** clang-extract: from crypto/cms/cms_kari.c:283:1 */
static int cms_kari_create_ephemeral_key(CMS_KeyAgreeRecipientInfo *kari,
                                         EVP_PKEY *pk)
{
    EVP_PKEY_CTX *pctx = NULL;
    EVP_PKEY *ekey = NULL;
    int rv = 0;
    const CMS_CTX *ctx = kari->cms_ctx;
    OSSL_LIB_CTX *libctx = (*klpe_ossl_cms_ctx_get0_libctx)(ctx);
    const char *propq = (*klpe_ossl_cms_ctx_get0_propq)(ctx);

    pctx = EVP_PKEY_CTX_new_from_pkey(libctx, pk, propq);
    if (pctx == NULL)
        goto err;
    if (EVP_PKEY_keygen_init(pctx) <= 0)
        goto err;
    if (EVP_PKEY_keygen(pctx, &ekey) <= 0)
        goto err;
    EVP_PKEY_CTX_free(pctx);
    pctx = EVP_PKEY_CTX_new_from_pkey(libctx, ekey, propq);
    if (pctx == NULL)
        goto err;
    if (EVP_PKEY_derive_init(pctx) <= 0)
        goto err;
    kari->pctx = pctx;
    rv = 1;
 err:
    if (!rv)
        EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_free(ekey);
    return rv;
}

/** clang-extract: from crypto/cms/cms_kari.c:316:1 */
static int cms_kari_set_originator_private_key(CMS_KeyAgreeRecipientInfo *kari,
                                               EVP_PKEY *originatorPrivKey )
{
    EVP_PKEY_CTX *pctx = NULL;
    int rv = 0;
    const CMS_CTX *ctx = kari->cms_ctx;

    pctx = EVP_PKEY_CTX_new_from_pkey((*klpe_ossl_cms_ctx_get0_libctx)(ctx),
                                      originatorPrivKey,
                                      (*klpe_ossl_cms_ctx_get0_propq)(ctx));
    if (pctx == NULL)
        goto err;
    if (EVP_PKEY_derive_init(pctx) <= 0)
         goto err;

    kari->pctx = pctx;
    rv = 1;
 err:
    if (rv == 0)
        EVP_PKEY_CTX_free(pctx);
    return rv;
}

/** clang-extract: from crypto/cms/cms_kari.c:341:1 */
int ossl_cms_RecipientInfo_kari_init(CMS_RecipientInfo *ri,  X509 *recip,
                                     EVP_PKEY *recipPubKey, X509 *originator,
                                     EVP_PKEY *originatorPrivKey,
                                     unsigned int flags, const CMS_CTX *ctx)
{
    CMS_KeyAgreeRecipientInfo *kari;
    CMS_RecipientEncryptedKey *rek = NULL;

    ri->d.kari = M_ASN1_new_of(CMS_KeyAgreeRecipientInfo);
    if (ri->d.kari == NULL)
        return 0;
    ri->type = CMS_RECIPINFO_AGREE;

    kari = ri->d.kari;
    kari->version = 3;
    kari->cms_ctx = ctx;

    rek = M_ASN1_new_of(CMS_RecipientEncryptedKey);
    if (rek == NULL)
        return 0;

    if (!sk_CMS_RecipientEncryptedKey_push(kari->recipientEncryptedKeys, rek)) {
        M_ASN1_free_of(rek, CMS_RecipientEncryptedKey);
        return 0;
    }

    if (flags & CMS_USE_KEYID) {
        rek->rid->type = CMS_REK_KEYIDENTIFIER;
        rek->rid->d.rKeyId = M_ASN1_new_of(CMS_RecipientKeyIdentifier);
        if (rek->rid->d.rKeyId == NULL)
            return 0;
        if (!(*klpe_ossl_cms_set1_keyid)(&rek->rid->d.rKeyId->subjectKeyIdentifier, recip))
            return 0;
    } else {
        rek->rid->type = CMS_REK_ISSUER_SERIAL;
        if (!(*klpe_ossl_cms_set1_ias)(&rek->rid->d.issuerAndSerialNumber, recip))
            return 0;
    }

    if (originatorPrivKey == NULL && originator == NULL) {
        /* Create ephemeral key */
        if (!cms_kari_create_ephemeral_key(kari, recipPubKey))
            return 0;
    } else {
        /* Use originator key */
        CMS_OriginatorIdentifierOrKey *oik = ri->d.kari->originator;

        if (originatorPrivKey == NULL || originator == NULL)
            return 0;

        if (flags & CMS_USE_ORIGINATOR_KEYID) {
             oik->type = CMS_OIK_KEYIDENTIFIER;
             oik->d.subjectKeyIdentifier = ASN1_OCTET_STRING_new();
             if (oik->d.subjectKeyIdentifier == NULL)
                  return 0;
             if (!(*klpe_ossl_cms_set1_keyid)(&oik->d.subjectKeyIdentifier, originator))
                  return 0;
        } else {
             oik->type = CMS_REK_ISSUER_SERIAL;
             if (!(*klpe_ossl_cms_set1_ias)(&oik->d.issuerAndSerialNumber, originator))
                  return 0;
        }

        if (!cms_kari_set_originator_private_key(kari, originatorPrivKey))
            return 0;
    }

    if (!EVP_PKEY_up_ref(recipPubKey))
        return 0;

    rek->pkey = recipPubKey;
    return 1;
}


/* { dg-final { scan-tree-dump "klpe_CMS_RecipientEncryptedKey_it" } } */
/* { dg-final { scan-tree-dump "klpe_CMS_KeyAgreeRecipientInfo_it" } } */
/* { dg-final { scan-tree-dump "klpe_CMS_RecipientKeyIdentifier_it" } } */

/* { dg-final { scan-tree-dump "#define CMS_RecipientEncryptedKey_it\t\(\*klpe_CMS_RecipientEncryptedKey_it\)" } } */
/* { dg-final { scan-tree-dump "#define CMS_KeyAgreeRecipientInfo_it\t\(\*klpe_CMS_KeyAgreeRecipientInfo_it\)" } } */
/* { dg-final { scan-tree-dump "#define CMS_RecipientKeyIdentifier_it\t\(\*klpe_CMS_RecipientKeyIdentifier_it\)" } } */
