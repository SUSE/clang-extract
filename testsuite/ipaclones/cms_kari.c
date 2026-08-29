/* { dg-options "-DCE_EXTRACT_FUNCTIONS=cms_kek_cipher -DCE_IPACLONES_PATH=$test_dir/cms_kari.c.000i.ipa-clones" }*/

/* This file was processed with clang-extract, originally from openssl-1_1 */

/** clang-extract: from /usr/lib64/clang/22/include/__stddef_size_t.h:18:1 */
typedef __SIZE_TYPE__ size_t;

/** clang-extract: from /usr/include/bits/types.h:41:1 */
typedef signed int __int32_t;

/** clang-extract: from /usr/include/bits/stdint-intn.h:26:1 */
typedef __int32_t int32_t;

#define OPENSSL_FILE __FILE__
#define OPENSSL_LINE __LINE__
/** clang-extract: from include/openssl/stack.h:17:1 */
typedef struct stack_st OPENSSL_STACK;

/** clang-extract: from include/openssl/stack.h:23:1 */
int OPENSSL_sk_num(const OPENSSL_STACK *);

/** clang-extract: from include/openssl/stack.h:24:1 */
void *OPENSSL_sk_value(const OPENSSL_STACK *, int);

/** clang-extract: from include/openssl/stack.h:42:1 */
int OPENSSL_sk_push(OPENSSL_STACK *st, const void *data);

#define STACK_OF(type) struct stack_st_##type
/** clang-extract: from include/openssl/ossl_typ.h:40:1 */
typedef struct asn1_string_st ASN1_INTEGER;

/** clang-extract: from include/openssl/ossl_typ.h:41:1 */
typedef struct asn1_string_st ASN1_ENUMERATED;

/** clang-extract: from include/openssl/ossl_typ.h:42:1 */
typedef struct asn1_string_st ASN1_BIT_STRING;

/** clang-extract: from include/openssl/ossl_typ.h:43:1 */
typedef struct asn1_string_st ASN1_OCTET_STRING;

/** clang-extract: from include/openssl/ossl_typ.h:44:1 */
typedef struct asn1_string_st ASN1_PRINTABLESTRING;

/** clang-extract: from include/openssl/ossl_typ.h:45:1 */
typedef struct asn1_string_st ASN1_T61STRING;

/** clang-extract: from include/openssl/ossl_typ.h:46:1 */
typedef struct asn1_string_st ASN1_IA5STRING;

/** clang-extract: from include/openssl/ossl_typ.h:47:1 */
typedef struct asn1_string_st ASN1_GENERALSTRING;

/** clang-extract: from include/openssl/ossl_typ.h:48:1 */
typedef struct asn1_string_st ASN1_UNIVERSALSTRING;

/** clang-extract: from include/openssl/ossl_typ.h:49:1 */
typedef struct asn1_string_st ASN1_BMPSTRING;

/** clang-extract: from include/openssl/ossl_typ.h:50:1 */
typedef struct asn1_string_st ASN1_UTCTIME;

/** clang-extract: from include/openssl/ossl_typ.h:52:1 */
typedef struct asn1_string_st ASN1_GENERALIZEDTIME;

/** clang-extract: from include/openssl/ossl_typ.h:53:1 */
typedef struct asn1_string_st ASN1_VISIBLESTRING;

/** clang-extract: from include/openssl/ossl_typ.h:54:1 */
typedef struct asn1_string_st ASN1_UTF8STRING;

/** clang-extract: from include/openssl/ossl_typ.h:55:1 */
typedef struct asn1_string_st ASN1_STRING;

/** clang-extract: from include/openssl/ossl_typ.h:56:1 */
typedef int ASN1_BOOLEAN;

/** clang-extract: from include/openssl/ossl_typ.h:60:1 */
typedef struct asn1_object_st ASN1_OBJECT;

/** clang-extract: from include/openssl/ossl_typ.h:62:1 */
typedef struct ASN1_ITEM_st ASN1_ITEM;

/** clang-extract: from include/openssl/ossl_typ.h:89:1 */
typedef struct evp_cipher_st EVP_CIPHER;

/** clang-extract: from include/openssl/ossl_typ.h:90:1 */
typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

/** clang-extract: from include/openssl/ossl_typ.h:93:1 */
typedef struct evp_pkey_st EVP_PKEY;

/** clang-extract: from include/openssl/ossl_typ.h:98:1 */
typedef struct evp_pkey_ctx_st EVP_PKEY_CTX;

/** clang-extract: from include/openssl/ossl_typ.h:123:1 */
typedef struct x509_st X509;

/** clang-extract: from include/openssl/ossl_typ.h:124:1 */
typedef struct X509_algor_st X509_ALGOR;

/** clang-extract: from include/openssl/ossl_typ.h:128:1 */
typedef struct X509_name_st X509_NAME;

/** clang-extract: from include/openssl/ossl_typ.h:149:1 */
typedef struct engine_st ENGINE;

#define OPENSSL_malloc(num) \
        CRYPTO_malloc(num, OPENSSL_FILE, OPENSSL_LINE)
