/* { dg-options "-DCE_EXTRACT_FUNCTIONS=OSSL_CMP_exec_RR_ses -DCE_IPACLONES_PATH=$test_dir/libcrypto-lib-cmp_client.c.000i.ipa-clones -DCE_NO_EXTERNALIZATION" }*/
/* Check if there is an use-after-free.  */

#define __LEAF
#define __THROW	__attribute__ ((__nothrow__ __LEAF))
/** clang-extract: from /usr/lib64/clang/22/include/__stddef_size_t.h:18:1 */
typedef __SIZE_TYPE__ size_t;

/** clang-extract: from /usr/include/bits/types.h:44:1 */
typedef signed long int __int64_t;

/** clang-extract: from /usr/include/bits/types.h:45:1 */
typedef unsigned long int __uint64_t;

#define __SLONGWORD_TYPE	long int
#define __STD_TYPE		typedef
#define __SYSCALL_SLONG_TYPE	__SLONGWORD_TYPE
#define __TIME_T_TYPE		__SYSCALL_SLONG_TYPE
/** clang-extract: from crypto/cmp/cmp_client.c:157:1 */
__STD_TYPE __TIME_T_TYPE __time_t;

#undef __STD_TYPE
/** clang-extract: from /usr/include/bits/types/time_t.h:10:1 */
typedef __time_t time_t;

/** clang-extract: from /usr/include/bits/stdint-intn.h:27:1 */
typedef __int64_t int64_t;

#define INT_MAX   __INT_MAX__
#define ULONG_MAX (__LONG_MAX__ *2UL+1UL)
#define OPENSSL_FILE __FILE__
#define OPENSSL_LINE __LINE__
#define OPENSSL_FUNC __func__
/** clang-extract: from /usr/include/bits/stdint-uintn.h:27:1 */
typedef __uint64_t uint64_t;

#define ossl_unused __attribute__((unused))
/** clang-extract: from /usr/include/time.h:85:1 */
extern time_t time (time_t *__timer) __THROW;

/** clang-extract: from include/openssl/stack.h:23:1 */
typedef struct stack_st OPENSSL_STACK;

/** clang-extract: from include/openssl/stack.h:29:1 */
int OPENSSL_sk_num(const OPENSSL_STACK *);

/** clang-extract: from include/openssl/stack.h:30:1 */
void *OPENSSL_sk_value(const OPENSSL_STACK *, int);

/** clang-extract: from include/openssl/stack.h:35:1 */
OPENSSL_STACK *OPENSSL_sk_new_null(void);

/** clang-extract: from include/openssl/stack.h:38:1 */
void OPENSSL_sk_free(OPENSSL_STACK *);

/** clang-extract: from include/openssl/stack.h:49:1 */
int OPENSSL_sk_push(OPENSSL_STACK *st, const void *data);

/** clang-extract: from include/openssl/stack.h:51:1 */
void *OPENSSL_sk_shift(OPENSSL_STACK *st);

#define STACK_OF(type) struct stack_st_##type
/** clang-extract: from include/openssl/types.h:63:1 */
typedef struct asn1_string_st ASN1_INTEGER;

/** clang-extract: from include/openssl/types.h:65:1 */
typedef struct asn1_string_st ASN1_BIT_STRING;

/** clang-extract: from include/openssl/types.h:66:1 */
typedef struct asn1_string_st ASN1_OCTET_STRING;

/** clang-extract: from include/openssl/types.h:75:1 */
typedef struct asn1_string_st ASN1_GENERALIZEDTIME;

/** clang-extract: from include/openssl/types.h:77:1 */
typedef struct asn1_string_st ASN1_UTF8STRING;

/** clang-extract: from include/openssl/types.h:78:1 */
typedef struct asn1_string_st ASN1_STRING;

/** clang-extract: from include/openssl/types.h:83:1 */
typedef struct asn1_type_st ASN1_TYPE;

/** clang-extract: from include/openssl/types.h:95:1 */
typedef struct bio_st BIO;

/** clang-extract: from include/openssl/types.h:112:1 */
typedef struct evp_md_st EVP_MD;

/** clang-extract: from include/openssl/types.h:116:1 */
typedef struct evp_pkey_st EVP_PKEY;

/** clang-extract: from include/openssl/types.h:169:1 */
typedef struct x509_st X509;

/** clang-extract: from include/openssl/types.h:174:1 */
typedef struct X509_name_st X509_NAME;

/** clang-extract: from include/openssl/types.h:176:1 */
typedef struct x509_store_st X509_STORE;

/** clang-extract: from include/openssl/types.h:177:1 */
typedef struct x509_store_ctx_st X509_STORE_CTX;

/** clang-extract: from include/openssl/types.h:182:1 */
typedef struct X509_VERIFY_PARAM_st X509_VERIFY_PARAM;

/** clang-extract: from include/openssl/types.h:214:1 */
typedef struct ossl_http_req_ctx_st OSSL_HTTP_REQ_CTX;

/** clang-extract: from include/openssl/types.h:227:1 */
typedef struct ossl_lib_ctx_st OSSL_LIB_CTX;

#define OPENSSL_free(addr) \
        CRYPTO_free(addr, OPENSSL_FILE, OPENSSL_LINE)
/** clang-extract: from include/openssl/crypto.h:364:1 */
void CRYPTO_free(void *ptr, const char *file, int line);

/** clang-extract: from include/openssl/crypto.h:575:1 */
void OSSL_sleep(uint64_t millis);

#define NULL ((void*)0)
#define ossl_likely(x)     __builtin_expect(!!(x), 1)
#define ossl_assert(x) ossl_likely((x) != 0)
#define ossl_bio__attr__ __attribute__
#define ossl_bio__printf__ __printf__
/** clang-extract: from include/openssl/bio.h:961:1 */
int BIO_snprintf(char *buf, size_t n, const char *format, ...)
ossl_bio__attr__((__format__(ossl_bio__printf__, 3, 4)));

#undef ossl_bio__attr__
#undef ossl_bio__printf__
/** clang-extract: from include/openssl/asn1.h:669:1 */
extern ASN1_STRING *ASN1_STRING_dup(const ASN1_STRING *a);

/** clang-extract: from include/openssl/asn1.h:701:1 */
struct stack_st_ASN1_INTEGER;/* Full definition was removed.  */

/** clang-extract: from include/openssl/asn1.h:734:1 */
int ASN1_INTEGER_cmp(const ASN1_INTEGER *x, const ASN1_INTEGER *y);

/** clang-extract: from include/openssl/asn1.h:763:1 */
struct stack_st_ASN1_UTF8STRING;/* Full definition was removed.  */

/** clang-extract: from include/openssl/asn1.h:763:1 */
__attribute__((unused)) static inline ASN1_UTF8STRING *ossl_check_ASN1_UTF8STRING_type(ASN1_UTF8STRING *ptr) {
    return ptr;
}

/** clang-extract: from include/openssl/asn1.h:763:1 */
__attribute__((unused)) static inline const OPENSSL_STACK *ossl_check_const_ASN1_UTF8STRING_sk_type(const struct stack_st_ASN1_UTF8STRING *sk) {
    return (const OPENSSL_STACK *)sk;
}

/** clang-extract: from include/openssl/asn1.h:763:1 */
__attribute__((unused)) static inline OPENSSL_STACK *ossl_check_ASN1_UTF8STRING_sk_type(struct stack_st_ASN1_UTF8STRING *sk) {
    return (OPENSSL_STACK *)sk;
}

#define sk_ASN1_UTF8STRING_num(sk) OPENSSL_sk_num(ossl_check_const_ASN1_UTF8STRING_sk_type(sk))
#define sk_ASN1_UTF8STRING_value(sk, idx) ((ASN1_UTF8STRING *)OPENSSL_sk_value(ossl_check_const_ASN1_UTF8STRING_sk_type(sk), (idx)))
#define sk_ASN1_UTF8STRING_new_null() ((STACK_OF(ASN1_UTF8STRING) *)OPENSSL_sk_new_null())
#define sk_ASN1_UTF8STRING_push(sk, ptr) OPENSSL_sk_push(ossl_check_ASN1_UTF8STRING_sk_type(sk), ossl_check_ASN1_UTF8STRING_type(ptr))
/** clang-extract: from include/openssl/asn1.h:793:1 */
extern void ASN1_UTF8STRING_free(ASN1_UTF8STRING *a);

/** clang-extract: from include/openssl/asn1.h:872:1 */
int ASN1_INTEGER_get_int64(int64_t *pr, const ASN1_INTEGER *a);

/** clang-extract: from include/openssl/asn1.h:878:1 */
long ASN1_INTEGER_get(const ASN1_INTEGER *a);

#define ERR_LIB_CMP             58
/** clang-extract: from include/openssl/err.h:395:1 */
void ERR_new(void);

/** clang-extract: from include/openssl/err.h:396:1 */
void ERR_set_debug(const char *file, int line, const char *func);

/** clang-extract: from include/openssl/err.h:397:1 */
void ERR_set_error(int lib, int reason, const char *fmt, ...);

#define ERR_raise(lib, reason) ERR_raise_data((lib),(reason),NULL)
#define ERR_raise_data                                         \
    (ERR_new(),                                                 \
     ERR_set_debug(OPENSSL_FILE,OPENSSL_LINE,OPENSSL_FUNC),     \
     ERR_set_error)
/** clang-extract: from include/openssl/err.h:470:1 */
void ERR_add_error_data(int num, ...);

/** clang-extract: from include/internal/cryptlib.h:159:1 */
char *ossl_sk_ASN1_UTF8STRING2text(STACK_OF(ASN1_UTF8STRING) *text,
                                   const char *sep, size_t max_len);

/** clang-extract: from include/openssl/x509.h:78:1 */
struct stack_st_X509;/* Full definition was removed.  */

/** clang-extract: from include/openssl/x509.h:78:1 */
__attribute__((unused)) static inline const OPENSSL_STACK *ossl_check_const_X509_sk_type(const struct stack_st_X509 *sk) {
    return (const OPENSSL_STACK *)sk;
}

/** clang-extract: from include/openssl/x509.h:78:1 */
__attribute__((unused)) static inline OPENSSL_STACK *ossl_check_X509_sk_type(struct stack_st_X509 *sk) {
    return (OPENSSL_STACK *)sk;
}

#define sk_X509_num(sk) OPENSSL_sk_num(ossl_check_const_X509_sk_type(sk))
#define sk_X509_free(sk) OPENSSL_sk_free(ossl_check_X509_sk_type(sk))
#define sk_X509_shift(sk) ((X509 *)OPENSSL_sk_shift(ossl_check_X509_sk_type(sk)))
/** clang-extract: from include/openssl/x509.h:130:1 */
struct stack_st_X509_CRL;/* Full definition was removed.  */

/** clang-extract: from include/openssl/x509.h:130:1 */
__attribute__((unused)) static inline const OPENSSL_STACK *ossl_check_const_X509_CRL_sk_type(const struct stack_st_X509_CRL *sk) {
    return (const OPENSSL_STACK *)sk;
}

#define sk_X509_CRL_num(sk) OPENSSL_sk_num(ossl_check_const_X509_CRL_sk_type(sk))
/** clang-extract: from include/openssl/x509.h:234:1 */
struct stack_st_X509_EXTENSION;/* Full definition was removed.  */

/** clang-extract: from include/openssl/x509.h:261:1 */
typedef STACK_OF(X509_EXTENSION) X509_EXTENSIONS;

/** clang-extract: from include/openssl/x509.h:291:1 */
typedef struct X509_req_st X509_REQ;

