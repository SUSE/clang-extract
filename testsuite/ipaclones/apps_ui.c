/* { dg-options "-DCE_EXTRACT_FUNCTIONS=ui_malloc -DCE_IPACLONES_PATH=$test_dir/libapps-lib-apps_ui.c.000i.ipa-clones" }*/

#define __LEAF
#define __THROW	__attribute__ ((__nothrow__ __LEAF))
#define __attribute_pure__ __attribute__ ((__pure__))
#define __attribute_nonnull__(params) __attribute__ ((__nonnull__ params))
#define __nonnull(params) __attribute_nonnull__ (params)
/** clang-extract: from /usr/lib64/clang/22/include/__stddef_size_t.h:18:1 */
typedef __SIZE_TYPE__ size_t;

/** clang-extract: from /usr/include/string.h:439:1 */
extern size_t strlen (const char *__s)
     __THROW __attribute_pure__ __nonnull ((1));

#define OPENSSL_FILE __FILE__
#define OPENSSL_LINE __LINE__
#define OSSL_CRYPTO_ALLOC __attribute__((__malloc__))
/** clang-extract: from /usr/include/stdlib.h:778:1 */
extern void exit (int __status) __THROW __attribute__ ((__noreturn__));

/** clang-extract: from include/openssl/types.h:95:1 */
typedef struct bio_st BIO;

/** clang-extract: from include/openssl/types.h:192:1 */
typedef struct ui_st UI;

/** clang-extract: from include/openssl/types.h:193:1 */
typedef struct ui_method_st UI_METHOD;

#define OPENSSL_malloc(num) \
        CRYPTO_malloc(num, OPENSSL_FILE, OPENSSL_LINE)
#define OPENSSL_clear_free(addr, num) \
        CRYPTO_clear_free(addr, num, OPENSSL_FILE, OPENSSL_LINE)
#define OPENSSL_free(addr) \
        CRYPTO_free(addr, OPENSSL_FILE, OPENSSL_LINE)
/** clang-extract: from include/openssl/crypto.h:356:1 */
OSSL_CRYPTO_ALLOC void *CRYPTO_malloc(size_t num, const char *file, int line);

/** clang-extract: from include/openssl/crypto.h:364:1 */
void CRYPTO_free(void *ptr, const char *file, int line);

/** clang-extract: from include/openssl/crypto.h:365:1 */
void CRYPTO_clear_free(void *ptr, size_t num, const char *file, int line);

/** clang-extract: from include/openssl/crypto.h:382:1 */
void OPENSSL_cleanse(void *ptr, size_t len);

#define NULL ((void*)0)
#define ossl_bio__attr__ __attribute__
#define ossl_bio__printf__ __printf__
/** clang-extract: from include/openssl/bio.h:957:1 */
int BIO_printf(BIO *bio, const char *format, ...)
ossl_bio__attr__((__format__(ossl_bio__printf__, 2, 3)));

#undef ossl_bio__attr__
#undef ossl_bio__printf__
/** clang-extract: from include/openssl/err.h:468:1 */
void ERR_print_errors(BIO *bp);

/** clang-extract: from include/openssl/ui.h:54:1 */
UI *UI_new_method(const UI_METHOD *method);

/** clang-extract: from include/openssl/ui.h:55:1 */
void UI_free(UI *ui);

/** clang-extract: from include/openssl/ui.h:100:1 */
int UI_add_input_string(UI *ui, const char *prompt, int flags,
                        char *result_buf, int minsize, int maxsize);

/** clang-extract: from include/openssl/ui.h:104:1 */
int UI_add_verify_string(UI *ui, const char *prompt, int flags,
                         char *result_buf, int minsize, int maxsize,
                         const char *test_buf);

#define UI_INPUT_FLAG_DEFAULT_PWD       0x02
/** clang-extract: from include/openssl/ui.h:164:1 */
char *UI_construct_prompt(UI *ui_method,
                          const char *phrase_desc, const char *object_name);

/** clang-extract: from include/openssl/ui.h:178:1 */
void *UI_add_user_data(UI *ui, void *user_data);

/** clang-extract: from include/openssl/ui.h:186:1 */
void *UI_get0_user_data(UI *ui);

/** clang-extract: from include/openssl/ui.h:193:1 */
int UI_process(UI *ui);

/** clang-extract: from include/openssl/ui.h:200:1 */
int UI_ctrl(UI *ui, int cmd, long i, void *p, void (*f) (void));

#define UI_CTRL_PRINT_ERRORS            1
#define UI_CTRL_IS_REDOABLE             2
/** clang-extract: from include/openssl/ui.h:242:1 */
const UI_METHOD *UI_null(void);

