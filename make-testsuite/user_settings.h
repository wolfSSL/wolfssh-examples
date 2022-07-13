/* To use single-precision math, uncomment MATH_CHOICE_SP.
 * To use fast math, uncomment MATH_CHOICE_FAST.
 * To use normal math, comment both. */
#define MATH_CHOICE_SP
/*#define MATH_CHOICE_FAST*/

/* This was built on an M1 Mac mini and an AMD based Ubuntu box.
 * These are known 64-bit and have the UINT128_T available, so the
 * following is set. */
#define HAVE___UINT128_T

#if defined(MATH_CHOICE_SP)
    #define WOLFSSL_SP
    #define WOLFSSL_HAVE_SP_RSA
    #define WOLFSSL_HAVE_SP_DH
    #define WOLFSSL_HAVE_SP_ECC
    #define SP_WORD_SIZE 64
#elif defined(MATH_CHOICE_FAST)
    #define USE_FAST_MATH
#endif

#define WOLFSSL_KEY_GEN
#define NO_MD5
#define NO_DSA
#define WOLFCRYPT_ONLY
#define NO_PKCS8
#define NO_PKCS12
#define HAVE_ECC
#define TFM_TIMING_RESISTANT
#define ECC_TIMING_RESISTANT
#define WC_RSA_BLINDING
#define HAVE_AESGCM
#define HAVE_AESCCM
#define WOLFSSL_SHA384
#define WOLFSSL_SHA512

#define WOLFSSH_KEYGEN
#define HAVE_WC_ECC_SET_RNG
#define NO_MAIN_DRIVER