/** clang-extract: from include/openssl/x509_vfy.h:237:1 */
int X509_verify_cert(X509_STORE_CTX *ctx);

/** clang-extract: from include/openssl/x509_vfy.h:239:1 */
STACK_OF(X509) *X509_build_chain(X509 *target, STACK_OF(X509) *certs,
                                 X509_STORE *store, int with_self_signed,
                                 OSSL_LIB_CTX *libctx, const char *propq);

#define X509_V_FLAG_USE_CHECK_TIME              0x2
#define X509_V_FLAG_POLICY_CHECK                0x80
#define X509_V_FLAG_PARTIAL_CHAIN               0x80000
#define X509_V_FLAG_NO_CHECK_TIME               0x200000
/** clang-extract: from include/openssl/x509_vfy.h:562:1 */
X509_STORE_CTX *X509_STORE_CTX_new_ex(OSSL_LIB_CTX *libctx, const char *propq);

/** clang-extract: from include/openssl/x509_vfy.h:567:1 */
void X509_STORE_CTX_free(X509_STORE_CTX *ctx);

/** clang-extract: from include/openssl/x509_vfy.h:568:1 */
int X509_STORE_CTX_init(X509_STORE_CTX *ctx, X509_STORE *trust_store,
                        X509 *target, STACK_OF(X509) *untrusted);

/** clang-extract: from include/openssl/x509_vfy.h:769:1 */
STACK_OF(X509) *X509_STORE_CTX_get0_chain(const X509_STORE_CTX *ctx);

/** clang-extract: from include/openssl/x509_vfy.h:789:1 */
X509_VERIFY_PARAM *X509_STORE_CTX_get0_param(const X509_STORE_CTX *ctx);

/** clang-extract: from include/openssl/x509_vfy.h:811:1 */
int X509_VERIFY_PARAM_clear_flags(X509_VERIFY_PARAM *param,
                                  unsigned long flags);

/** clang-extract: from include/openssl/http.h:75:1 */
typedef BIO *(*OSSL_HTTP_bio_cb_t)(BIO *bio, void *arg, int connect, int detail);

/** clang-extract: from include/openssl/x509.h:766:1 */
extern void X509_free(X509 *a);

/** clang-extract: from include/openssl/x509.h:819:1 */
char *X509_NAME_oneline(const X509_NAME *a, char *buf, int size);

/** clang-extract: from include/openssl/x509.h:860:1 */
X509_NAME *X509_get_subject_name(const X509 *a);

/** clang-extract: from include/openssl/x509.h:975:1 */
int X509_check_private_key(const X509 *cert, const EVP_PKEY *pkey);

/** clang-extract: from include/openssl/x509.h:980:1 */
void OSSL_STACK_OF_X509_free(STACK_OF(X509) *certs);

#define X509_ADD_FLAG_UP_REF   0x1
#define X509_ADD_FLAG_NO_DUP   0x4
#define X509_ADD_FLAG_NO_SS    0x8
/** clang-extract: from include/openssl/x509.h:1006:1 */
int X509_NAME_cmp(const X509_NAME *a, const X509_NAME *b);

/** clang-extract: from include/openssl/x509v3.h:237:1 */
struct stack_st_GENERAL_NAME;/* Full definition was removed.  */

/** clang-extract: from include/openssl/x509v3.h:467:1 */
struct stack_st_POLICYINFO;/* Full definition was removed.  */

/** clang-extract: from include/openssl/x509v3.h:495:1 */
typedef STACK_OF(POLICYINFO) CERTIFICATEPOLICIES;

/** clang-extract: from include/openssl/crmf.h:50:1 */
typedef struct ossl_crmf_encryptedkey_st OSSL_CRMF_ENCRYPTEDKEY;

/** clang-extract: from include/openssl/crmf.h:53:1 */
typedef struct ossl_crmf_msg_st OSSL_CRMF_MSG;

/** clang-extract: from include/openssl/crmf.h:56:1 */
struct stack_st_OSSL_CRMF_MSG;/* Full definition was removed.  */

/** clang-extract: from include/openssl/crmf.h:118:1 */
typedef struct ossl_crmf_certid_st OSSL_CRMF_CERTID;

/** clang-extract: from include/openssl/crmf.h:121:1 */
struct stack_st_OSSL_CRMF_CERTID;/* Full definition was removed.  */

/** clang-extract: from include/openssl/crmf.h:121:1 */
__attribute__((unused)) static inline const OPENSSL_STACK *ossl_check_const_OSSL_CRMF_CERTID_sk_type(const struct stack_st_OSSL_CRMF_CERTID *sk) {
    return (const OPENSSL_STACK *)sk;
}

#define sk_OSSL_CRMF_CERTID_num(sk) OPENSSL_sk_num(ossl_check_const_OSSL_CRMF_CERTID_sk_type(sk))
/** clang-extract: from include/openssl/crmf.h:149:1 */
typedef struct ossl_crmf_pkipublicationinfo_st OSSL_CRMF_PKIPUBLICATIONINFO;

/** clang-extract: from include/openssl/crmf.h:153:1 */
typedef struct ossl_crmf_certtemplate_st OSSL_CRMF_CERTTEMPLATE;

/** clang-extract: from include/openssl/crmf.h:156:1 */
typedef STACK_OF(OSSL_CRMF_MSG) OSSL_CRMF_MSGS;

/** clang-extract: from include/openssl/crmf.h:240:1 */
const X509_NAME
*OSSL_CRMF_CERTTEMPLATE_get0_issuer(const OSSL_CRMF_CERTTEMPLATE *tmpl);

/** clang-extract: from include/openssl/crmf.h:242:1 */
const ASN1_INTEGER
*OSSL_CRMF_CERTTEMPLATE_get0_serialNumber(const OSSL_CRMF_CERTTEMPLATE *tmpl);

/** clang-extract: from include/openssl/crmf.h:246:1 */
const X509_NAME
*OSSL_CRMF_CERTID_get0_issuer(const OSSL_CRMF_CERTID *cid);

/** clang-extract: from include/openssl/crmf.h:248:1 */
const ASN1_INTEGER
*OSSL_CRMF_CERTID_get0_serialNumber(const OSSL_CRMF_CERTID *cid);

#define CMP_R_BAD_CHECKAFTER_IN_POLLREP                  167
#define CMP_R_BAD_REQUEST_ID                             108
#define CMP_R_CERTIFICATE_NOT_ACCEPTED                   169
#define CMP_R_CERTIFICATE_NOT_FOUND                      112
#define CMP_R_CHECKAFTER_OUT_OF_RANGE                    181
#define CMP_R_ENCOUNTERED_KEYUPDATEWARNING               176
#define CMP_R_ENCOUNTERED_WAITING                        162
#define CMP_R_INVALID_ARGS                               100
#define CMP_R_MISSING_CERTID                             165
#define CMP_R_MISSING_REFERENCE_CERT                     168
#define CMP_R_MULTIPLE_RESPONSES_NOT_SUPPORTED           170
#define CMP_R_NULL_ARGUMENT                              103
#define CMP_R_POLLING_FAILED                             172
#define CMP_R_RECEIVED_ERROR                             180
#define CMP_R_REQUEST_REJECTED_BY_SERVER                 182
#define CMP_R_TOTAL_TIMEOUT                              184
#define CMP_R_TRANSFER_ERROR                             159
#define CMP_R_UNEXPECTED_PKIBODY                         133
#define CMP_R_UNEXPECTED_PKISTATUS                       185
#define CMP_R_UNKNOWN_PKISTATUS                          186
#define CMP_R_WRONG_CERTID_IN_RP                         187
#define CMP_R_WRONG_RP_COMPONENT_COUNT                   188
#define CMP_R_WRONG_SERIAL_IN_RP                         173
/** clang-extract: from include/openssl/cmp_util.h:33:1 */
typedef int OSSL_CMP_severity;

#define OSSL_CMP_LOG_ERR     3
#define OSSL_CMP_LOG_WARNING 4
#define OSSL_CMP_LOG_INFO    6
#define OSSL_CMP_LOG_DEBUG   7
/** clang-extract: from include/openssl/cmp_util.h:44:1 */
typedef int (*OSSL_CMP_log_cb_t)(const char *func, const char *file, int line,
                                 OSSL_CMP_severity level, const char *msg);

#define OSSL_CMP_PKIFAILUREINFO_incorrectData 7
/** clang-extract: from include/openssl/cmp.h:142:1 */
typedef ASN1_BIT_STRING OSSL_CMP_PKIFAILUREINFO;

#define OSSL_CMP_CTX_FAILINFO_badRequest (1 << 2)
#define OSSL_CMP_PKISTATUS_request                -3
#define OSSL_CMP_PKISTATUS_trans                  -2
#define OSSL_CMP_PKISTATUS_accepted               0
#define OSSL_CMP_PKISTATUS_grantedWithMods        1
#define OSSL_CMP_PKISTATUS_rejection              2
#define OSSL_CMP_PKISTATUS_waiting                3
#define OSSL_CMP_PKISTATUS_revocationWarning      4
#define OSSL_CMP_PKISTATUS_revocationNotification 5
#define OSSL_CMP_PKISTATUS_keyUpdateWarning       6
/** clang-extract: from include/openssl/cmp.h:207:1 */
typedef ASN1_INTEGER OSSL_CMP_PKISTATUS;

/** clang-extract: from include/openssl/cmp.h:215:1 */
typedef struct ossl_cmp_ctx_st OSSL_CMP_CTX;

/** clang-extract: from include/openssl/cmp.h:216:1 */
typedef struct ossl_cmp_pkiheader_st OSSL_CMP_PKIHEADER;

/** clang-extract: from include/openssl/cmp.h:218:1 */
typedef struct ossl_cmp_msg_st OSSL_CMP_MSG;

/** clang-extract: from include/openssl/cmp.h:222:1 */
struct stack_st_OSSL_CMP_CERTSTATUS;/* Full definition was removed.  */

/** clang-extract: from include/openssl/cmp.h:251:1 */
struct stack_st_OSSL_CMP_ITAV;/* Full definition was removed.  */

/** clang-extract: from include/openssl/cmp.h:318:1 */
typedef struct ossl_cmp_revrepcontent_st OSSL_CMP_REVREPCONTENT;

/** clang-extract: from include/openssl/cmp.h:319:1 */
typedef struct ossl_cmp_pkisi_st OSSL_CMP_PKISI;

/** clang-extract: from include/openssl/cmp.h:320:1 */
extern void OSSL_CMP_PKISI_free(OSSL_CMP_PKISI *a);

/** clang-extract: from include/openssl/cmp.h:322:1 */
struct stack_st_OSSL_CMP_PKISI;/* Full definition was removed.  */

/** clang-extract: from include/openssl/cmp.h:322:1 */
__attribute__((unused)) static inline const OPENSSL_STACK *ossl_check_const_OSSL_CMP_PKISI_sk_type(const struct stack_st_OSSL_CMP_PKISI *sk) {
    return (const OPENSSL_STACK *)sk;
}

#define sk_OSSL_CMP_PKISI_num(sk) OPENSSL_sk_num(ossl_check_const_OSSL_CMP_PKISI_sk_type(sk))
/** clang-extract: from include/openssl/cmp.h:349:1 */
typedef struct ossl_cmp_certrepmessage_st OSSL_CMP_CERTREPMESSAGE;

/** clang-extract: from include/openssl/cmp.h:377:1 */
typedef struct ossl_cmp_pollrep_st OSSL_CMP_POLLREP;

/** clang-extract: from include/openssl/cmp.h:378:1 */
typedef STACK_OF(OSSL_CMP_POLLREP) OSSL_CMP_POLLREPCONTENT;