#define OPENSSL_clear_free(addr, num) \
        CRYPTO_clear_free(addr, num, OPENSSL_FILE, OPENSSL_LINE)
#define OPENSSL_free(addr) \
        CRYPTO_free(addr, OPENSSL_FILE, OPENSSL_LINE)
/** clang-extract: from include/openssl/crypto.h:266:1 */
void *CRYPTO_malloc(size_t num, const char *file, int line);

/** clang-extract: from include/openssl/crypto.h:271:1 */
void CRYPTO_free(void *ptr, const char *file, int line);

/** clang-extract: from include/openssl/crypto.h:272:1 */
void CRYPTO_clear_free(void *ptr, size_t num, const char *file, int line);

/** clang-extract: from include/openssl/crypto.h:289:1 */
void OPENSSL_cleanse(void *ptr, size_t len);

#define NULL ((void*)0)
#define ERR_PUT_error(a,b,c,d,e)        ERR_put_error(a,b,c,d,e)
#define ERR_LIB_CMS             46
#define CMSerr(f,r) ERR_PUT_error(ERR_LIB_CMS,(f),(r),OPENSSL_FILE,OPENSSL_LINE)
/** clang-extract: from include/openssl/err.h:220:1 */
void ERR_put_error(int lib, int func, int reason, const char *file, int line);

/** clang-extract: from include/openssl/asn1.h:146:1 */
struct asn1_string_st {
    int length;
    int type;
    unsigned char *data;
    /*
     * The value of the following field depends on the type being held.  It
     * is mostly being used for BIT_STRING so if the input data has a
     * non-zero 'unused bits' value, it will be handled correctly
     */
    long flags;
};

/** clang-extract: from include/openssl/asn1.h:213:1 */
typedef struct ASN1_VALUE_st ASN1_VALUE;

#define CHECKED_PTR_OF(type, p) \
    ((void*) (1 ? p : (type*)0))
#define ASN1_ITEM_rptr(ref) (&(ref##_it))
/** clang-extract: from include/openssl/asn1.h:444:1 */
typedef struct asn1_type_st {
    int type;
    union {
        char *ptr;
        ASN1_BOOLEAN boolean;
        ASN1_STRING *asn1_string;
        ASN1_OBJECT *object;
        ASN1_INTEGER *integer;
        ASN1_ENUMERATED *enumerated;
        ASN1_BIT_STRING *bit_string;
        ASN1_OCTET_STRING *octet_string;
        ASN1_PRINTABLESTRING *printablestring;
        ASN1_T61STRING *t61string;
        ASN1_IA5STRING *ia5string;
        ASN1_GENERALSTRING *generalstring;
        ASN1_BMPSTRING *bmpstring;
        ASN1_UNIVERSALSTRING *universalstring;
        ASN1_UTCTIME *utctime;
        ASN1_GENERALIZEDTIME *generalizedtime;
        ASN1_VISIBLESTRING *visiblestring;
        ASN1_UTF8STRING *utf8string;
        /*
         * set and sequence are left complete and still contain the set or
         * sequence bytes
         */
        ASN1_STRING *set;
        ASN1_STRING *sequence;
        ASN1_VALUE *asn1_value;
    } value;
} ASN1_TYPE;

/** clang-extract: from include/openssl/asn1.h:550:1 */
void ASN1_STRING_set0(ASN1_STRING *str, void *data, int len);

#define M_ASN1_new_of(type) (type *)ASN1_item_new(ASN1_ITEM_rptr(type))
#define M_ASN1_free_of(x, type) \
                ASN1_item_free(CHECKED_PTR_OF(type, x), ASN1_ITEM_rptr(type))
/** clang-extract: from include/openssl/asn1.h:806:1 */
ASN1_VALUE *ASN1_item_new(const ASN1_ITEM *it);

/** clang-extract: from include/openssl/asn1.h:807:1 */
void ASN1_item_free(ASN1_VALUE *val, const ASN1_ITEM *it);

#define EVP_MAX_KEY_LENGTH              64
#define NID_des_ede3_cbc                44
#define EVP_CIPH_WRAP_MODE              0x10002
#define EVP_CIPH_MODE                   0xF0007
/** clang-extract: from include/openssl/evp.h:468:1 */
int EVP_CIPHER_key_length(const EVP_CIPHER *cipher);

/** clang-extract: from include/openssl/evp.h:470:1 */
unsigned long EVP_CIPHER_flags(const EVP_CIPHER *cipher);

#define EVP_CIPHER_mode(e)              (EVP_CIPHER_flags(e) & EVP_CIPH_MODE)
/** clang-extract: from include/openssl/evp.h:473:1 */
const EVP_CIPHER *EVP_CIPHER_CTX_cipher(const EVP_CIPHER_CTX *ctx);

/** clang-extract: from include/openssl/evp.h:477:1 */
int EVP_CIPHER_CTX_key_length(const EVP_CIPHER_CTX *ctx);

#define EVP_CIPHER_CTX_mode(c)         EVP_CIPHER_mode(EVP_CIPHER_CTX_cipher(c))
/** clang-extract: from include/openssl/evp.h:582:12 */
int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx,
                                  const EVP_CIPHER *cipher, ENGINE *impl,
                                  const unsigned char *key,
                                  const unsigned char *iv);