/** clang-extract: from include/openssl/ui.h:291:1 */
typedef struct ui_string_st UI_STRING;

/** clang-extract: from include/openssl/ui.h:325:1 */
enum UI_string_types {
    UIT_NONE = 0,
    UIT_PROMPT,                 /* Prompt for a string */
    UIT_VERIFY,                 /* Prompt for a string and verify */
    UIT_BOOLEAN,                /* Prompt for a yes/no response */
    UIT_INFO,                   /* Send info to the user */
    UIT_ERROR                   /* Send an error message to the user */
};

/** clang-extract: from include/openssl/ui.h:336:1 */
void UI_destroy_method(UI_METHOD *ui_method);

/** clang-extract: from include/openssl/ui.h:354:1 */
int (*UI_method_get_opener(const UI_METHOD *method)) (UI *);

/** clang-extract: from include/openssl/ui.h:355:1 */
int (*UI_method_get_writer(const UI_METHOD *method)) (UI *, UI_STRING *);

/** clang-extract: from include/openssl/ui.h:357:1 */
int (*UI_method_get_reader(const UI_METHOD *method)) (UI *, UI_STRING *);

/** clang-extract: from include/openssl/ui.h:358:1 */
int (*UI_method_get_closer(const UI_METHOD *method)) (UI *);

/** clang-extract: from include/openssl/ui.h:371:1 */
enum UI_string_types UI_get_string_type(UI_STRING *uis);

/** clang-extract: from include/openssl/ui.h:373:1 */
int UI_get_input_flags(UI_STRING *uis);

/** clang-extract: from include/openssl/ui.h:393:1 */
int UI_set_result(UI *ui, UI_STRING *uis, const char *result);

#define PW_MIN_LENGTH 4
/** clang-extract: from apps/include/apps_ui.h:15:1 */
typedef struct pw_cb_data {
    const void *password;
    const char *prompt_info;
} PW_CB_DATA;

/** clang-extract: from apps/include/apps_ui.h:20:1 */
int password_callback(char *buf, int bufsiz, int verify, PW_CB_DATA *cb_data);

/** clang-extract: from apps/include/apps_ui.h:23:1 */
void destroy_ui_method(void);

/** clang-extract: from apps/include/apps_ui.h:24:1 */
int set_base_ui_method(const UI_METHOD *ui_method);

/** clang-extract: from apps/include/apps_ui.h:25:1 */
const UI_METHOD *get_ui_method(void);

/** clang-extract: from apps/include/apps_ui.h:27:1 */
extern BIO *bio_err;

/** clang-extract: from apps/lib/apps_ui.c:15:1 */
static UI_METHOD *ui_method = NULL;

/** clang-extract: from apps/lib/apps_ui.c:16:1 */
static const UI_METHOD *ui_base_method = NULL;

/** clang-extract: from apps/lib/apps_ui.c:18:1 */
static int ui_open(UI *ui)
{
    int (*opener)(UI *ui) = UI_method_get_opener(ui_base_method);

    if (opener != NULL)
        return opener(ui);
    return 1;
}

/** clang-extract: from apps/lib/apps_ui.c:27:1 */
static int ui_read(UI *ui, UI_STRING *uis)
{
    int (*reader)(UI *ui, UI_STRING *uis) = NULL;

    if (UI_get_input_flags(uis) & UI_INPUT_FLAG_DEFAULT_PWD
        && UI_get0_user_data(ui)) {
        switch (UI_get_string_type(uis)) {
        case UIT_PROMPT:
        case UIT_VERIFY:
            {
                const char *password =
                    ((PW_CB_DATA *)UI_get0_user_data(ui))->password;

                if (password != NULL) {
                    UI_set_result(ui, uis, password);
                    return 1;
                }
            }
            break;
        case UIT_NONE:
        case UIT_BOOLEAN:
        case UIT_INFO:
        case UIT_ERROR:
            break;
        }
    }

    reader = UI_method_get_reader(ui_base_method);
    if (reader != NULL)
        return reader(ui, uis);
    /* Default to the empty password if we've got nothing better */
    UI_set_result(ui, uis, "");
    return 1;
}

/** clang-extract: from apps/lib/apps_ui.c:62:1 */
static int ui_write(UI *ui, UI_STRING *uis)
{
    int (*writer)(UI *ui, UI_STRING *uis) = NULL;

    if (UI_get_input_flags(uis) & UI_INPUT_FLAG_DEFAULT_PWD
        && UI_get0_user_data(ui)) {
        switch (UI_get_string_type(uis)) {
        case UIT_PROMPT:
        case UIT_VERIFY:
            {
                const char *password =
                    ((PW_CB_DATA *)UI_get0_user_data(ui))->password;

                if (password != NULL)
                    return 1;
            }
            break;
        case UIT_NONE:
        case UIT_BOOLEAN:
        case UIT_INFO:
        case UIT_ERROR:
            break;
        }
    }

    writer = UI_method_get_writer(ui_base_method);
    if (writer != NULL)
        return writer(ui, uis);
    return 1;
}