/** clang-extract: from include/openssl/cmp.h:379:1 */
typedef struct ossl_cmp_certresponse_st OSSL_CMP_CERTRESPONSE;

/** clang-extract: from include/openssl/cmp.h:380:1 */
struct stack_st_OSSL_CMP_CERTRESPONSE;/* Full definition was removed.  */

/** clang-extract: from include/openssl/cmp.h:380:1 */
__attribute__((unused)) static inline const OPENSSL_STACK *ossl_check_const_OSSL_CMP_CERTRESPONSE_sk_type(const struct stack_st_OSSL_CMP_CERTRESPONSE *sk) {
    return (const OPENSSL_STACK *)sk;
}

#define sk_OSSL_CMP_CERTRESPONSE_num(sk) OPENSSL_sk_num(ossl_check_const_OSSL_CMP_CERTRESPONSE_sk_type(sk))
/** clang-extract: from include/openssl/cmp.h:407:1 */
typedef STACK_OF(ASN1_UTF8STRING) OSSL_CMP_PKIFREETEXT;

/** clang-extract: from include/openssl/cmp.h:473:1 */
void OSSL_CMP_MSG_free(OSSL_CMP_MSG *msg);

/** clang-extract: from include/openssl/cmp.h:512:1 */
void OSSL_CMP_CTX_print_errors(const OSSL_CMP_CTX *ctx);

/** clang-extract: from include/openssl/cmp.h:524:1 */
typedef OSSL_CMP_MSG *(*OSSL_CMP_transfer_cb_t) (OSSL_CMP_CTX *ctx,
                                                 const OSSL_CMP_MSG *req);

/** clang-extract: from include/openssl/cmp.h:571:1 */
typedef int (*OSSL_CMP_certConf_cb_t) (OSSL_CMP_CTX *ctx, X509 *cert,
                                       int fail_info, const char **txt);

/** clang-extract: from include/openssl/cmp.h:573:1 */
int OSSL_CMP_certConf_cb(OSSL_CMP_CTX *ctx, X509 *cert, int fail_info,
                         const char **text);

/** clang-extract: from include/openssl/cmp.h:577:1 */
void *OSSL_CMP_CTX_get_certConf_cb_arg(const OSSL_CMP_CTX *ctx);

#define OSSL_CMP_PKISI_BUFLEN 1024
/** clang-extract: from include/openssl/cmp.h:587:1 */
STACK_OF(X509) *OSSL_CMP_CTX_get1_extraCertsIn(const OSSL_CMP_CTX *ctx);

/** clang-extract: from include/openssl/cmp.h:594:1 */
char *OSSL_CMP_CTX_snprint_PKIStatus(const OSSL_CMP_CTX *ctx, char *buf,
                                     size_t bufsize);

/** clang-extract: from include/openssl/cmp.h:598:1 */
OSSL_CMP_PKISI *
OSSL_CMP_STATUSINFO_new(int status, int fail_info, const char *text);

/** clang-extract: from include/openssl/cmp.h:610:1 */
int OSSL_CMP_MSG_get_bodytype(const OSSL_CMP_MSG *msg);

/** clang-extract: from include/openssl/cmp.h:628:1 */
OSSL_CMP_MSG *OSSL_CMP_MSG_http_perform(OSSL_CMP_CTX *ctx,
                                        const OSSL_CMP_MSG *req);

/** clang-extract: from include/openssl/cmp.h:690:1 */
X509 *OSSL_CMP_exec_certreq(OSSL_CMP_CTX *ctx, int req_type,
                            const OSSL_CRMF_MSG *crm);

/** clang-extract: from include/openssl/cmp.h:708:1 */
int OSSL_CMP_exec_RR_ses(OSSL_CMP_CTX *ctx);

/** clang-extract: from include/crypto/x509.h:327:1 */
int ossl_x509_add_certs_new(STACK_OF(X509) **p_sk, STACK_OF(X509) *certs,
                            int flags);

/** clang-extract: from crypto/cmp/cmp_local.h:33:1 */
struct ossl_cmp_ctx_st {
    OSSL_LIB_CTX *libctx;
    char *propq;
    OSSL_CMP_log_cb_t log_cb; /* log callback for error/debug/etc. output */
    OSSL_CMP_severity log_verbosity; /* level of verbosity of log output */

    /* message transfer */
    OSSL_CMP_transfer_cb_t transfer_cb; /* default: OSSL_CMP_MSG_http_perform */
    void *transfer_cb_arg; /* allows to store optional argument to cb */
    /* HTTP-based transfer */
    OSSL_HTTP_REQ_CTX *http_ctx;
    char *serverPath;
    char *server;
    int serverPort;
    char *proxy;
    char *no_proxy;
    int keep_alive; /* persistent connection: 0=no, 1=prefer, 2=require */
    int msg_timeout; /* max seconds to wait for each CMP message round trip */
    int total_timeout; /* max number of seconds an enrollment may take, incl. */
    int tls_used; /* whether to use TLS for client-side HTTP connections */
    /* attempts polling for a response if a 'waiting' PKIStatus is received */
    time_t end_time; /* session start time + totaltimeout */
# ifndef OPENSSL_NO_HTTP
    OSSL_HTTP_bio_cb_t http_cb;
    void *http_cb_arg; /* allows to store optional argument to cb */
# endif

    /* server authentication */
    /*
     * unprotectedErrors may be set as workaround for broken server responses:
     * accept missing or invalid protection of regular error messages, negative
     * certificate responses (ip/cp/kup), revocation responses (rp), and PKIConf
     */
    int unprotectedErrors;
    int noCacheExtraCerts;
    X509 *srvCert; /* certificate used to identify the server */
    X509 *validatedSrvCert; /* caches any already validated server cert */
    X509_NAME *expected_sender; /* expected sender in header of response */
    X509_STORE *trusted; /* trust store maybe w CRLs and cert verify callback */
    STACK_OF(X509) *untrusted; /* untrusted (intermediate CA) certs */
    int ignore_keyusage; /* ignore key usage entry when validating certs */
    /*
     * permitTAInExtraCertsForIR allows use of root certs in extracerts
     * when validating message protection; this is used for 3GPP-style E.7
     */
    int permitTAInExtraCertsForIR;

    /* client authentication */
    int unprotectedSend; /* send unprotected PKI messages */
    X509 *cert; /* protection cert used to identify and sign for MSG_SIG_ALG */
    STACK_OF(X509) *chain; /* (cached) chain of protection cert including it */
    EVP_PKEY *pkey; /* the key pair corresponding to cert */
    ASN1_OCTET_STRING *referenceValue; /* optional username for MSG_MAC_ALG */
    ASN1_OCTET_STRING *secretValue; /* password/shared secret for MSG_MAC_ALG */
    /* PBMParameters for MSG_MAC_ALG */
    size_t pbm_slen; /* salt length, currently fixed to 16 */
    EVP_MD *pbm_owf; /* one-way function (OWF), default: SHA256 */
    int pbm_itercnt; /* OWF iteration count, currently fixed to 500 */
    int pbm_mac; /* NID of MAC algorithm, default: HMAC-SHA1 as per RFC 4210 */

    /* CMP message header and extra certificates */
    X509_NAME *recipient; /* to set in recipient in pkiheader */
    EVP_MD *digest; /* digest used in MSG_SIG_ALG and POPO, default SHA256 */
    ASN1_OCTET_STRING *transactionID; /* the current transaction ID */
    ASN1_OCTET_STRING *senderNonce; /* last nonce sent */
    ASN1_OCTET_STRING *recipNonce; /* last nonce received */
    ASN1_OCTET_STRING *first_senderNonce; /* sender nonce when starting to poll */
    ASN1_UTF8STRING *freeText; /* optional string to include each msg */
    STACK_OF(OSSL_CMP_ITAV) *geninfo_ITAVs;
    int implicitConfirm; /* set implicitConfirm in IR/KUR/CR messages */
    int disableConfirm; /* disable certConf in IR/KUR/CR for broken servers */
    STACK_OF(X509) *extraCertsOut; /* to be included in request messages */

    /* certificate template */
    EVP_PKEY *newPkey; /* explicit new private/public key for cert enrollment */
    int newPkey_priv; /* flag indicating if newPkey contains private key */
    X509_NAME *issuer; /* issuer name to used in cert template, also in rr */
    ASN1_INTEGER *serialNumber; /* certificate serial number to use in rr */
    int days; /* Number of days new certificates are asked to be valid for */
    X509_NAME *subjectName; /* subject name to be used in cert template */
    STACK_OF(GENERAL_NAME) *subjectAltNames; /* to add to the cert template */
    int SubjectAltName_nodefault;
    int setSubjectAltNameCritical;
    X509_EXTENSIONS *reqExtensions; /* exts to be added to cert template */
    CERTIFICATEPOLICIES *policies; /* policies to be included in extensions */
    int setPoliciesCritical;
    int popoMethod; /* Proof-of-possession mechanism; default: signature */
    X509 *oldCert; /* cert to be updated (via KUR) or to be revoked (via RR) */
    X509_REQ *p10CSR; /* for P10CR: PKCS#10 CSR to be sent */

    /* misc body contents */
    int revocationReason; /* revocation reason code to be included in RR */
    STACK_OF(OSSL_CMP_ITAV) *genm_ITAVs; /* content of general message */

    /* result returned in responses, so far supporting only one certResponse */
    int status; /* PKIStatus of last received IP/CP/KUP/RP/error or -1 */
    OSSL_CMP_PKIFREETEXT *statusString; /* of last IP/CP/KUP/RP/error */
    int failInfoCode; /* failInfoCode of last received IP/CP/KUP/error, or -1 */
    X509 *newCert; /* newly enrolled cert received from the CA */
    STACK_OF(X509) *newChain; /* chain of newly enrolled cert received */
    STACK_OF(X509) *caPubs; /* CA certs received from server (in IP message) */
    STACK_OF(X509) *extraCertsIn; /* extraCerts received from server */

    /* certificate confirmation */
    OSSL_CMP_certConf_cb_t certConf_cb; /* callback for app checking new cert */
    void *certConf_cb_arg; /* allows to store an argument individual to cb */
};

/** clang-extract: from crypto/cmp/cmp_local.h:157:1 */
typedef struct ossl_cmp_revanncontent_st {
    ASN1_INTEGER *status;
    OSSL_CRMF_CERTID *certId;
    ASN1_GENERALIZEDTIME *willBeRevokedAt;
    ASN1_GENERALIZEDTIME *badSinceDate;
    X509_EXTENSIONS *crlDetails;
} OSSL_CMP_REVANNCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:204:1 */
typedef struct ossl_cmp_cakeyupdanncontent_st {
    X509 *oldWithNew;
    X509 *newWithOld;
    X509 *newWithNew;
} OSSL_CMP_CAKEYUPDANNCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:248:1 */
typedef STACK_OF(OSSL_CMP_MSG) OSSL_CMP_MSGS;

/** clang-extract: from crypto/cmp/cmp_local.h:310:1 */
typedef struct ossl_cmp_certorenccert_st {
    int type;
    union {
        X509 *certificate;
        OSSL_CRMF_ENCRYPTEDKEY *encryptedCert;
    } value;
} OSSL_CMP_CERTORENCCERT;

/** clang-extract: from crypto/cmp/cmp_local.h:327:1 */
typedef struct ossl_cmp_certifiedkeypair_st {
    OSSL_CMP_CERTORENCCERT *certOrEncCert;
    OSSL_CRMF_ENCRYPTEDKEY *privateKey;
    OSSL_CRMF_PKIPUBLICATIONINFO *publicationInfo;
} OSSL_CMP_CERTIFIEDKEYPAIR;