/** clang-extract: from include/openssl/evp.h:609:12 */
int EVP_CipherInit_ex(EVP_CIPHER_CTX *ctx,
                                 const EVP_CIPHER *cipher, ENGINE *impl,
                                 const unsigned char *key,
                                 const unsigned char *iv, int enc);

/** clang-extract: from include/openssl/evp.h:613:8 */
int EVP_CipherUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out,
                            int *outl, const unsigned char *in, int inl);

/** clang-extract: from include/openssl/evp.h:680:1 */
int EVP_CIPHER_CTX_reset(EVP_CIPHER_CTX *c);

/** clang-extract: from include/openssl/evp.h:758:1 */
const EVP_CIPHER *EVP_des_ede3_wrap(void);

/** clang-extract: from include/openssl/evp.h:820:1 */
const EVP_CIPHER *EVP_aes_128_wrap(void);

/** clang-extract: from include/openssl/evp.h:835:1 */
const EVP_CIPHER *EVP_aes_192_wrap(void);

/** clang-extract: from include/openssl/evp.h:851:1 */
const EVP_CIPHER *EVP_aes_256_wrap(void);

/** clang-extract: from include/openssl/evp.h:1040:1 */
int EVP_PKEY_up_ref(EVP_PKEY *pkey);

/** clang-extract: from include/openssl/evp.h:1041:1 */
void EVP_PKEY_free(EVP_PKEY *pkey);

/** clang-extract: from include/openssl/evp.h:1073:1 */
int EVP_CIPHER_type(const EVP_CIPHER *ctx);

/** clang-extract: from include/openssl/evp.h:1341:1 */
EVP_PKEY_CTX *EVP_PKEY_CTX_new(EVP_PKEY *pkey, ENGINE *e);

/** clang-extract: from include/openssl/evp.h:1344:1 */
void EVP_PKEY_CTX_free(EVP_PKEY_CTX *ctx);

/** clang-extract: from include/openssl/evp.h:1407:1 */
int EVP_PKEY_derive_init(EVP_PKEY_CTX *ctx);

/** clang-extract: from include/openssl/evp.h:1408:1 */
int EVP_PKEY_derive_set_peer(EVP_PKEY_CTX *ctx, EVP_PKEY *peer);

/** clang-extract: from include/openssl/evp.h:1409:1 */
int EVP_PKEY_derive(EVP_PKEY_CTX *ctx, unsigned char *key, size_t *keylen);

/** clang-extract: from include/openssl/evp.h:1415:1 */
int EVP_PKEY_keygen_init(EVP_PKEY_CTX *ctx);

/** clang-extract: from include/openssl/evp.h:1416:1 */
int EVP_PKEY_keygen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey);

/** clang-extract: from include/openssl/x509.h:89:1 */
struct stack_st_X509_ATTRIBUTE;/* Full definition was removed.  */

#define CMS_F_CMS_RECIPIENTINFO_KARI_ENCRYPT             178
#define CMS_F_CMS_RECIPIENTINFO_KARI_GET0_ALG            175
#define CMS_F_CMS_RECIPIENTINFO_KARI_GET0_ORIG_ID        173
#define CMS_F_CMS_RECIPIENTINFO_KARI_GET0_REKS           172
#define CMS_F_CMS_RECIPIENTINFO_KARI_ORIG_ID_CMP         174
#define CMS_R_NOT_KEY_AGREEMENT                          181
/** clang-extract: from include/openssl/cms.h:23:1 */
typedef struct CMS_ContentInfo_st CMS_ContentInfo;

/** clang-extract: from include/openssl/cms.h:27:1 */
typedef struct CMS_RecipientInfo_st CMS_RecipientInfo;

/** clang-extract: from include/openssl/cms.h:30:1 */
typedef struct CMS_RecipientEncryptedKey_st CMS_RecipientEncryptedKey;

/** clang-extract: from include/openssl/cms.h:31:1 */
typedef struct CMS_OtherKeyAttribute_st CMS_OtherKeyAttribute;

/** clang-extract: from include/openssl/cms.h:34:1 */
struct stack_st_CMS_RecipientEncryptedKey;/* Full definition was removed.  */

/** clang-extract: from include/openssl/cms.h:34:1 */
__attribute__((unused)) static inline int sk_CMS_RecipientEncryptedKey_num(const struct stack_st_CMS_RecipientEncryptedKey *sk) {
    return OPENSSL_sk_num((const OPENSSL_STACK *)sk);
}

/** clang-extract: from include/openssl/cms.h:34:1 */
__attribute__((unused)) static inline CMS_RecipientEncryptedKey *sk_CMS_RecipientEncryptedKey_value(const struct stack_st_CMS_RecipientEncryptedKey *sk, int idx) {
    return (CMS_RecipientEncryptedKey *)OPENSSL_sk_value((const OPENSSL_STACK *)sk, idx);
}

/** clang-extract: from include/openssl/cms.h:34:1 */
__attribute__((unused)) static inline int sk_CMS_RecipientEncryptedKey_push(struct stack_st_CMS_RecipientEncryptedKey *sk, CMS_RecipientEncryptedKey *ptr) {
    return OPENSSL_sk_push((OPENSSL_STACK *)sk, (const void *)ptr);
}