/** clang-extract: from apps/lib/apps_ui.c:93:1 */
static int ui_close(UI *ui)
{
    int (*closer)(UI *ui) = UI_method_get_closer(ui_base_method);

    if (closer != NULL)
        return closer(ui);
    return 1;
}

/** clang-extract: from apps/lib/apps_ui.c:103:1 */
static char *ui_prompt_construct(UI *ui, const char *phrase_desc,
                                 const char *object_name)
{
    PW_CB_DATA *cb_data = (PW_CB_DATA *)UI_get0_user_data(ui);

    if (phrase_desc == NULL)
        phrase_desc = "pass phrase";
    if (object_name == NULL && cb_data != NULL)
        object_name = cb_data->prompt_info;
    return UI_construct_prompt(NULL, phrase_desc, object_name);
}

/** clang-extract: from apps/lib/apps_ui.c:115:1 */
int set_base_ui_method(const UI_METHOD *ui_meth)
{
    if (ui_meth == NULL)
        ui_meth = UI_null();
    ui_base_method = ui_meth;
    return 1;
}

/** clang-extract: from apps/lib/apps_ui.c:139:1 */
void destroy_ui_method(void)
{
    if (ui_method != NULL) {
        UI_destroy_method(ui_method);
        ui_method = NULL;
    }
}

/** clang-extract: from apps/lib/apps_ui.c:147:1 */
const UI_METHOD *get_ui_method(void)
{
    return ui_method;
}

/** clang-extract: from apps/lib/apps_ui.c:152:1 */
static void *ui_malloc(int sz, const char *what)
{
    void *vp = OPENSSL_malloc(sz);

    if (vp == NULL) {
        BIO_printf(bio_err, "Could not allocate %d bytes for %s\n", sz, what);
        ERR_print_errors(bio_err);
        exit(1);
    }
    return vp;
}

/** clang-extract: from apps/lib/apps_ui.c:164:1 */
int password_callback(char *buf, int bufsiz, int verify, PW_CB_DATA *cb_data)
{
    int res = 0;
    UI *ui;
    int ok = 0;
    char *buff = NULL;
    int ui_flags = 0;
    const char *prompt_info = NULL;
    char *prompt;

    if ((ui = UI_new_method(ui_method)) == NULL)
        return 0;

    if (cb_data != NULL && cb_data->prompt_info != NULL)
        prompt_info = cb_data->prompt_info;
    prompt = UI_construct_prompt(ui, "pass phrase", prompt_info);
    if (prompt == NULL) {
        BIO_printf(bio_err, "Out of memory\n");
        UI_free(ui);
        return 0;
    }

    ui_flags |= UI_INPUT_FLAG_DEFAULT_PWD;
    UI_ctrl(ui, UI_CTRL_PRINT_ERRORS, 1, 0, 0);

    /* We know that there is no previous user data to return to us */
    (void)UI_add_user_data(ui, cb_data);

    ok = UI_add_input_string(ui, prompt, ui_flags, buf,
                             PW_MIN_LENGTH, bufsiz - 1);

    if (ok >= 0 && verify) {
        buff = ui_malloc(bufsiz, "password buffer");
        ok = UI_add_verify_string(ui, prompt, ui_flags, buff,
                                  PW_MIN_LENGTH, bufsiz - 1, buf);
    }
    if (ok >= 0)
        do {
            ok = UI_process(ui);
        } while (ok < 0 && UI_ctrl(ui, UI_CTRL_IS_REDOABLE, 0, 0, 0));

    OPENSSL_clear_free(buff, (unsigned int)bufsiz);

    if (ok >= 0)
        res = strlen(buf);
    if (ok == -1) {
        BIO_printf(bio_err, "User interface error\n");
        ERR_print_errors(bio_err);
        OPENSSL_cleanse(buf, (unsigned int)bufsiz);
        res = 0;
    }
    if (ok == -2) {
        BIO_printf(bio_err, "aborted!\n");
        OPENSSL_cleanse(buf, (unsigned int)bufsiz);
        res = 0;
    }
    UI_free(ui);
    OPENSSL_free(prompt);
    return res;
}

/* { dg-final { scan-tree-dump "password_callback\(.*\)\n{" } } */