/** clang-extract: from crypto/cmp/cmp_local.h:341:1 */
struct ossl_cmp_pkisi_st {
    OSSL_CMP_PKISTATUS *status;
    OSSL_CMP_PKIFREETEXT *statusString;
    OSSL_CMP_PKIFAILUREINFO *failInfo;
};

/** clang-extract: from crypto/cmp/cmp_local.h:355:1 */
struct ossl_cmp_revdetails_st {
    OSSL_CRMF_CERTTEMPLATE *certDetails;
    X509_EXTENSIONS *crlEntryDetails;
};

/** clang-extract: from crypto/cmp/cmp_local.h:359:1 */
typedef struct ossl_cmp_revdetails_st OSSL_CMP_REVDETAILS;

/** clang-extract: from crypto/cmp/cmp_local.h:361:1 */
struct stack_st_OSSL_CMP_REVDETAILS;/* Full definition was removed.  */

/** clang-extract: from crypto/cmp/cmp_local.h:361:1 */
__attribute__((unused)) static inline OSSL_CMP_REVDETAILS *sk_OSSL_CMP_REVDETAILS_value(const struct stack_st_OSSL_CMP_REVDETAILS *sk, int idx) {
    return (OSSL_CMP_REVDETAILS *)OPENSSL_sk_value((const OPENSSL_STACK *)sk, idx);
}

/** clang-extract: from crypto/cmp/cmp_local.h:376:1 */
struct ossl_cmp_revrepcontent_st {
    STACK_OF(OSSL_CMP_PKISI) *status;
    STACK_OF(OSSL_CRMF_CERTID) *revCerts;
    STACK_OF(X509_CRL) *crls;
};

/** clang-extract: from crypto/cmp/cmp_local.h:393:1 */
typedef struct ossl_cmp_keyrecrepcontent_st {
    OSSL_CMP_PKISI *status;
    X509 *newSigCert;
    STACK_OF(X509) *caCerts;
    STACK_OF(OSSL_CMP_CERTIFIEDKEYPAIR) *keyPairHist;
} OSSL_CMP_KEYRECREPCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:410:1 */
typedef struct ossl_cmp_errormsgcontent_st {
    OSSL_CMP_PKISI *pKIStatusInfo;
    ASN1_INTEGER *errorCode;
    OSSL_CMP_PKIFREETEXT *errorDetails;
} OSSL_CMP_ERRORMSGCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:437:1 */
typedef STACK_OF(OSSL_CMP_CERTSTATUS) OSSL_CMP_CERTCONFIRMCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:453:1 */
struct ossl_cmp_certresponse_st {
    ASN1_INTEGER *certReqId;
    OSSL_CMP_PKISI *status;
    OSSL_CMP_CERTIFIEDKEYPAIR *certifiedKeyPair;
    ASN1_OCTET_STRING *rspInfo;
};

/** clang-extract: from crypto/cmp/cmp_local.h:468:1 */
struct ossl_cmp_certrepmessage_st {
    STACK_OF(X509) *caPubs;
    STACK_OF(OSSL_CMP_CERTRESPONSE) *response;
};

/** clang-extract: from crypto/cmp/cmp_local.h:483:1 */
struct stack_st_OSSL_CMP_POLLREQ;/* Full definition was removed.  */

/** clang-extract: from crypto/cmp/cmp_local.h:484:1 */
typedef STACK_OF(OSSL_CMP_POLLREQ) OSSL_CMP_POLLREQCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:494:1 */
struct ossl_cmp_pollrep_st {
    ASN1_INTEGER *certReqId;
    ASN1_INTEGER *checkAfter;
    OSSL_CMP_PKIFREETEXT *reason;
};

/** clang-extract: from crypto/cmp/cmp_local.h:500:1 */
__attribute__((unused)) static inline int sk_OSSL_CMP_POLLREP_num(const struct stack_st_OSSL_CMP_POLLREP *sk) {
    return OPENSSL_sk_num((const OPENSSL_STACK *)sk);
}

/** clang-extract: from crypto/cmp/cmp_local.h:553:1 */
typedef STACK_OF(OSSL_CMP_CHALLENGE) OSSL_CMP_POPODECKEYCHALLCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:555:1 */
typedef STACK_OF(ASN1_INTEGER) OSSL_CMP_POPODECKEYRESPCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:557:1 */
typedef STACK_OF(OSSL_CMP_REVDETAILS) OSSL_CMP_REVREQCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:559:1 */
typedef STACK_OF(X509_CRL) OSSL_CMP_CRLANNCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:561:1 */
typedef STACK_OF(OSSL_CMP_ITAV) OSSL_CMP_GENMSGCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:563:1 */
typedef STACK_OF(OSSL_CMP_ITAV) OSSL_CMP_GENREPCONTENT;

/** clang-extract: from crypto/cmp/cmp_local.h:597:1 */
typedef struct ossl_cmp_pkibody_st {
    int type;
    union {
        OSSL_CRMF_MSGS *ir; /* 0 */
        OSSL_CMP_CERTREPMESSAGE *ip; /* 1 */
        OSSL_CRMF_MSGS *cr; /* 2 */
        OSSL_CMP_CERTREPMESSAGE *cp; /* 3 */
        /*-
         * p10cr      [4]  CertificationRequest,     --imported from [PKCS10]
         *
         * PKCS10_CERTIFICATIONREQUEST is effectively X509_REQ
         * so it is used directly
         */
        X509_REQ *p10cr; /* 4 */
        /*-
         * popdecc    [5]  POPODecKeyChallContent, --pop Challenge
         *
         * POPODecKeyChallContent ::= SEQUENCE OF Challenge
         */
        OSSL_CMP_POPODECKEYCHALLCONTENT *popdecc; /* 5 */
        /*-
         * popdecr    [6]  POPODecKeyRespContent,  --pop Response
         *
         * POPODecKeyRespContent ::= SEQUENCE OF INTEGER
         */
        OSSL_CMP_POPODECKEYRESPCONTENT *popdecr; /* 6 */
        OSSL_CRMF_MSGS *kur; /* 7 */
        OSSL_CMP_CERTREPMESSAGE *kup; /* 8 */
        OSSL_CRMF_MSGS *krr; /* 9 */

        /*-
         * krp        [10] KeyRecRepContent,         --Key Recovery Response
         */
        OSSL_CMP_KEYRECREPCONTENT *krp; /* 10 */
        /*-
         * rr         [11] RevReqContent,            --Revocation Request
         */
        OSSL_CMP_REVREQCONTENT *rr; /* 11 */
        /*-
         * rp         [12] RevRepContent,            --Revocation Response
         */
        OSSL_CMP_REVREPCONTENT *rp; /* 12 */
        /*-
         * ccr        [13] CertReqMessages,          --Cross-Cert. Request
         */
        OSSL_CRMF_MSGS *ccr; /* 13 */
        /*-
         * ccp        [14] CertRepMessage,           --Cross-Cert. Response
         */
        OSSL_CMP_CERTREPMESSAGE *ccp; /* 14 */
        /*-
         * ckuann     [15] CAKeyUpdAnnContent,       --CA Key Update Ann.
         */
        OSSL_CMP_CAKEYUPDANNCONTENT *ckuann; /* 15 */
        /*-
         * cann       [16] CertAnnContent,           --Certificate Ann.
         * OSSL_CMP_CMPCERTIFICATE is effectively X509 so it is used directly
         */
        X509 *cann; /* 16 */
        /*-
         * rann       [17] RevAnnContent,            --Revocation Ann.
         */
        OSSL_CMP_REVANNCONTENT *rann; /* 17 */
        /*-
         * crlann     [18] CRLAnnContent,            --CRL Announcement
         * CRLAnnContent ::= SEQUENCE OF CertificateList
         */
        OSSL_CMP_CRLANNCONTENT *crlann; /* 18 */
        /*-
         * PKIConfirmContent ::= NULL
         * pkiconf    [19] PKIConfirmContent,        --Confirmation
         * OSSL_CMP_PKICONFIRMCONTENT would be only a typedef of ASN1_NULL
         * OSSL_CMP_CONFIRMCONTENT *pkiconf;
         *
         * NOTE: this should ASN1_NULL according to the RFC
         * but there might be a struct in it when sent from faulty servers...
         */
        ASN1_TYPE *pkiconf; /* 19 */
        /*-
         * nested     [20] NestedMessageContent,     --Nested Message
         * NestedMessageContent ::= PKIMessages
         */
        OSSL_CMP_MSGS *nested; /* 20 */
        /*-
         * genm       [21] GenMsgContent,            --General Message
         * GenMsgContent ::= SEQUENCE OF InfoTypeAndValue
         */
        OSSL_CMP_GENMSGCONTENT *genm; /* 21 */
        /*-
         * genp       [22] GenRepContent,            --General Response
         * GenRepContent ::= SEQUENCE OF InfoTypeAndValue
         */
        OSSL_CMP_GENREPCONTENT *genp; /* 22 */
        /*-
         * error      [23] ErrorMsgContent,          --Error Message
         */
        OSSL_CMP_ERRORMSGCONTENT *error; /* 23 */
        /*-
         * certConf [24] CertConfirmContent,     --Certificate confirm
         */
        OSSL_CMP_CERTCONFIRMCONTENT *certConf; /* 24 */
        /*-
         * pollReq    [25] PollReqContent,           --Polling request
         */
        OSSL_CMP_POLLREQCONTENT *pollReq; /* 25 */
        /*-
         * pollRep    [26] PollRepContent            --Polling response
         */
        OSSL_CMP_POLLREPCONTENT *pollRep; /* 26 */
    } value;
} OSSL_CMP_PKIBODY;

/** clang-extract: from crypto/cmp/cmp_local.h:723:1 */
struct ossl_cmp_msg_st {
    OSSL_CMP_PKIHEADER *header;
    OSSL_CMP_PKIBODY *body;
    ASN1_BIT_STRING *protection; /* 0 */
    /* OSSL_CMP_CMPCERTIFICATE is effectively X509 so it is used directly */
    STACK_OF(X509) *extraCerts; /* 1 */
    OSSL_LIB_CTX *libctx;
    char *propq;
};

/** clang-extract: from crypto/cmp/cmp_local.h:817:1 */
int ossl_cmp_asn1_get_int(const ASN1_INTEGER *a);

/** clang-extract: from crypto/cmp/cmp_local.h:837:1 */
int ossl_cmp_print_log(OSSL_CMP_severity level, const OSSL_CMP_CTX *ctx,
                       const char *func, const char *file, int line,
                       const char *level_str, const char *format, ...);