/** clang-extract: from include/openssl/cms.h:35:1 */
struct stack_st_CMS_RecipientInfo;/* Full definition was removed.  */

#define CMS_RECIPINFO_AGREE             1
#define CMS_USE_KEYID                   0x10000
/** clang-extract: from include/openssl/cms.h:299:1 */
int CMS_RecipientInfo_kari_get0_alg(CMS_RecipientInfo *ri,
                                    X509_ALGOR **palg,
                                    ASN1_OCTET_STRING **pukm);

/** clang-extract: from include/openssl/cms.h:302:1 */
STACK_OF(CMS_RecipientEncryptedKey)
*CMS_RecipientInfo_kari_get0_reks(CMS_RecipientInfo *ri);

/** clang-extract: from include/openssl/cms.h:305:1 */
int CMS_RecipientInfo_kari_get0_orig_id(CMS_RecipientInfo *ri,
                                        X509_ALGOR **pubalg,
                                        ASN1_BIT_STRING **pubkey,
                                        ASN1_OCTET_STRING **keyid,
                                        X509_NAME **issuer,
                                        ASN1_INTEGER **sno);

/** clang-extract: from include/openssl/cms.h:312:1 */
int CMS_RecipientInfo_kari_orig_id_cmp(CMS_RecipientInfo *ri, X509 *cert);

/** clang-extract: from include/openssl/cms.h:314:1 */
int CMS_RecipientEncryptedKey_get0_id(CMS_RecipientEncryptedKey *rek,
                                      ASN1_OCTET_STRING **keyid,
                                      ASN1_GENERALIZEDTIME **tm,
                                      CMS_OtherKeyAttribute **other,
                                      X509_NAME **issuer, ASN1_INTEGER **sno);

/** clang-extract: from include/openssl/cms.h:319:1 */
int CMS_RecipientEncryptedKey_cert_cmp(CMS_RecipientEncryptedKey *rek,
                                       X509 *cert);

/** clang-extract: from include/openssl/cms.h:323:1 */
int CMS_RecipientInfo_kari_decrypt(CMS_ContentInfo *cms,
                                   CMS_RecipientInfo *ri,
                                   CMS_RecipientEncryptedKey *rek);

/** clang-extract: from crypto/cms/cms_local.h:21:1 */
typedef struct CMS_IssuerAndSerialNumber_st CMS_IssuerAndSerialNumber;

/** clang-extract: from crypto/cms/cms_local.h:24:1 */
typedef struct CMS_SignedData_st CMS_SignedData;

/** clang-extract: from crypto/cms/cms_local.h:26:1 */
typedef struct CMS_OriginatorInfo_st CMS_OriginatorInfo;

/** clang-extract: from crypto/cms/cms_local.h:27:1 */
typedef struct CMS_EncryptedContentInfo_st CMS_EncryptedContentInfo;

/** clang-extract: from crypto/cms/cms_local.h:28:1 */
typedef struct CMS_EnvelopedData_st CMS_EnvelopedData;

/** clang-extract: from crypto/cms/cms_local.h:29:1 */
typedef struct CMS_DigestedData_st CMS_DigestedData;

/** clang-extract: from crypto/cms/cms_local.h:30:1 */
typedef struct CMS_EncryptedData_st CMS_EncryptedData;

/** clang-extract: from crypto/cms/cms_local.h:31:1 */
typedef struct CMS_AuthenticatedData_st CMS_AuthenticatedData;

/** clang-extract: from crypto/cms/cms_local.h:32:1 */
typedef struct CMS_CompressedData_st CMS_CompressedData;

/** clang-extract: from crypto/cms/cms_local.h:34:1 */
typedef struct CMS_KeyTransRecipientInfo_st CMS_KeyTransRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:35:1 */
typedef struct CMS_OriginatorPublicKey_st CMS_OriginatorPublicKey;

/** clang-extract: from crypto/cms/cms_local.h:36:1 */
typedef struct CMS_OriginatorIdentifierOrKey_st CMS_OriginatorIdentifierOrKey;

/** clang-extract: from crypto/cms/cms_local.h:37:1 */
typedef struct CMS_KeyAgreeRecipientInfo_st CMS_KeyAgreeRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:38:1 */
typedef struct CMS_RecipientKeyIdentifier_st CMS_RecipientKeyIdentifier;

/** clang-extract: from crypto/cms/cms_local.h:39:1 */
typedef struct CMS_KeyAgreeRecipientIdentifier_st
    CMS_KeyAgreeRecipientIdentifier;

/** clang-extract: from crypto/cms/cms_local.h:42:1 */
typedef struct CMS_KEKRecipientInfo_st CMS_KEKRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:43:1 */
typedef struct CMS_PasswordRecipientInfo_st CMS_PasswordRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:44:1 */
typedef struct CMS_OtherRecipientInfo_st CMS_OtherRecipientInfo;

/** clang-extract: from crypto/cms/cms_local.h:47:1 */
struct CMS_ContentInfo_st {
    ASN1_OBJECT *contentType;
    union {
        ASN1_OCTET_STRING *data;
        CMS_SignedData *signedData;
        CMS_EnvelopedData *envelopedData;
        CMS_DigestedData *digestedData;
        CMS_EncryptedData *encryptedData;
        CMS_AuthenticatedData *authenticatedData;
        CMS_CompressedData *compressedData;
        ASN1_TYPE *other;
        /* Other types ... */
        void *otherData;
    } d;
};

/** clang-extract: from crypto/cms/cms_local.h:105:1 */
struct CMS_EnvelopedData_st {
    int32_t version;
    CMS_OriginatorInfo *originatorInfo;
    STACK_OF(CMS_RecipientInfo) *recipientInfos;
    CMS_EncryptedContentInfo *encryptedContentInfo;
    STACK_OF(X509_ATTRIBUTE) *unprotectedAttrs;
};

/** clang-extract: from crypto/cms/cms_local.h:118:1 */
struct CMS_EncryptedContentInfo_st {
    ASN1_OBJECT *contentType;
    X509_ALGOR *contentEncryptionAlgorithm;
    ASN1_OCTET_STRING *encryptedContent;
    /* Content encryption algorithm and key */
    const EVP_CIPHER *cipher;
    unsigned char *key;
    size_t keylen;
    /* Set to 1 if we are debugging decrypt and don't fake keys for MMA */
    int debug;
    /* Set to 1 if we have no cert and need extra safety measures for MMA */
    int havenocert;
};

/** clang-extract: from crypto/cms/cms_local.h:132:1 */
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

/** clang-extract: from crypto/cms/cms_local.h:157:1 */
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
};

/** clang-extract: from crypto/cms/cms_local.h:169:1 */
struct CMS_OriginatorIdentifierOrKey_st {
    int type;
    union {
        CMS_IssuerAndSerialNumber *issuerAndSerialNumber;
        ASN1_OCTET_STRING *subjectKeyIdentifier;
        CMS_OriginatorPublicKey *originatorKey;
    } d;
};

/** clang-extract: from crypto/cms/cms_local.h:178:1 */
struct CMS_OriginatorPublicKey_st {
    X509_ALGOR *algorithm;
    ASN1_BIT_STRING *publicKey;
};

/** clang-extract: from crypto/cms/cms_local.h:183:1 */
struct CMS_RecipientEncryptedKey_st {
    CMS_KeyAgreeRecipientIdentifier *rid;
    ASN1_OCTET_STRING *encryptedKey;
    /* Public key associated with this recipient */
    EVP_PKEY *pkey;
};

/** clang-extract: from crypto/cms/cms_local.h:190:1 */
struct CMS_KeyAgreeRecipientIdentifier_st {
    int type;
    union {
        CMS_IssuerAndSerialNumber *issuerAndSerialNumber;
        CMS_RecipientKeyIdentifier *rKeyId;
    } d;
};

/** clang-extract: from crypto/cms/cms_local.h:198:1 */
struct CMS_RecipientKeyIdentifier_st {
    ASN1_OCTET_STRING *subjectKeyIdentifier;
    ASN1_GENERALIZEDTIME *date;
    CMS_OtherKeyAttribute *other;
};

/** clang-extract: from crypto/cms/cms_local.h:310:1 */
struct CMS_IssuerAndSerialNumber_st {
    X509_NAME *issuer;
    ASN1_INTEGER *serialNumber;
};

#define CMS_REK_ISSUER_SERIAL           0
#define CMS_REK_KEYIDENTIFIER           1
#define CMS_OIK_ISSUER_SERIAL           0
#define CMS_OIK_KEYIDENTIFIER           1
#define CMS_OIK_PUBKEY                  2
/** clang-extract: from crypto/cms/cms_local.h:391:1 */
int cms_ias_cert_cmp(CMS_IssuerAndSerialNumber *ias, X509 *cert);

/** clang-extract: from crypto/cms/cms_local.h:392:1 */
int cms_keyid_cert_cmp(ASN1_OCTET_STRING *keyid, X509 *cert);

/** clang-extract: from crypto/cms/cms_local.h:393:1 */
int cms_set1_ias(CMS_IssuerAndSerialNumber **pias, X509 *cert);

/** clang-extract: from crypto/cms/cms_local.h:394:1 */
int cms_set1_keyid(ASN1_OCTET_STRING **pkeyid, X509 *cert);

/** clang-extract: from crypto/cms/cms_local.h:408:1 */
int cms_env_asn1_ctrl(CMS_RecipientInfo *ri, int cmd);

/** clang-extract: from crypto/cms/cms_local.h:411:1 */
int cms_RecipientInfo_kari_init(CMS_RecipientInfo *ri, X509 *recip,
                                EVP_PKEY *pk, unsigned int flags);

/** clang-extract: from crypto/cms/cms_local.h:413:1 */
int cms_RecipientInfo_kari_encrypt(CMS_ContentInfo *cms,
                                   CMS_RecipientInfo *ri);

/** clang-extract: from crypto/cms/cms_local.h:427:1 */
extern const ASN1_ITEM CMS_KeyAgreeRecipientInfo_it;