#define ossl_cmp_log(level, ctx, msg) \
    ossl_cmp_print_log(OSSL_CMP_LOG_##level, ctx, OPENSSL_FUNC, OPENSSL_FILE, \
                       OPENSSL_LINE, #level, "%s", msg)
#define ossl_cmp_log1(level, ctx, fmt, arg1) \
    ossl_cmp_print_log(OSSL_CMP_LOG_##level, ctx, OPENSSL_FUNC, OPENSSL_FILE, \
                       OPENSSL_LINE, #level, fmt, arg1)
#define ossl_cmp_log2(level, ctx, fmt, arg1, arg2) \
    ossl_cmp_print_log(OSSL_CMP_LOG_##level, ctx, OPENSSL_FUNC, OPENSSL_FILE, \
                       OPENSSL_LINE, #level, fmt, arg1, arg2)
#define OSSL_CMP_LOG_ERROR OSSL_CMP_LOG_ERR
#define OSSL_CMP_LOG_WARN OSSL_CMP_LOG_WARNING
#define ossl_cmp_err(ctx, msg)   ossl_cmp_log(ERROR, ctx, msg)
#define ossl_cmp_warn(ctx, msg)  ossl_cmp_log(WARN,  ctx, msg)
#define ossl_cmp_info(ctx, msg)  ossl_cmp_log(INFO,  ctx, msg)
#define ossl_cmp_debug(ctx, msg) ossl_cmp_log(DEBUG, ctx, msg)
/** clang-extract: from crypto/cmp/cmp_local.h:865:1 */
int ossl_cmp_ctx_set0_statusString(OSSL_CMP_CTX *ctx,
                                   OSSL_CMP_PKIFREETEXT *text);

/** clang-extract: from crypto/cmp/cmp_local.h:868:1 */
int ossl_cmp_ctx_set0_newCert(OSSL_CMP_CTX *ctx, X509 *cert);

/** clang-extract: from crypto/cmp/cmp_local.h:869:1 */
int ossl_cmp_ctx_set1_newChain(OSSL_CMP_CTX *ctx, STACK_OF(X509) *newChain);

/** clang-extract: from crypto/cmp/cmp_local.h:870:1 */
int ossl_cmp_ctx_set1_caPubs(OSSL_CMP_CTX *ctx, STACK_OF(X509) *caPubs);

/** clang-extract: from crypto/cmp/cmp_local.h:871:1 */
int ossl_cmp_ctx_set1_extraCertsIn(OSSL_CMP_CTX *ctx,
                                   STACK_OF(X509) *extraCertsIn);

/** clang-extract: from crypto/cmp/cmp_local.h:875:1 */
EVP_PKEY *ossl_cmp_ctx_get0_newPubkey(const OSSL_CMP_CTX *ctx);

/** clang-extract: from crypto/cmp/cmp_local.h:876:1 */
int ossl_cmp_ctx_set1_first_senderNonce(OSSL_CMP_CTX *ctx,
                                        const ASN1_OCTET_STRING *nonce);

/** clang-extract: from crypto/cmp/cmp_local.h:880:1 */
int ossl_cmp_pkisi_get_status(const OSSL_CMP_PKISI *si);

/** clang-extract: from crypto/cmp/cmp_local.h:883:1 */
int ossl_cmp_pkisi_get_pkifailureinfo(const OSSL_CMP_PKISI *si);

/** clang-extract: from crypto/cmp/cmp_local.h:904:1 */
int ossl_cmp_hdr_has_implicitConfirm(const OSSL_CMP_PKIHEADER *hdr);

#define OSSL_CMP_PKIBODY_IP        1
#define OSSL_CMP_PKIBODY_CP        3
#define OSSL_CMP_PKIBODY_P10CR     4
#define OSSL_CMP_PKIBODY_KUR       7
#define OSSL_CMP_PKIBODY_KUP       8
#define OSSL_CMP_PKIBODY_RP       12
#define OSSL_CMP_PKIBODY_PKICONF  19
#define OSSL_CMP_PKIBODY_ERROR    23
#define OSSL_CMP_PKIBODY_POLLREP  26
#define OSSL_CMP_CERTREQID         0
#define OSSL_CMP_CERTREQID_NONE    -1
#define OSSL_CMP_REVREQSID 0
/** clang-extract: from crypto/cmp/cmp_local.h:948:1 */
const char *ossl_cmp_bodytype_to_string(int type);

/** clang-extract: from crypto/cmp/cmp_local.h:951:1 */
OSSL_CMP_MSG *ossl_cmp_certreq_new(OSSL_CMP_CTX *ctx, int bodytype,
                                   const OSSL_CRMF_MSG *crm);

/** clang-extract: from crypto/cmp/cmp_local.h:959:1 */
OSSL_CMP_MSG *ossl_cmp_rr_new(OSSL_CMP_CTX *ctx);

/** clang-extract: from crypto/cmp/cmp_local.h:972:1 */
OSSL_CMP_MSG *ossl_cmp_error_new(OSSL_CMP_CTX *ctx, const OSSL_CMP_PKISI *si,
                                 int64_t errorCode, const char *details,
                                 int unprotected);

/** clang-extract: from crypto/cmp/cmp_local.h:977:1 */
OSSL_CMP_MSG *ossl_cmp_certConf_new(OSSL_CMP_CTX *ctx, int certReqId,
                                    int fail_info, const char *text);

/** clang-extract: from crypto/cmp/cmp_local.h:979:1 */
OSSL_CMP_MSG *ossl_cmp_pollReq_new(OSSL_CMP_CTX *ctx, int crid);

/** clang-extract: from crypto/cmp/cmp_local.h:982:1 */
OSSL_CMP_PKISI *
ossl_cmp_revrepcontent_get_pkisi(OSSL_CMP_REVREPCONTENT *rrep, int rsid);

/** clang-extract: from crypto/cmp/cmp_local.h:984:1 */
OSSL_CRMF_CERTID *ossl_cmp_revrepcontent_get_CertId(OSSL_CMP_REVREPCONTENT *rc,
                                                    int rsid);

/** clang-extract: from crypto/cmp/cmp_local.h:986:1 */
OSSL_CMP_POLLREP *
ossl_cmp_pollrepcontent_get0_pollrep(const OSSL_CMP_POLLREPCONTENT *prc,
                                     int rid);

/** clang-extract: from crypto/cmp/cmp_local.h:989:1 */
OSSL_CMP_CERTRESPONSE *
ossl_cmp_certrepmessage_get0_certresponse(const OSSL_CMP_CERTREPMESSAGE *crm,
                                          int rid);

/** clang-extract: from crypto/cmp/cmp_local.h:992:1 */
X509 *ossl_cmp_certresponse_get1_cert(const OSSL_CMP_CTX *ctx,
                                      const OSSL_CMP_CERTRESPONSE *crep);

/** clang-extract: from crypto/cmp/cmp_local.h:995:1 */
int ossl_cmp_is_error_with_waiting(const OSSL_CMP_MSG *msg);

/** clang-extract: from crypto/cmp/cmp_local.h:1005:1 */
typedef int (*ossl_cmp_allow_unprotected_cb_t)(const OSSL_CMP_CTX *ctx,
                                               const OSSL_CMP_MSG *msg,
                                               int invalid_protection, int arg);

/** clang-extract: from crypto/cmp/cmp_local.h:1008:1 */
int ossl_cmp_msg_check_update(OSSL_CMP_CTX *ctx, const OSSL_CMP_MSG *msg,
                              ossl_cmp_allow_unprotected_cb_t cb, int cb_arg);

#define OSSL_CMP_EXPECTED_RESP_TIME 2
/** clang-extract: from crypto/cmp/cmp_local.h:1018:1 */
int ossl_cmp_exchange_certConf(OSSL_CMP_CTX *ctx, int certReqId,
                               int fail_info, const char *txt);

/** clang-extract: from crypto/cmp/cmp_local.h:1020:1 */
int ossl_cmp_exchange_error(OSSL_CMP_CTX *ctx, int status, int fail_info,
                            const char *txt, int errorCode, const char *detail);

#define IS_CREP(t) ((t) == OSSL_CMP_PKIBODY_IP || (t) == OSSL_CMP_PKIBODY_CP \
                        || (t) == OSSL_CMP_PKIBODY_KUP)
/** clang-extract: from crypto/cmp/cmp_client.c:31:1 */
static int unprotected_exception(const OSSL_CMP_CTX *ctx,
                                 const OSSL_CMP_MSG *rep,
                                 int invalid_protection,
                                 ossl_unused int expected_type)
{
    int rcvd_type = OSSL_CMP_MSG_get_bodytype(rep /* may be NULL */);
    const char *msg_type = NULL;

    if (!ossl_assert(ctx != NULL && rep != NULL))
        return -1;

    if (!ctx->unprotectedErrors)
        return 0;

    switch (rcvd_type) {
    case OSSL_CMP_PKIBODY_ERROR:
        msg_type = "error response";
        break;
    case OSSL_CMP_PKIBODY_RP:
        {
            OSSL_CMP_PKISI *si =
                ossl_cmp_revrepcontent_get_pkisi(rep->body->value.rp,
                                                 OSSL_CMP_REVREQSID);

            if (si == NULL)
                return -1;
            if (ossl_cmp_pkisi_get_status(si) == OSSL_CMP_PKISTATUS_rejection)
                msg_type = "revocation response message with rejection status";
            break;
        }
    case OSSL_CMP_PKIBODY_PKICONF:
        msg_type = "PKI Confirmation message";
        break;
    default:
        if (IS_CREP(rcvd_type)) {
            int any_rid = OSSL_CMP_CERTREQID_NONE;
            OSSL_CMP_CERTREPMESSAGE *crepmsg = rep->body->value.ip;
            OSSL_CMP_CERTRESPONSE *crep =
                ossl_cmp_certrepmessage_get0_certresponse(crepmsg, any_rid);

            if (sk_OSSL_CMP_CERTRESPONSE_num(crepmsg->response) > 1)
                return -1;
            if (crep == NULL)
                return -1;
            if (ossl_cmp_pkisi_get_status(crep->status)
                == OSSL_CMP_PKISTATUS_rejection)
                msg_type = "CertRepMessage with rejection status";
        }
    }
    if (msg_type == NULL)
        return 0;
    ossl_cmp_log2(WARN, ctx, "ignoring %s protection of %s",
                  invalid_protection ? "invalid" : "missing", msg_type);
    return 1;
}

/** clang-extract: from crypto/cmp/cmp_client.c:88:1 */
static int save_statusInfo(OSSL_CMP_CTX *ctx, OSSL_CMP_PKISI *si)
{
    int i;
    OSSL_CMP_PKIFREETEXT *ss;

    if (!ossl_assert(ctx != NULL && si != NULL))
        return 0;

    ctx->status = ossl_cmp_pkisi_get_status(si);
    if (ctx->status < OSSL_CMP_PKISTATUS_accepted)
        return 0;

    ctx->failInfoCode = ossl_cmp_pkisi_get_pkifailureinfo(si);

    if (!ossl_cmp_ctx_set0_statusString(ctx, sk_ASN1_UTF8STRING_new_null())
            || (ctx->statusString == NULL))
        return 0;

    ss = si->statusString; /* may be NULL */
    for (i = 0; i < sk_ASN1_UTF8STRING_num(ss); i++) {
        ASN1_UTF8STRING *str = sk_ASN1_UTF8STRING_value(ss, i);
        ASN1_UTF8STRING *dup = ASN1_STRING_dup(str);

        if (dup == NULL || !sk_ASN1_UTF8STRING_push(ctx->statusString, dup)) {
            ASN1_UTF8STRING_free(dup);
            return 0;
        }
    }
    return 1;
}

/** clang-extract: from crypto/cmp/cmp_client.c:119:1 */
static int is_crep_with_waiting(const OSSL_CMP_MSG *resp, int rid)
{
    OSSL_CMP_CERTREPMESSAGE *crepmsg;
    OSSL_CMP_CERTRESPONSE *crep;
    int bt = OSSL_CMP_MSG_get_bodytype(resp);

    if (!IS_CREP(bt))
        return 0;

    crepmsg = resp->body->value.ip; /* same for cp and kup */
    crep = ossl_cmp_certrepmessage_get0_certresponse(crepmsg, rid);

    return (crep != NULL
            && ossl_cmp_pkisi_get_status(crep->status)
            == OSSL_CMP_PKISTATUS_waiting);
}

/** clang-extract: from crypto/cmp/cmp_client.c:142:1 */
static int send_receive_check(OSSL_CMP_CTX *ctx, const OSSL_CMP_MSG *req,
                              OSSL_CMP_MSG **rep, int expected_type)
{
    int begin_transaction =
        expected_type != OSSL_CMP_PKIBODY_POLLREP
        && expected_type != OSSL_CMP_PKIBODY_PKICONF;
    const char *req_type_str =
        ossl_cmp_bodytype_to_string(OSSL_CMP_MSG_get_bodytype(req));
    const char *expected_type_str = ossl_cmp_bodytype_to_string(expected_type);
    int bak_msg_timeout = ctx->msg_timeout;
    int bt;
    time_t now = time(NULL);
    int time_left;
    OSSL_CMP_transfer_cb_t transfer_cb = ctx->transfer_cb;

#ifndef OPENSSL_NO_HTTP
    if (transfer_cb == NULL)
        transfer_cb = OSSL_CMP_MSG_http_perform;
#endif
    *rep = NULL;

    if (ctx->total_timeout != 0 /* not waiting indefinitely */) {
        if (begin_transaction)
            ctx->end_time = now + ctx->total_timeout;
        if (now >= ctx->end_time) {
            ERR_raise(ERR_LIB_CMP, CMP_R_TOTAL_TIMEOUT);
            return 0;
        }
        if (!ossl_assert(ctx->end_time - now < INT_MAX)) {
            /* actually cannot happen due to assignment in initial_certreq() */
            ERR_raise(ERR_LIB_CMP, CMP_R_INVALID_ARGS);
            return 0;
        }
        time_left = (int)(ctx->end_time - now);
        if (ctx->msg_timeout == 0 || time_left < ctx->msg_timeout)
            ctx->msg_timeout = time_left;
    }

    /* should print error queue since transfer_cb may call ERR_clear_error() */
    OSSL_CMP_CTX_print_errors(ctx);

    if (ctx->server != NULL)
        ossl_cmp_log1(INFO, ctx, "sending %s", req_type_str);

    *rep = (*transfer_cb)(ctx, req);
    ctx->msg_timeout = bak_msg_timeout;

    if (*rep == NULL) {
        ERR_raise_data(ERR_LIB_CMP,
                       ctx->total_timeout != 0 && time(NULL) >= ctx->end_time ?
                       CMP_R_TOTAL_TIMEOUT : CMP_R_TRANSFER_ERROR,
                       "request sent: %s, expected response: %s",
                       req_type_str, expected_type_str);
        return 0;
    }

    bt = OSSL_CMP_MSG_get_bodytype(*rep);
    /*
     * The body type in the 'bt' variable is not yet verified.
     * Still we use this preliminary value already for a progress report because
     * the following msg verification may also produce log entries and may fail.
     */
    ossl_cmp_log2(INFO, ctx, "received %s%s", ossl_cmp_bodytype_to_string(bt),
                  ossl_cmp_is_error_with_waiting(*rep) ? " (waiting)" : "");

    /* copy received extraCerts to ctx->extraCertsIn so they can be retrieved */
    if (bt != OSSL_CMP_PKIBODY_POLLREP && bt != OSSL_CMP_PKIBODY_PKICONF
            && !ossl_cmp_ctx_set1_extraCertsIn(ctx, (*rep)->extraCerts))
        return 0;

    if (!ossl_cmp_msg_check_update(ctx, *rep, unprotected_exception,
                                   expected_type))
        return 0;

    /*
     * rep can have the expected response type, which during polling is pollRep.
     * When polling, also any other non-error response (the final response)
     * is fine here. When not yet polling, delayed delivery may be initiated
     * by the server returning an error message with 'waiting' status (or a
     * response message of expected type ip/cp/kup with 'waiting' status).
     */
    if (bt == expected_type
        || (expected_type == OSSL_CMP_PKIBODY_POLLREP
            ? bt != OSSL_CMP_PKIBODY_ERROR
            : ossl_cmp_is_error_with_waiting(*rep)))
        return 1;

    /* received message type is not one of the expected ones (e.g., error) */
    ERR_raise(ERR_LIB_CMP, bt == OSSL_CMP_PKIBODY_ERROR ? CMP_R_RECEIVED_ERROR :
              CMP_R_UNEXPECTED_PKIBODY); /* in next line for mkerr.pl */

    if (bt != OSSL_CMP_PKIBODY_ERROR) {
        ERR_add_error_data(3, "message type is '",
                           ossl_cmp_bodytype_to_string(bt), "'");
    } else {
        OSSL_CMP_ERRORMSGCONTENT *emc = (*rep)->body->value.error;
        OSSL_CMP_PKISI *si = emc->pKIStatusInfo;
        char buf[OSSL_CMP_PKISI_BUFLEN];

        if (save_statusInfo(ctx, si)
                && OSSL_CMP_CTX_snprint_PKIStatus(ctx, buf,
                                                  sizeof(buf)) != NULL)
            ERR_add_error_data(1, buf);
        if (emc->errorCode != NULL
                && BIO_snprintf(buf, sizeof(buf), "; errorCode: %08lX",
                                ASN1_INTEGER_get(emc->errorCode)) > 0)
            ERR_add_error_data(1, buf);
        if (emc->errorDetails != NULL) {
            char *text = ossl_sk_ASN1_UTF8STRING2text(emc->errorDetails, ", ",
                                                      OSSL_CMP_PKISI_BUFLEN - 1);

            if (text != NULL && *text != '\0')
                ERR_add_error_data(2, "; errorDetails: ", text);
            OPENSSL_free(text);
        }
        if (ctx->status != OSSL_CMP_PKISTATUS_rejection) {
            ERR_raise(ERR_LIB_CMP, CMP_R_UNEXPECTED_PKISTATUS);
            if (ctx->status == OSSL_CMP_PKISTATUS_waiting)
                ctx->status = OSSL_CMP_PKISTATUS_rejection;
        }
    }
    return 0;
}

/** clang-extract: from crypto/cmp/cmp_client.c:282:1 */
static int poll_for_response(OSSL_CMP_CTX *ctx, int sleep, int rid,
                             OSSL_CMP_MSG **rep, int *checkAfter)
{
    OSSL_CMP_MSG *preq = NULL;
    OSSL_CMP_MSG *prep = NULL;

    ossl_cmp_info(ctx,
                  "received 'waiting' PKIStatus, starting to poll for response");
    *rep = NULL;
    for (;;) {
        if ((preq = ossl_cmp_pollReq_new(ctx, rid)) == NULL)
            goto err;

        if (!send_receive_check(ctx, preq, &prep, OSSL_CMP_PKIBODY_POLLREP))
            goto err;

        /* handle potential pollRep */
        if (OSSL_CMP_MSG_get_bodytype(prep) == OSSL_CMP_PKIBODY_POLLREP) {
            OSSL_CMP_POLLREPCONTENT *prc = prep->body->value.pollRep;
            OSSL_CMP_POLLREP *pollRep = NULL;
            int64_t check_after;
            char str[OSSL_CMP_PKISI_BUFLEN];
            int len;

            if (sk_OSSL_CMP_POLLREP_num(prc) > 1) {
                ERR_raise(ERR_LIB_CMP, CMP_R_MULTIPLE_RESPONSES_NOT_SUPPORTED);
                goto err;
            }
            pollRep = ossl_cmp_pollrepcontent_get0_pollrep(prc, rid);
            if (pollRep == NULL)
                goto err;

            if (!ASN1_INTEGER_get_int64(&check_after, pollRep->checkAfter)) {
                ERR_raise(ERR_LIB_CMP, CMP_R_BAD_CHECKAFTER_IN_POLLREP);
                goto err;
            }
            if (check_after < 0 || (uint64_t)check_after
                > (sleep ? ULONG_MAX / 1000 : INT_MAX)) {
                ERR_raise(ERR_LIB_CMP, CMP_R_CHECKAFTER_OUT_OF_RANGE);
                if (BIO_snprintf(str, OSSL_CMP_PKISI_BUFLEN, "value = %jd",
                                 check_after) >= 0)
                    ERR_add_error_data(1, str);
                goto err;
            }

            if (pollRep->reason == NULL
                    || (len = BIO_snprintf(str, OSSL_CMP_PKISI_BUFLEN,
                                           " with reason = '")) < 0) {
                *str = '\0';
            } else {
                char *text = ossl_sk_ASN1_UTF8STRING2text(pollRep->reason, ", ",
                                                          sizeof(str) - len - 2);

                if (text == NULL
                        || BIO_snprintf(str + len, sizeof(str) - len,
                                        "%s'", text) < 0)
                    *str = '\0';
                OPENSSL_free(text);
            }
            ossl_cmp_log2(INFO, ctx,
                          "received polling response%s; checkAfter = %ld seconds",
                          str, check_after);

            if (ctx->total_timeout != 0) { /* timeout is not infinite */
                const int exp = OSSL_CMP_EXPECTED_RESP_TIME;
                int64_t time_left = (int64_t)(ctx->end_time - exp - time(NULL));

                if (time_left <= 0) {
                    ERR_raise(ERR_LIB_CMP, CMP_R_TOTAL_TIMEOUT);
                    goto err;
                }
                if (time_left < check_after)
                    check_after = time_left;
                /* poll one last time just when timeout was reached */
            }

            OSSL_CMP_MSG_free(preq);
            preq = NULL;
            OSSL_CMP_MSG_free(prep);
            prep = NULL;
            if (sleep) {
                OSSL_sleep((unsigned long)(1000 * check_after));
            } else {
                if (checkAfter != NULL)
                    *checkAfter = (int)check_after;
                return -1; /* exits the loop */
            }
        } else if (is_crep_with_waiting(prep, rid)
                   || ossl_cmp_is_error_with_waiting(prep)) {
            /* received status must not be 'waiting' */
            (void)ossl_cmp_exchange_error(ctx, OSSL_CMP_PKISTATUS_rejection,
                                          OSSL_CMP_CTX_FAILINFO_badRequest,
                                          "polling already started",
                                          0 /* errorCode */, NULL);
            ERR_raise(ERR_LIB_CMP, CMP_R_UNEXPECTED_PKISTATUS);
            goto err;
        } else {
            ossl_cmp_info(ctx, "received final response after polling");
            if (!ossl_cmp_ctx_set1_first_senderNonce(ctx, NULL))
                return 0;
            break;
        }
    }
    if (prep == NULL)
        goto err;

    OSSL_CMP_MSG_free(preq);
    *rep = prep;

    return 1;
 err:
    (void)ossl_cmp_ctx_set1_first_senderNonce(ctx, NULL);
    OSSL_CMP_MSG_free(preq);
    OSSL_CMP_MSG_free(prep);
    return 0;
}

/** clang-extract: from crypto/cmp/cmp_client.c:399:1 */
static int save_senderNonce_if_waiting(OSSL_CMP_CTX *ctx,
                                       const OSSL_CMP_MSG *rep, int rid)
{
    /*
     * Lightweight CMP Profile section 4.4 states: the senderNonce of the
     * preceding request message because this value will be needed for checking
     * the recipNonce of the final response to be received after polling.
     */
    if ((is_crep_with_waiting(rep, rid)
         || ossl_cmp_is_error_with_waiting(rep))
        && !ossl_cmp_ctx_set1_first_senderNonce(ctx, ctx->senderNonce))
        return 0;

    return 1;
}

/** clang-extract: from crypto/cmp/cmp_client.c:419:1 */
static int send_receive_also_delayed(OSSL_CMP_CTX *ctx, const OSSL_CMP_MSG *req,
                                     OSSL_CMP_MSG **rep, int expected_type)
{

    if (!send_receive_check(ctx, req, rep, expected_type))
        return 0;

    if (ossl_cmp_is_error_with_waiting(*rep)) {
        if (!save_senderNonce_if_waiting(ctx, *rep, OSSL_CMP_CERTREQID_NONE))
            return 0;
        /* not modifying ctx->status during certConf and error exchanges */
        if (expected_type != OSSL_CMP_PKIBODY_PKICONF
            && !save_statusInfo(ctx, (*rep)->body->value.error->pKIStatusInfo))
            return 0;

        OSSL_CMP_MSG_free(*rep);
        *rep = NULL;

        if (poll_for_response(ctx, 1 /* can sleep */, OSSL_CMP_CERTREQID_NONE,
                              rep, NULL /* checkAfter */) <= 0) {
            ERR_raise(ERR_LIB_CMP, CMP_R_POLLING_FAILED);
            return 0;
        }
    }
    if (OSSL_CMP_MSG_get_bodytype(*rep) != expected_type) {
        ERR_raise(ERR_LIB_CMP, CMP_R_UNEXPECTED_PKIBODY);
        return 0;
    }

    return 1;
}

/** clang-extract: from crypto/cmp/cmp_client.c:454:1 */
int ossl_cmp_exchange_certConf(OSSL_CMP_CTX *ctx, int certReqId,
                               int fail_info, const char *txt)
{
    OSSL_CMP_MSG *certConf;
    OSSL_CMP_MSG *PKIconf = NULL;
    int res = 0;

    /* OSSL_CMP_certConf_new() also checks if all necessary options are set */
    certConf = ossl_cmp_certConf_new(ctx, certReqId, fail_info, txt);
    if (certConf == NULL)
        goto err;

    res = send_receive_also_delayed(ctx, certConf, &PKIconf,
                                    OSSL_CMP_PKIBODY_PKICONF);

 err:
    OSSL_CMP_MSG_free(certConf);
    OSSL_CMP_MSG_free(PKIconf);
    return res;
}

/** clang-extract: from crypto/cmp/cmp_client.c:476:1 */
int ossl_cmp_exchange_error(OSSL_CMP_CTX *ctx, int status, int fail_info,
                            const char *txt, int errorCode, const char *details)
{
    OSSL_CMP_MSG *error = NULL;
    OSSL_CMP_PKISI *si = NULL;
    OSSL_CMP_MSG *PKIconf = NULL;
    int res = 0;

    /* not overwriting ctx->status on error exchange */
    if ((si = OSSL_CMP_STATUSINFO_new(status, fail_info, txt)) == NULL)
        goto err;
    /* ossl_cmp_error_new() also checks if all necessary options are set */
    if ((error = ossl_cmp_error_new(ctx, si, errorCode, details, 0)) == NULL)
        goto err;

    res = send_receive_also_delayed(ctx, error,
                                    &PKIconf, OSSL_CMP_PKIBODY_PKICONF);

 err:
    OSSL_CMP_MSG_free(error);
    OSSL_CMP_PKISI_free(si);
    OSSL_CMP_MSG_free(PKIconf);
    return res;
}

/** clang-extract: from crypto/cmp/cmp_client.c:506:1 */
static X509 *get1_cert_status(OSSL_CMP_CTX *ctx, int bodytype,
                              OSSL_CMP_CERTRESPONSE *crep)
{
    char buf[OSSL_CMP_PKISI_BUFLEN];
    X509 *crt = NULL;

    if (!ossl_assert(ctx != NULL && crep != NULL))
        return NULL;

    switch (ossl_cmp_pkisi_get_status(crep->status)) {
    case OSSL_CMP_PKISTATUS_waiting:
        ossl_cmp_err(ctx,
                     "received \"waiting\" status for cert when actually aiming to extract cert");
        ERR_raise(ERR_LIB_CMP, CMP_R_ENCOUNTERED_WAITING);
        goto err;
    case OSSL_CMP_PKISTATUS_grantedWithMods:
        ossl_cmp_warn(ctx, "received \"grantedWithMods\" for certificate");
        break;
    case OSSL_CMP_PKISTATUS_accepted:
        break;
        /* get all information in case of a rejection before going to error */
    case OSSL_CMP_PKISTATUS_rejection:
        ossl_cmp_err(ctx, "received \"rejection\" status rather than cert");
        ERR_raise(ERR_LIB_CMP, CMP_R_REQUEST_REJECTED_BY_SERVER);
        goto err;
    case OSSL_CMP_PKISTATUS_revocationWarning:
        ossl_cmp_warn(ctx,
                      "received \"revocationWarning\" - a revocation of the cert is imminent");
        break;
    case OSSL_CMP_PKISTATUS_revocationNotification:
        ossl_cmp_warn(ctx,
                      "received \"revocationNotification\" - a revocation of the cert has occurred");
        break;
    case OSSL_CMP_PKISTATUS_keyUpdateWarning:
        if (bodytype != OSSL_CMP_PKIBODY_KUR) {
            ERR_raise(ERR_LIB_CMP, CMP_R_ENCOUNTERED_KEYUPDATEWARNING);
            goto err;
        }
        break;
    default:
        ossl_cmp_log1(ERROR, ctx,
                      "received unsupported PKIStatus %d for certificate",
                      ctx->status);
        ERR_raise(ERR_LIB_CMP, CMP_R_UNKNOWN_PKISTATUS);
        goto err;
    }
    crt = ossl_cmp_certresponse_get1_cert(ctx, crep);
    if (crt == NULL) /* according to PKIStatus, we can expect a cert */
        ERR_raise(ERR_LIB_CMP, CMP_R_CERTIFICATE_NOT_FOUND);

    return crt;

 err:
    if (OSSL_CMP_CTX_snprint_PKIStatus(ctx, buf, sizeof(buf)) != NULL)
        ERR_add_error_data(1, buf);
    return NULL;
}

/** clang-extract: from crypto/cmp/cmp_client.c:581:1 */
int OSSL_CMP_certConf_cb(OSSL_CMP_CTX *ctx, X509 *cert, int fail_info,
                         const char **text)
{
    X509_STORE *out_trusted = OSSL_CMP_CTX_get_certConf_cb_arg(ctx);
    STACK_OF(X509) *chain = NULL;

    (void)text; /* make (artificial) use of var to prevent compiler warning */

    if (fail_info != 0) /* accept any error flagged by CMP core library */
        return fail_info;

    if (out_trusted == NULL) {
        ossl_cmp_debug(ctx, "trying to build chain for newly enrolled cert");
        chain = X509_build_chain(cert, ctx->untrusted, out_trusted,
                                 0, ctx->libctx, ctx->propq);
    } else {
        X509_STORE_CTX *csc = X509_STORE_CTX_new_ex(ctx->libctx, ctx->propq);

        ossl_cmp_debug(ctx, "validating newly enrolled cert");
        if (csc == NULL)
            goto err;
        if (!X509_STORE_CTX_init(csc, out_trusted, cert, ctx->untrusted))
            goto err;
        /* disable any cert status/revocation checking etc. */
        X509_VERIFY_PARAM_clear_flags(X509_STORE_CTX_get0_param(csc),
                                      ~(X509_V_FLAG_USE_CHECK_TIME
                                        | X509_V_FLAG_NO_CHECK_TIME
                                        | X509_V_FLAG_PARTIAL_CHAIN
                                        | X509_V_FLAG_POLICY_CHECK));
        if (X509_verify_cert(csc) <= 0)
            goto err;

        if (!ossl_x509_add_certs_new(&chain, X509_STORE_CTX_get0_chain(csc),
                                     X509_ADD_FLAG_UP_REF | X509_ADD_FLAG_NO_DUP
                                     | X509_ADD_FLAG_NO_SS)) {
            sk_X509_free(chain);
            chain = NULL;
        }
    err:
        X509_STORE_CTX_free(csc);
    }

    if (sk_X509_num(chain) > 0)
        X509_free(sk_X509_shift(chain)); /* remove leaf (EE) cert */
    if (out_trusted != NULL) {
        if (chain == NULL) {
            ossl_cmp_err(ctx, "failed to validate newly enrolled cert");
            fail_info = 1 << OSSL_CMP_PKIFAILUREINFO_incorrectData;
        } else {
            ossl_cmp_debug(ctx,
                           "success validating newly enrolled cert");
        }
    } else if (chain == NULL) {
        ossl_cmp_warn(ctx, "could not build approximate chain for newly enrolled cert, resorting to received extraCerts");
        chain = OSSL_CMP_CTX_get1_extraCertsIn(ctx);
    } else {
        ossl_cmp_debug(ctx,
                       "success building approximate chain for newly enrolled cert");
    }
    (void)ossl_cmp_ctx_set1_newChain(ctx, chain);
    OSSL_STACK_OF_X509_free(chain);

    return fail_info;
}

/** clang-extract: from crypto/cmp/cmp_client.c:654:1 */
static int cert_response(OSSL_CMP_CTX *ctx, int sleep, int rid,
                         OSSL_CMP_MSG **resp, int *checkAfter,
                         ossl_unused int req_type,
                         ossl_unused int expected_type)
{
    EVP_PKEY *rkey = NULL;
    int fail_info = 0; /* no failure */
    const char *txt = NULL;
    OSSL_CMP_CERTREPMESSAGE *crepmsg = NULL;
    OSSL_CMP_CERTRESPONSE *crep = NULL;
    OSSL_CMP_certConf_cb_t cb;
    X509 *cert;
    char *subj = NULL;
    int ret = 1;
    int rcvd_type;
    OSSL_CMP_PKISI *si;

    if (!ossl_assert(ctx != NULL))
        return 0;

 retry:
    rcvd_type = OSSL_CMP_MSG_get_bodytype(*resp);
    if (IS_CREP(rcvd_type)) {
        crepmsg = (*resp)->body->value.ip; /* same for cp and kup */
        if (sk_OSSL_CMP_CERTRESPONSE_num(crepmsg->response) > 1) {
            ERR_raise(ERR_LIB_CMP, CMP_R_MULTIPLE_RESPONSES_NOT_SUPPORTED);
            return 0;
        }
        crep = ossl_cmp_certrepmessage_get0_certresponse(crepmsg, rid);
        if (crep == NULL)
            return 0;
        si = crep->status;

        if (rid == OSSL_CMP_CERTREQID_NONE) {
            /* for OSSL_CMP_PKIBODY_P10CR learn CertReqId from response */
            rid = ossl_cmp_asn1_get_int(crep->certReqId);
            if (rid < OSSL_CMP_CERTREQID_NONE) {
                ERR_raise(ERR_LIB_CMP, CMP_R_BAD_REQUEST_ID);
                return 0;
            }
        }
    } else if (rcvd_type == OSSL_CMP_PKIBODY_ERROR) {
        si = (*resp)->body->value.error->pKIStatusInfo;
    } else {
        ERR_raise(ERR_LIB_CMP, CMP_R_UNEXPECTED_PKIBODY);
        return 0;
    }

    if (!save_statusInfo(ctx, si))
        return 0;

    if (ossl_cmp_pkisi_get_status(si) == OSSL_CMP_PKISTATUS_waiting) {
        /*
         * Here we allow both and error message with waiting indication
         * as well as a certificate response with waiting indication, where
         * its flavor (ip, cp, or kup) may not strictly match ir/cr/p10cr/kur.
         */
        OSSL_CMP_MSG_free(*resp);
        *resp = NULL;
        if ((ret = poll_for_response(ctx, sleep, rid, resp, checkAfter)) != 0) {
            if (ret == -1) /* at this point implies sleep == 0 */
                return ret; /* waiting */
            goto retry; /* got some response other than pollRep */
        } else {
            ERR_raise(ERR_LIB_CMP, CMP_R_POLLING_FAILED);
            return 0;
        }
    }

    /* at this point, we have received ip/cp/kup/error without waiting */
    if (rcvd_type == OSSL_CMP_PKIBODY_ERROR) {
        ERR_raise(ERR_LIB_CMP, CMP_R_RECEIVED_ERROR);
        return 0;
    }
    /* here we are strict on the flavor of ip/cp/kup: must match request */
    if (rcvd_type != expected_type) {
        ERR_raise(ERR_LIB_CMP, CMP_R_UNEXPECTED_PKIBODY);
        return 0;
    }

    cert = get1_cert_status(ctx, (*resp)->body->type, crep);
    if (cert == NULL) {
        ERR_add_error_data(1, "; cannot extract certificate from response");
        return 0;
    }
    if (!ossl_cmp_ctx_set0_newCert(ctx, cert)) {
        X509_free(cert);
        return 0;
    }

    /*
     * if the CMP server returned certificates in the caPubs field, copy them
     * to the context so that they can be retrieved if necessary
     */
    if (crepmsg != NULL && crepmsg->caPubs != NULL
            && !ossl_cmp_ctx_set1_caPubs(ctx, crepmsg->caPubs))
        return 0;

    subj = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
    rkey = ossl_cmp_ctx_get0_newPubkey(ctx);
    if (rkey != NULL
        /* X509_check_private_key() also works if rkey is just public key */
            && !(X509_check_private_key(ctx->newCert, rkey))) {
        fail_info = 1 << OSSL_CMP_PKIFAILUREINFO_incorrectData;
        txt = "public key in new certificate does not match our enrollment key";
        /*-
         * not calling (void)ossl_cmp_exchange_error(ctx,
         *                   OSSL_CMP_PKISTATUS_rejection, fail_info, txt)
         * not throwing CMP_R_CERTIFICATE_NOT_ACCEPTED with txt
         * not returning 0
         * since we better leave this for the certConf_cb to decide
         */
    }

    /*
     * Execute the certification checking callback function,
     * which can determine whether to accept a newly enrolled certificate.
     * It may overrule the pre-decision reflected in 'fail_info' and '*txt'.
     */
    cb = ctx->certConf_cb != NULL ? ctx->certConf_cb : OSSL_CMP_certConf_cb;
    if ((fail_info = cb(ctx, ctx->newCert, fail_info, &txt)) != 0
            && txt == NULL)
        txt = "CMP client did not accept it";
    if (fail_info != 0) /* immediately log error before any certConf exchange */
        ossl_cmp_log1(ERROR, ctx,
                      "rejecting newly enrolled cert with subject: %s", subj);
    /*
     * certConf exchange should better be moved to do_certreq_seq() such that
     * also more low-level errors with CertReqMessages get reported to server
     */
    if (!ctx->disableConfirm
            && !ossl_cmp_hdr_has_implicitConfirm((*resp)->header)) {
        if (!ossl_cmp_exchange_certConf(ctx, rid, fail_info, txt))
            ret = 0;
    }

    /* not throwing failure earlier as transfer_cb may call ERR_clear_error() */
    if (fail_info != 0) {
        ERR_raise_data(ERR_LIB_CMP, CMP_R_CERTIFICATE_NOT_ACCEPTED,
                       "rejecting newly enrolled cert with subject: %s; %s",
                       subj, txt);
        ctx->status = OSSL_CMP_PKISTATUS_rejection;
        ret = 0;
    }
    OPENSSL_free(subj);
    return ret;
}

/** clang-extract: from crypto/cmp/cmp_client.c:802:1 */
static int initial_certreq(OSSL_CMP_CTX *ctx,
                           int req_type, const OSSL_CRMF_MSG *crm,
                           OSSL_CMP_MSG **p_rep, int rep_type)
{
    OSSL_CMP_MSG *req;
    int res;

    ctx->status = OSSL_CMP_PKISTATUS_request;
    if (!ossl_cmp_ctx_set0_newCert(ctx, NULL))
        return 0;

    /* also checks if all necessary options are set */
    if ((req = ossl_cmp_certreq_new(ctx, req_type, crm)) == NULL)
        return 0;

    ctx->status = OSSL_CMP_PKISTATUS_trans;
    res = send_receive_check(ctx, req, p_rep, rep_type);
    OSSL_CMP_MSG_free(req);
    return res;
}

/** clang-extract: from crypto/cmp/cmp_client.c:867:1 */
X509 *OSSL_CMP_exec_certreq(OSSL_CMP_CTX *ctx, int req_type,
                            const OSSL_CRMF_MSG *crm)
{
    OSSL_CMP_MSG *rep = NULL;
    int is_p10 = req_type == OSSL_CMP_PKIBODY_P10CR;
    int rid = is_p10 ? OSSL_CMP_CERTREQID_NONE : OSSL_CMP_CERTREQID;
    int rep_type = is_p10 ? OSSL_CMP_PKIBODY_CP : req_type + 1;
    X509 *result = NULL;

    if (ctx == NULL) {
        ERR_raise(ERR_LIB_CMP, CMP_R_NULL_ARGUMENT);
        return NULL;
    }

    if (!initial_certreq(ctx, req_type, crm, &rep, rep_type))
        goto err;

    if (!save_senderNonce_if_waiting(ctx, rep, rid))
        return 0;

    if (cert_response(ctx, 1 /* sleep */, rid, &rep, NULL, req_type, rep_type)
        <= 0)
        goto err;

    result = ctx->newCert;
 err:
    OSSL_CMP_MSG_free(rep);
    return result;
}

/** clang-extract: from crypto/cmp/cmp_client.c:897:1 */
int OSSL_CMP_exec_RR_ses(OSSL_CMP_CTX *ctx)
{
    OSSL_CMP_MSG *rr = NULL;
    OSSL_CMP_MSG *rp = NULL;
    const int num_RevDetails = 1;
    const int rsid = OSSL_CMP_REVREQSID;
    OSSL_CMP_REVREPCONTENT *rrep = NULL;
    OSSL_CMP_PKISI *si = NULL;
    char buf[OSSL_CMP_PKISI_BUFLEN];
    int ret = 0;

    if (ctx == NULL) {
        ERR_raise(ERR_LIB_CMP, CMP_R_INVALID_ARGS);
        return 0;
    }
    ctx->status = OSSL_CMP_PKISTATUS_request;
    if (ctx->oldCert == NULL && ctx->p10CSR == NULL
        && (ctx->serialNumber == NULL || ctx->issuer == NULL)) {
        ERR_raise(ERR_LIB_CMP, CMP_R_MISSING_REFERENCE_CERT);
        return 0;
    }

    /* OSSL_CMP_rr_new() also checks if all necessary options are set */
    if ((rr = ossl_cmp_rr_new(ctx)) == NULL)
        goto end;

    ctx->status = OSSL_CMP_PKISTATUS_trans;
    if (!send_receive_also_delayed(ctx, rr, &rp, OSSL_CMP_PKIBODY_RP))
        goto end;

    rrep = rp->body->value.rp;
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    if (sk_OSSL_CMP_PKISI_num(rrep->status) != num_RevDetails) {
        ERR_raise(ERR_LIB_CMP, CMP_R_WRONG_RP_COMPONENT_COUNT);
        goto end;
    }
#else
    if (sk_OSSL_CMP_PKISI_num(rrep->status) < 1) {
        ERR_raise(ERR_LIB_CMP, CMP_R_WRONG_RP_COMPONENT_COUNT);
        goto end;
    }
#endif

    /* evaluate PKIStatus field */
    si = ossl_cmp_revrepcontent_get_pkisi(rrep, rsid);
    if (!save_statusInfo(ctx, si))
        goto err;
    switch (ossl_cmp_pkisi_get_status(si)) {
    case OSSL_CMP_PKISTATUS_accepted:
        ossl_cmp_info(ctx, "revocation accepted (PKIStatus=accepted)");
        ret = 1;
        break;
    case OSSL_CMP_PKISTATUS_grantedWithMods:
        ossl_cmp_info(ctx, "revocation accepted (PKIStatus=grantedWithMods)");
        ret = 1;
        break;
    case OSSL_CMP_PKISTATUS_rejection:
        ERR_raise(ERR_LIB_CMP, CMP_R_REQUEST_REJECTED_BY_SERVER);
        goto err;
    case OSSL_CMP_PKISTATUS_revocationWarning:
        ossl_cmp_info(ctx, "revocation accepted (PKIStatus=revocationWarning)");
        ret = 1;
        break;
    case OSSL_CMP_PKISTATUS_revocationNotification:
        /* interpretation as warning or error depends on CA */
        ossl_cmp_warn(ctx,
                      "revocation accepted (PKIStatus=revocationNotification)");
        ret = 1;
        break;
    case OSSL_CMP_PKISTATUS_waiting:
    case OSSL_CMP_PKISTATUS_keyUpdateWarning:
        ERR_raise(ERR_LIB_CMP, CMP_R_UNEXPECTED_PKISTATUS);
        goto err;
    default:
        ERR_raise(ERR_LIB_CMP, CMP_R_UNKNOWN_PKISTATUS);
        goto err;
    }

    /* check any present CertId in optional revCerts field */
    if (sk_OSSL_CRMF_CERTID_num(rrep->revCerts) >= 1) {
        OSSL_CRMF_CERTID *cid;
        OSSL_CRMF_CERTTEMPLATE *tmpl =
            sk_OSSL_CMP_REVDETAILS_value(rr->body->value.rr, rsid)->certDetails;
        const X509_NAME *issuer = OSSL_CRMF_CERTTEMPLATE_get0_issuer(tmpl);
        const ASN1_INTEGER *serial =
            OSSL_CRMF_CERTTEMPLATE_get0_serialNumber(tmpl);

        if (sk_OSSL_CRMF_CERTID_num(rrep->revCerts) != num_RevDetails) {
            ERR_raise(ERR_LIB_CMP, CMP_R_WRONG_RP_COMPONENT_COUNT);
            ret = 0;
            goto err;
        }
        if ((cid = ossl_cmp_revrepcontent_get_CertId(rrep, rsid)) == NULL) {
            ERR_raise(ERR_LIB_CMP, CMP_R_MISSING_CERTID);
            ret = 0;
            goto err;
        }
        if (X509_NAME_cmp(issuer, OSSL_CRMF_CERTID_get0_issuer(cid)) != 0) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            ERR_raise(ERR_LIB_CMP, CMP_R_WRONG_CERTID_IN_RP);
            ret = 0;
            goto err;
#endif
        }
        if (ASN1_INTEGER_cmp(serial,
                             OSSL_CRMF_CERTID_get0_serialNumber(cid)) != 0) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            ERR_raise(ERR_LIB_CMP, CMP_R_WRONG_SERIAL_IN_RP);
            ret = 0;
            goto err;
#endif
        }
    }

    /* check number of any optionally present crls */
    if (rrep->crls != NULL && sk_X509_CRL_num(rrep->crls) != num_RevDetails) {
        ERR_raise(ERR_LIB_CMP, CMP_R_WRONG_RP_COMPONENT_COUNT);
        ret = 0;
        goto err;
    }

 err:
    if (ret == 0
            && OSSL_CMP_CTX_snprint_PKIStatus(ctx, buf, sizeof(buf)) != NULL)
        ERR_add_error_data(1, buf);

 end:
    OSSL_CMP_MSG_free(rr);
    OSSL_CMP_MSG_free(rp);
    return ret;
}


/* { dg-final { scan-tree-dump "OSSL_CMP_exec_RR_ses\(OSSL_CMP_CTX \*ctx\)\n{" } } */