/** clang-extract: from crypto/cms/cms_local.h:429:1 */
extern const ASN1_ITEM CMS_OriginatorPublicKey_it;

/** clang-extract: from crypto/cms/cms_local.h:433:1 */
extern const ASN1_ITEM CMS_RecipientEncryptedKey_it;

/** clang-extract: from crypto/cms/cms_local.h:434:1 */
extern const ASN1_ITEM CMS_RecipientKeyIdentifier_it;

/** clang-extract: from crypto/cms/cms_kari.c:22:1 */
int CMS_RecipientInfo_kari_get0_alg(CMS_RecipientInfo *ri,
                                    X509_ALGOR **palg,
                                    ASN1_OCTET_STRING **pukm)
{
    if (ri->type != CMS_RECIPINFO_AGREE) {
        CMSerr(CMS_F_CMS_RECIPIENTINFO_KARI_GET0_ALG,
               CMS_R_NOT_KEY_AGREEMENT);
        return 0;
    }
    if (palg)
        *palg = ri->d.kari->keyEncryptionAlgorithm;
    if (pukm)
        *pukm = ri->d.kari->ukm;
    return 1;
}

/** clang-extract: from crypto/cms/cms_kari.c:40:1 */
STACK_OF(CMS_RecipientEncryptedKey)
*CMS_RecipientInfo_kari_get0_reks(CMS_RecipientInfo *ri)
{
    if (ri->type != CMS_RECIPINFO_AGREE) {
        CMSerr(CMS_F_CMS_RECIPIENTINFO_KARI_GET0_REKS,
               CMS_R_NOT_KEY_AGREEMENT);
        return NULL;
    }
    return ri->d.kari->recipientEncryptedKeys;
}

/** clang-extract: from crypto/cms/cms_kari.c:51:1 */
int CMS_RecipientInfo_kari_get0_orig_id(CMS_RecipientInfo *ri,
                                        X509_ALGOR **pubalg,
                                        ASN1_BIT_STRING **pubkey,
                                        ASN1_OCTET_STRING **keyid,
                                        X509_NAME **issuer,
                                        ASN1_INTEGER **sno)
{
    CMS_OriginatorIdentifierOrKey *oik;
    if (ri->type != CMS_RECIPINFO_AGREE) {
        CMSerr(CMS_F_CMS_RECIPIENTINFO_KARI_GET0_ORIG_ID,
               CMS_R_NOT_KEY_AGREEMENT);
        return 0;
    }
    oik = ri->d.kari->originator;
    if (issuer)
        *issuer = NULL;
    if (sno)
        *sno = NULL;
    if (keyid)
        *keyid = NULL;
    if (pubalg)
        *pubalg = NULL;
    if (pubkey)
        *pubkey = NULL;
    if (oik->type == CMS_OIK_ISSUER_SERIAL) {
        if (issuer)
            *issuer = oik->d.issuerAndSerialNumber->issuer;
        if (sno)
            *sno = oik->d.issuerAndSerialNumber->serialNumber;
    } else if (oik->type == CMS_OIK_KEYIDENTIFIER) {
        if (keyid)
            *keyid = oik->d.subjectKeyIdentifier;
    } else if (oik->type == CMS_OIK_PUBKEY) {
        if (pubalg)
            *pubalg = oik->d.originatorKey->algorithm;
        if (pubkey)
            *pubkey = oik->d.originatorKey->publicKey;
    } else
        return 0;
    return 1;
}

/** clang-extract: from crypto/cms/cms_kari.c:93:1 */
int CMS_RecipientInfo_kari_orig_id_cmp(CMS_RecipientInfo *ri, X509 *cert)
{
    CMS_OriginatorIdentifierOrKey *oik;
    if (ri->type != CMS_RECIPINFO_AGREE) {
        CMSerr(CMS_F_CMS_RECIPIENTINFO_KARI_ORIG_ID_CMP,
               CMS_R_NOT_KEY_AGREEMENT);
        return -2;
    }
    oik = ri->d.kari->originator;
    if (oik->type == CMS_OIK_ISSUER_SERIAL)
        return cms_ias_cert_cmp(oik->d.issuerAndSerialNumber, cert);
    else if (oik->type == CMS_OIK_KEYIDENTIFIER)
        return cms_keyid_cert_cmp(oik->d.subjectKeyIdentifier, cert);
    return -1;
}

/** clang-extract: from crypto/cms/cms_kari.c:109:1 */
int CMS_RecipientEncryptedKey_get0_id(CMS_RecipientEncryptedKey *rek,
                                      ASN1_OCTET_STRING **keyid,
                                      ASN1_GENERALIZEDTIME **tm,
                                      CMS_OtherKeyAttribute **other,
                                      X509_NAME **issuer, ASN1_INTEGER **sno)
{
    CMS_KeyAgreeRecipientIdentifier *rid = rek->rid;
    if (rid->type == CMS_REK_ISSUER_SERIAL) {
        if (issuer)
            *issuer = rid->d.issuerAndSerialNumber->issuer;
        if (sno)
            *sno = rid->d.issuerAndSerialNumber->serialNumber;
        if (keyid)
            *keyid = NULL;
        if (tm)
            *tm = NULL;
        if (other)
            *other = NULL;
    } else if (rid->type == CMS_REK_KEYIDENTIFIER) {
        if (keyid)
            *keyid = rid->d.rKeyId->subjectKeyIdentifier;
        if (tm)
            *tm = rid->d.rKeyId->date;
        if (other)
            *other = rid->d.rKeyId->other;
        if (issuer)
            *issuer = NULL;
        if (sno)
            *sno = NULL;
    } else
        return 0;
    return 1;
}

/** clang-extract: from crypto/cms/cms_kari.c:143:1 */
int CMS_RecipientEncryptedKey_cert_cmp(CMS_RecipientEncryptedKey *rek,
                                       X509 *cert)
{
    CMS_KeyAgreeRecipientIdentifier *rid = rek->rid;
    if (rid->type == CMS_REK_ISSUER_SERIAL)
        return cms_ias_cert_cmp(rid->d.issuerAndSerialNumber, cert);
    else if (rid->type == CMS_REK_KEYIDENTIFIER)
        return cms_keyid_cert_cmp(rid->d.rKeyId->subjectKeyIdentifier, cert);
    else
        return -1;
}

/** clang-extract: from crypto/cms/cms_kari.c:186:1 */
static int cms_kek_cipher(unsigned char **pout, size_t *poutlen,
                          const unsigned char *in, size_t inlen,
                          CMS_KeyAgreeRecipientInfo *kari, int enc)
{
    /* Key encryption key */
    unsigned char kek[EVP_MAX_KEY_LENGTH];
    size_t keklen;
    int rv = 0;
    unsigned char *out = NULL;
    int outlen;
    size_t outsize;
    keklen = EVP_CIPHER_CTX_key_length(kari->ctx);
    if (keklen > EVP_MAX_KEY_LENGTH)
        return 0;
    /* Derive KEK */
    if (EVP_PKEY_derive(kari->pctx, kek, &keklen) <= 0)
        goto err;
    /* Set KEK in context */
    if (!EVP_CipherInit_ex(kari->ctx, NULL, NULL, kek, NULL, enc))
        goto err;
    /* obtain output length of ciphered key */
    if (!EVP_CipherUpdate(kari->ctx, NULL, &outlen, in, inlen))
        goto err;
    /*
     * On its integrity-failure paths that primitive writes and cleanses up to
     * inlen bytes of the output buffer. Size the buffer for that worst case so
     * a failed unwrap cannot write past the allocation.
     */
    outsize = (size_t)outlen < inlen ? inlen : (size_t)outlen;
    out = OPENSSL_malloc(outsize);
    if (out == NULL)
        goto err;
    if (!EVP_CipherUpdate(kari->ctx, out, &outlen, in, inlen))
        goto err;
    *pout = out;
    *poutlen = (size_t)outlen;
    rv = 1;

 err:
    OPENSSL_cleanse(kek, keklen);
    if (!rv)
        OPENSSL_free(out);
    EVP_CIPHER_CTX_reset(kari->ctx);
    /* FIXME: WHY IS kari->pctx freed here?  /RL */
    EVP_PKEY_CTX_free(kari->pctx);
    kari->pctx = NULL;
    return rv;
}

/** clang-extract: from crypto/cms/cms_kari.c:235:1 */
int CMS_RecipientInfo_kari_decrypt(CMS_ContentInfo *cms,
                                   CMS_RecipientInfo *ri,
                                   CMS_RecipientEncryptedKey *rek)
{
    int rv = 0;
    unsigned char *enckey = NULL, *cek = NULL;
    size_t enckeylen;
    size_t ceklen;
    CMS_EncryptedContentInfo *ec;
    enckeylen = rek->encryptedKey->length;
    enckey = rek->encryptedKey->data;
    /* Setup all parameters to derive KEK */
    if (!cms_env_asn1_ctrl(ri, 1))
        goto err;
    /* Attempt to decrypt CEK */
    if (!cms_kek_cipher(&cek, &ceklen, enckey, enckeylen, ri->d.kari, 0))
        goto err;
    ec = cms->d.envelopedData->encryptedContentInfo;
    OPENSSL_clear_free(ec->key, ec->keylen);
    ec->key = cek;
    ec->keylen = ceklen;
    cek = NULL;
    rv = 1;
 err:
    OPENSSL_free(cek);
    return rv;
}

/** clang-extract: from crypto/cms/cms_kari.c:264:1 */
static int cms_kari_create_ephemeral_key(CMS_KeyAgreeRecipientInfo *kari,
                                         EVP_PKEY *pk)
{
    EVP_PKEY_CTX *pctx = NULL;
    EVP_PKEY *ekey = NULL;
    int rv = 0;
    pctx = EVP_PKEY_CTX_new(pk, NULL);
    if (!pctx)
        goto err;
    if (EVP_PKEY_keygen_init(pctx) <= 0)
        goto err;
    if (EVP_PKEY_keygen(pctx, &ekey) <= 0)
        goto err;
    EVP_PKEY_CTX_free(pctx);
    pctx = EVP_PKEY_CTX_new(ekey, NULL);
    if (!pctx)
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

/** clang-extract: from crypto/cms/cms_kari.c:294:1 */
int cms_RecipientInfo_kari_init(CMS_RecipientInfo *ri, X509 *recip,
                                EVP_PKEY *pk, unsigned int flags)
{
    CMS_KeyAgreeRecipientInfo *kari;
    CMS_RecipientEncryptedKey *rek = NULL;

    ri->d.kari = M_ASN1_new_of(CMS_KeyAgreeRecipientInfo);
    if (!ri->d.kari)
        return 0;
    ri->type = CMS_RECIPINFO_AGREE;

    kari = ri->d.kari;
    kari->version = 3;

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
        if (!cms_set1_keyid(&rek->rid->d.rKeyId->subjectKeyIdentifier, recip))
            return 0;
    } else {
        rek->rid->type = CMS_REK_ISSUER_SERIAL;
        if (!cms_set1_ias(&rek->rid->d.issuerAndSerialNumber, recip))
            return 0;
    }

    /* Create ephemeral key */
    if (!cms_kari_create_ephemeral_key(kari, pk))
        return 0;

    EVP_PKEY_up_ref(pk);
    rek->pkey = pk;
    return 1;
}

/** clang-extract: from crypto/cms/cms_kari.c:339:1 */
static int cms_wrap_init(CMS_KeyAgreeRecipientInfo *kari,
                         const EVP_CIPHER *cipher)
{
    EVP_CIPHER_CTX *ctx = kari->ctx;
    const EVP_CIPHER *kekcipher;
    int keylen = EVP_CIPHER_key_length(cipher);
    /* If a suitable wrap algorithm is already set nothing to do */
    kekcipher = EVP_CIPHER_CTX_cipher(ctx);

    if (kekcipher) {
        if (EVP_CIPHER_CTX_mode(ctx) != EVP_CIPH_WRAP_MODE)
            return 0;
        return 1;
    }
    /*
     * Pick a cipher based on content encryption cipher. If it is DES3 use
     * DES3 wrap otherwise use AES wrap similar to key size.
     */
#ifndef OPENSSL_NO_DES
    if (EVP_CIPHER_type(cipher) == NID_des_ede3_cbc)
        kekcipher = EVP_des_ede3_wrap();
    else
#endif
    if (keylen <= 16)
        kekcipher = EVP_aes_128_wrap();
    else if (keylen <= 24)
        kekcipher = EVP_aes_192_wrap();
    else
        kekcipher = EVP_aes_256_wrap();
    return EVP_EncryptInit_ex(ctx, kekcipher, NULL, NULL, NULL);
}

/** clang-extract: from crypto/cms/cms_kari.c:373:1 */
int cms_RecipientInfo_kari_encrypt(CMS_ContentInfo *cms,
                                   CMS_RecipientInfo *ri)
{
    CMS_KeyAgreeRecipientInfo *kari;
    CMS_EncryptedContentInfo *ec;
    CMS_RecipientEncryptedKey *rek;
    STACK_OF(CMS_RecipientEncryptedKey) *reks;
    int i;

    if (ri->type != CMS_RECIPINFO_AGREE) {
        CMSerr(CMS_F_CMS_RECIPIENTINFO_KARI_ENCRYPT, CMS_R_NOT_KEY_AGREEMENT);
        return 0;
    }
    kari = ri->d.kari;
    reks = kari->recipientEncryptedKeys;
    ec = cms->d.envelopedData->encryptedContentInfo;
    /* Initialise wrap algorithm parameters */
    if (!cms_wrap_init(kari, ec->cipher))
        return 0;
    /*
     * If no originator key set up initialise for ephemeral key the public key
     * ASN1 structure will set the actual public key value.
     */
    if (kari->originator->type == -1) {
        CMS_OriginatorIdentifierOrKey *oik = kari->originator;
        oik->type = CMS_OIK_PUBKEY;
        oik->d.originatorKey = M_ASN1_new_of(CMS_OriginatorPublicKey);
        if (!oik->d.originatorKey)
            return 0;
    }
    /* Initialise KDF algorithm */
    if (!cms_env_asn1_ctrl(ri, 0))
        return 0;
    /* For each rek, derive KEK, encrypt CEK */
    for (i = 0; i < sk_CMS_RecipientEncryptedKey_num(reks); i++) {
        unsigned char *enckey;
        size_t enckeylen;
        rek = sk_CMS_RecipientEncryptedKey_value(reks, i);
        if (EVP_PKEY_derive_set_peer(kari->pctx, rek->pkey) <= 0)
            return 0;
        if (!cms_kek_cipher(&enckey, &enckeylen, ec->key, ec->keylen,
                            kari, 1))
            return 0;
        ASN1_STRING_set0(rek->encryptedKey, enckey, enckeylen);
    }

    return 1;

}

/* The following functions must be output with their full definition.  */
/* { dg-final { scan-tree-dump "cms_RecipientInfo_kari_encrypt\(.*\n.*\)\n{" } } */
/* { dg-final { scan-tree-dump "CMS_RecipientInfo_kari_decrypt\(.*\n.*\n.*\)\n{" } } */
