/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qssl_p.h>
#include <qsslsocket_openssl_symbols_p.h>

#ifdef Q_OS_WIN
# include <qsystemlibrary_p.h>
#else
# include <qlibrary.h>
#endif

#include <qmutex.h>
#include <qmutexpool_p.h>
#include <qdatetime.h>

#if defined(Q_OS_UNIX)
#include <qdir.h>
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include <link.h>
#endif

#ifdef Q_OS_DARWIN
#include "qcore_mac_p.h"
#endif

#include <algorithm>

/*

    We load OpenSSL symbols dynamically. Because symbols are known to
    disappear, and signatures sometimes change, between releases, we need to
    be careful about how this is done. To ensure we don't end up dereferencing
    null function pointers, and continue running even if certain functions are
    missing, we define helper functions for each of the symbols we load from
    OpenSSL, all prefixed with "q_" (declared in
    qsslsocket_openssl_symbols_p.h). So instead of calling SSL_connect
    directly, we call q_SSL_connect, which is a function that checks if the
    actual SSL_connect fptr is null, and returns a failure if it is, or calls
    SSL_connect if it isn't.

    This requires a somewhat tedious process of declaring each function we
    want to call in OpenSSL thrice: once with the q_, in _p.h, once using the
    DEFINEFUNC macros below, and once in the function that actually resolves
    the symbols, below the DEFINEFUNC declarations below.

    There's one DEFINEFUNC macro declared for every number of arguments
    exposed by OpenSSL (feel free to extend when needed). The easiest thing to
    do is to find an existing entry that matches the arg count of the function
    you want to import, and do the same.

    The first macro arg is the function return type. The second is the
    verbatim name of the function/symbol. Then follows a list of N pairs of
    argument types with a variable name, and just the variable name (char *a,
    a, char *b, b, etc). Finally there's two arguments - a suitable return
    statement for the error case (for an int function, return 0 or return -1
    is usually right). Then either just "return" or DUMMYARG, the latter being
    for void functions.

    Note: Take into account that these macros and declarations are processed
    at compile-time, and the result depends on the OpenSSL headers the
    compiling host has installed, but the symbols are resolved at run-time,
    possibly with a different version of OpenSSL.
*/

#ifndef QT_LINKED_OPENSSL

namespace {

void qsslSocketCannotResolveSymbolWarning(const char *functionName)
{
   qWarning("QSslSocket: Can not resolve %s", functionName);
}

}  // namespace

#endif

#ifdef SSLEAY_MACROS
DEFINEFUNC3(void *, ASN1_dup, i2d_of_void *a, a, d2i_of_void *b, b, char *c, c, return 0, return)
#endif

DEFINEFUNC(long, ASN1_INTEGER_get, ASN1_INTEGER *a, a, return 0, return)
DEFINEFUNC(unsigned char *, ASN1_STRING_data, ASN1_STRING *a, a, return 0, return)
DEFINEFUNC(int, ASN1_STRING_length, ASN1_STRING *a, a, return 0, return)
DEFINEFUNC2(int, ASN1_STRING_to_UTF8, unsigned char **a, a, ASN1_STRING *b, b, return 0, return);
DEFINEFUNC4(long, BIO_ctrl, BIO *a, a, int b, b, long c, c, void *d, d, return -1, return)
DEFINEFUNC(int, BIO_free, BIO *a, a, return 0, return)
DEFINEFUNC(BIO *, BIO_new, BIO_METHOD *a, a, return 0, return)
DEFINEFUNC2(BIO *, BIO_new_mem_buf, void *a, a, int b, b, return 0, return)
DEFINEFUNC3(int, BIO_read, BIO *a, a, void *b, b, int c, c, return -1, return)
DEFINEFUNC(BIO_METHOD *, BIO_s_mem, void, DUMMYARG, return 0, return)
DEFINEFUNC3(int, BIO_write, BIO *a, a, const void *b, b, int c, c, return -1, return)
DEFINEFUNC(int, BN_num_bits, const BIGNUM *a, a, return 0, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC(const EC_GROUP *, EC_KEY_get0_group, const EC_KEY *k, k, return 0, return)
DEFINEFUNC(int, EC_GROUP_get_degree, const EC_GROUP *g, g, return 0, return)
#endif
DEFINEFUNC(int, CRYPTO_num_locks, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(void, CRYPTO_set_locking_callback, void (*a)(int, int, const char *, int), a, return, DUMMYARG)
DEFINEFUNC(void, CRYPTO_set_id_callback, unsigned long (*a)(), a, return, DUMMYARG)
DEFINEFUNC(void, CRYPTO_free, void *a, a, return, DUMMYARG)
DEFINEFUNC(DSA *, DSA_new, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(void, DSA_free, DSA *a, a, return, DUMMYARG)
DEFINEFUNC3(X509 *, d2i_X509, X509 **a, a, const unsigned char **b, b, long c, c, return 0, return)
DEFINEFUNC2(char *, ERR_error_string, unsigned long a, a, char *b, b, return 0, return)
DEFINEFUNC(unsigned long, ERR_get_error, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(void, ERR_free_strings, void, DUMMYARG, return, DUMMYARG)
DEFINEFUNC(void, EVP_CIPHER_CTX_cleanup, EVP_CIPHER_CTX *a, a, return, DUMMYARG)
DEFINEFUNC(void, EVP_CIPHER_CTX_init, EVP_CIPHER_CTX *a, a, return, DUMMYARG)
DEFINEFUNC4(int, EVP_CIPHER_CTX_ctrl, EVP_CIPHER_CTX *ctx, ctx, int type, type, int arg, arg, void *ptr, ptr, return 0, return);
DEFINEFUNC2(int, EVP_CIPHER_CTX_set_key_length, EVP_CIPHER_CTX *ctx, ctx, int keylen, keylen, return 0, return)
DEFINEFUNC5(int, EVP_CipherInit, EVP_CIPHER_CTX *ctx, ctx, const EVP_CIPHER *type, type, const unsigned char *key, key, const unsigned char *iv, iv, int enc, enc,
            return 0, return);
DEFINEFUNC5(int, EVP_CipherUpdate, EVP_CIPHER_CTX *ctx, ctx, unsigned char *out, out, int *outl, outl, const unsigned char *in, in, int inl, inl, return 0, return);
DEFINEFUNC3(int, EVP_CipherFinal, EVP_CIPHER_CTX *ctx, ctx, unsigned char *out, out, int *outl, outl, return 0, return);
DEFINEFUNC(const EVP_CIPHER *, EVP_des_cbc, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(const EVP_CIPHER *, EVP_des_ede3_cbc, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(const EVP_CIPHER *, EVP_rc2_cbc, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC3(int, EVP_PKEY_assign, EVP_PKEY *a, a, int b, b, char *c, c, return -1, return)
DEFINEFUNC2(int, EVP_PKEY_set1_RSA, EVP_PKEY *a, a, RSA *b, b, return -1, return)
DEFINEFUNC2(int, EVP_PKEY_set1_DSA, EVP_PKEY *a, a, DSA *b, b, return -1, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC2(int, EVP_PKEY_set1_EC_KEY, EVP_PKEY *a, a, EC_KEY *b, b, return -1, return)
#endif
DEFINEFUNC(void, EVP_PKEY_free, EVP_PKEY *a, a, return, DUMMYARG)
DEFINEFUNC(DSA *, EVP_PKEY_get1_DSA, EVP_PKEY *a, a, return 0, return)
DEFINEFUNC(RSA *, EVP_PKEY_get1_RSA, EVP_PKEY *a, a, return 0, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC(EC_KEY *, EVP_PKEY_get1_EC_KEY, EVP_PKEY *a, a, return 0, return)
#endif
DEFINEFUNC(EVP_PKEY *, EVP_PKEY_new, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(int, EVP_PKEY_type, int a, a, return NID_undef, return)
DEFINEFUNC2(int, i2d_X509, X509 *a, a, unsigned char **b, b, return -1, return)
DEFINEFUNC(const char *, OBJ_nid2sn, int a, a, return 0, return)
DEFINEFUNC(const char *, OBJ_nid2ln, int a, a, return 0, return)
DEFINEFUNC(int, OBJ_sn2nid, const char *s, s, return 0, return)
DEFINEFUNC(int, OBJ_ln2nid, const char *s, s, return 0, return)
DEFINEFUNC3(int, i2t_ASN1_OBJECT, char *a, a, int b, b, ASN1_OBJECT *c, c, return -1, return)
DEFINEFUNC4(int, OBJ_obj2txt, char *a, a, int b, b, ASN1_OBJECT *c, c, int d, d, return -1, return)

DEFINEFUNC(int, OBJ_obj2nid, const ASN1_OBJECT *a, a, return NID_undef, return)
#ifdef SSLEAY_MACROS
DEFINEFUNC6(void *, PEM_ASN1_read_bio, d2i_of_void *a, a, const char *b, b, BIO *c, c, void **d, d, pem_password_cb *e, e, void *f, f, return 0, return)
DEFINEFUNC6(void *, PEM_ASN1_write_bio, d2i_of_void *a, a, const char *b, b, BIO *c, c, void **d, d, pem_password_cb *e, e, void *f, f, return 0, return)
#else
DEFINEFUNC4(DSA *, PEM_read_bio_DSAPrivateKey, BIO *a, a, DSA **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
DEFINEFUNC4(RSA *, PEM_read_bio_RSAPrivateKey, BIO *a, a, RSA **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC4(EC_KEY *, PEM_read_bio_ECPrivateKey, BIO *a, a, EC_KEY **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
#endif
DEFINEFUNC7(int, PEM_write_bio_DSAPrivateKey, BIO *a, a, DSA *b, b, const EVP_CIPHER *c, c, unsigned char *d, d, int e, e, pem_password_cb *f, f, void *g, g,
            return 0, return)
DEFINEFUNC7(int, PEM_write_bio_RSAPrivateKey, BIO *a, a, RSA *b, b, const EVP_CIPHER *c, c, unsigned char *d, d, int e, e, pem_password_cb *f, f, void *g, g,
            return 0, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC7(int, PEM_write_bio_ECPrivateKey, BIO *a, a, EC_KEY *b, b, const EVP_CIPHER *c, c, unsigned char *d, d, int e, e, pem_password_cb *f, f, void *g, g,
            return 0, return)
#endif
#endif
DEFINEFUNC4(DSA *, PEM_read_bio_DSA_PUBKEY, BIO *a, a, DSA **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
DEFINEFUNC4(RSA *, PEM_read_bio_RSA_PUBKEY, BIO *a, a, RSA **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC4(EC_KEY *, PEM_read_bio_EC_PUBKEY, BIO *a, a, EC_KEY **b, b, pem_password_cb *c, c, void *d, d, return 0, return)
#endif
DEFINEFUNC2(int, PEM_write_bio_DSA_PUBKEY, BIO *a, a, DSA *b, b, return 0, return)
DEFINEFUNC2(int, PEM_write_bio_RSA_PUBKEY, BIO *a, a, RSA *b, b, return 0, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC2(int, PEM_write_bio_EC_PUBKEY, BIO *a, a, EC_KEY *b, b, return 0, return)
#endif
DEFINEFUNC2(void, RAND_seed, const void *a, a, int b, b, return, DUMMYARG)
DEFINEFUNC(int, RAND_status, void, DUMMYARG, return -1, return)
DEFINEFUNC(RSA *, RSA_new, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(void, RSA_free, RSA *a, a, return, DUMMYARG)
DEFINEFUNC(int, sk_num, STACK *a, a, return -1, return)
DEFINEFUNC2(void, sk_pop_free, STACK *a, a, void (*b)(void *), b, return, DUMMYARG)
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
DEFINEFUNC(_STACK *, sk_new_null, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC2(void, sk_push, _STACK *a, a, void *b, b, return, DUMMYARG)
DEFINEFUNC(void, sk_free, _STACK *a, a, return, DUMMYARG)
DEFINEFUNC2(void *, sk_value, STACK *a, a, int b, b, return 0, return)
#else
DEFINEFUNC(STACK *, sk_new_null, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC2(void, sk_push, STACK *a, a, char *b, b, return, DUMMYARG)
DEFINEFUNC(void, sk_free, STACK *a, a, return, DUMMYARG)
DEFINEFUNC2(char *, sk_value, STACK *a, a, int b, b, return 0, return)
#endif
DEFINEFUNC(int, SSL_accept, SSL *a, a, return -1, return)
DEFINEFUNC(int, SSL_clear, SSL *a, a, return -1, return)
DEFINEFUNC3(char *, SSL_CIPHER_description, SSL_CIPHER *a, a, char *b, b, int c, c, return 0, return)
DEFINEFUNC2(int, SSL_CIPHER_get_bits, SSL_CIPHER *a, a, int *b, b, return 0, return)
DEFINEFUNC(int, SSL_connect, SSL *a, a, return -1, return)
DEFINEFUNC(int, SSL_CTX_check_private_key, const SSL_CTX *a, a, return -1, return)
DEFINEFUNC4(long, SSL_CTX_ctrl, SSL_CTX *a, a, int b, b, long c, c, void *d, d, return -1, return)
DEFINEFUNC(void, SSL_CTX_free, SSL_CTX *a, a, return, DUMMYARG)
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
DEFINEFUNC(SSL_CTX *, SSL_CTX_new, const SSL_METHOD *a, a, return 0, return)
#else
DEFINEFUNC(SSL_CTX *, SSL_CTX_new, SSL_METHOD *a, a, return 0, return)
#endif
DEFINEFUNC2(int, SSL_CTX_set_cipher_list, SSL_CTX *a, a, const char *b, b, return -1, return)
DEFINEFUNC(int, SSL_CTX_set_default_verify_paths, SSL_CTX *a, a, return -1, return)
DEFINEFUNC3(void, SSL_CTX_set_verify, SSL_CTX *a, a, int b, b, int (*c)(int, X509_STORE_CTX *), c, return, DUMMYARG)
DEFINEFUNC2(void, SSL_CTX_set_verify_depth, SSL_CTX *a, a, int b, b, return, DUMMYARG)
DEFINEFUNC2(int, SSL_CTX_use_certificate, SSL_CTX *a, a, X509 *b, b, return -1, return)
DEFINEFUNC3(int, SSL_CTX_use_certificate_file, SSL_CTX *a, a, const char *b, b, int c, c, return -1, return)
DEFINEFUNC2(int, SSL_CTX_use_PrivateKey, SSL_CTX *a, a, EVP_PKEY *b, b, return -1, return)
DEFINEFUNC2(int, SSL_CTX_use_RSAPrivateKey, SSL_CTX *a, a, RSA *b, b, return -1, return)
DEFINEFUNC3(int, SSL_CTX_use_PrivateKey_file, SSL_CTX *a, a, const char *b, b, int c, c, return -1, return)
DEFINEFUNC(X509_STORE *, SSL_CTX_get_cert_store, const SSL_CTX *a, a, return 0, return)
DEFINEFUNC(void, SSL_free, SSL *a, a, return, DUMMYARG)
DEFINEFUNC(STACK_OF(SSL_CIPHER) *, SSL_get_ciphers, const SSL *a, a, return 0, return)
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
DEFINEFUNC(const SSL_CIPHER *, SSL_get_current_cipher, SSL *a, a, return 0, return)
#else
DEFINEFUNC(SSL_CIPHER *, SSL_get_current_cipher, SSL *a, a, return 0, return)
#endif
DEFINEFUNC(int, SSL_version, const SSL *a, a, return 0, return)
DEFINEFUNC2(int, SSL_get_error, SSL *a, a, int b, b, return -1, return)
DEFINEFUNC(STACK_OF(X509) *, SSL_get_peer_cert_chain, SSL *a, a, return 0, return)
DEFINEFUNC(X509 *, SSL_get_peer_certificate, SSL *a, a, return 0, return)
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
// 0.9.8 broke SC and BC by changing this function's signature.
DEFINEFUNC(long, SSL_get_verify_result, const SSL *a, a, return -1, return)
#else
DEFINEFUNC(long, SSL_get_verify_result, SSL *a, a, return -1, return)
#endif
DEFINEFUNC(int, SSL_library_init, void, DUMMYARG, return -1, return)
DEFINEFUNC(void, SSL_load_error_strings, void, DUMMYARG, return, DUMMYARG)
DEFINEFUNC(SSL *, SSL_new, SSL_CTX *a, a, return 0, return)
DEFINEFUNC4(long, SSL_ctrl, SSL *a, a, int cmd, cmd, long larg, larg, void *parg, parg, return -1, return)
DEFINEFUNC3(int, SSL_read, SSL *a, a, void *b, b, int c, c, return -1, return)
DEFINEFUNC3(void, SSL_set_bio, SSL *a, a, BIO *b, b, BIO *c, c, return, DUMMYARG)
DEFINEFUNC(void, SSL_set_accept_state, SSL *a, a, return, DUMMYARG)
DEFINEFUNC(void, SSL_set_connect_state, SSL *a, a, return, DUMMYARG)
DEFINEFUNC(int, SSL_shutdown, SSL *a, a, return -1, return)
DEFINEFUNC2(int, SSL_set_session, SSL *to, to, SSL_SESSION *session, session, return -1, return)
DEFINEFUNC(void, SSL_SESSION_free, SSL_SESSION *ses, ses, return, DUMMYARG)
DEFINEFUNC(SSL_SESSION *, SSL_get1_session, SSL *ssl, ssl, return 0, return)
DEFINEFUNC(SSL_SESSION *, SSL_get_session, const SSL *ssl, ssl, return 0, return)
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
DEFINEFUNC5(int, SSL_get_ex_new_index, long argl, argl, void *argp, argp, CRYPTO_EX_new *new_func, new_func, CRYPTO_EX_dup *dup_func, dup_func,
            CRYPTO_EX_free *free_func, free_func, return -1, return)
DEFINEFUNC3(int, SSL_set_ex_data, SSL *ssl, ssl, int idx, idx, void *arg, arg, return 0, return)
DEFINEFUNC2(void *, SSL_get_ex_data, const SSL *ssl, ssl, int idx, idx, return NULL, return)
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10001000L && !defined(OPENSSL_NO_PSK)
DEFINEFUNC2(void, SSL_set_psk_client_callback, SSL *ssl, ssl, q_psk_client_callback_t callback, callback, return, DUMMYARG)
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
#ifndef OPENSSL_NO_SSL2
DEFINEFUNC(const SSL_METHOD *, SSLv2_client_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
#ifndef OPENSSL_NO_SSL3_METHOD
DEFINEFUNC(const SSL_METHOD *, SSLv3_client_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
DEFINEFUNC(const SSL_METHOD *, SSLv23_client_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(const SSL_METHOD *, TLSv1_client_method, DUMMYARG, DUMMYARG, return 0, return)
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
DEFINEFUNC(const SSL_METHOD *, TLSv1_1_client_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(const SSL_METHOD *, TLSv1_2_client_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
#ifndef OPENSSL_NO_SSL2
DEFINEFUNC(const SSL_METHOD *, SSLv2_server_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
#ifndef OPENSSL_NO_SSL3_METHOD
DEFINEFUNC(const SSL_METHOD *, SSLv3_server_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
DEFINEFUNC(const SSL_METHOD *, SSLv23_server_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(const SSL_METHOD *, TLSv1_server_method, DUMMYARG, DUMMYARG, return 0, return)
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
DEFINEFUNC(const SSL_METHOD *, TLSv1_1_server_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(const SSL_METHOD *, TLSv1_2_server_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
#else
#ifndef OPENSSL_NO_SSL2
DEFINEFUNC(SSL_METHOD *, SSLv2_client_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
#ifndef OPENSSL_NO_SSL3_METHOD
DEFINEFUNC(SSL_METHOD *, SSLv3_client_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
DEFINEFUNC(SSL_METHOD *, SSLv23_client_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, TLSv1_client_method, DUMMYARG, DUMMYARG, return 0, return)
#ifndef OPENSSL_NO_SSL2
DEFINEFUNC(SSL_METHOD *, SSLv2_server_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
#ifndef OPENSSL_NO_SSL3_METHOD
DEFINEFUNC(SSL_METHOD *, SSLv3_server_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
DEFINEFUNC(SSL_METHOD *, SSLv23_server_method, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(SSL_METHOD *, TLSv1_server_method, DUMMYARG, DUMMYARG, return 0, return)
#endif
DEFINEFUNC3(int, SSL_write, SSL *a, a, const void *b, b, int c, c, return -1, return)
DEFINEFUNC2(int, X509_cmp, X509 *a, a, X509 *b, b, return -1, return)
#ifndef SSLEAY_MACROS
DEFINEFUNC(X509 *, X509_dup, X509 *a, a, return 0, return)
#endif
DEFINEFUNC2(void, X509_print, BIO *a, a, X509 *b, b, return, DUMMYARG);
DEFINEFUNC(ASN1_OBJECT *, X509_EXTENSION_get_object, X509_EXTENSION *a, a, return 0, return)
DEFINEFUNC(void, X509_free, X509 *a, a, return, DUMMYARG)
DEFINEFUNC2(X509_EXTENSION *, X509_get_ext, X509 *a, a, int b, b, return 0, return)
DEFINEFUNC(int, X509_get_ext_count, X509 *a, a, return 0, return)
DEFINEFUNC4(void *, X509_get_ext_d2i, X509 *a, a, int b, b, int *c, c, int *d, d, return 0, return)
DEFINEFUNC(const X509V3_EXT_METHOD *, X509V3_EXT_get, X509_EXTENSION *a, a, return 0, return)
DEFINEFUNC(void *, X509V3_EXT_d2i, X509_EXTENSION *a, a, return 0, return)
DEFINEFUNC(int, X509_EXTENSION_get_critical, X509_EXTENSION *a, a, return 0, return)
DEFINEFUNC(ASN1_OCTET_STRING *, X509_EXTENSION_get_data, X509_EXTENSION *a, a, return 0, return)
DEFINEFUNC(void, BASIC_CONSTRAINTS_free, BASIC_CONSTRAINTS *a, a, return, DUMMYARG)
DEFINEFUNC(void, AUTHORITY_KEYID_free, AUTHORITY_KEYID *a, a, return, DUMMYARG)
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
DEFINEFUNC2(int, ASN1_STRING_print, BIO *a, a, const ASN1_STRING *b, b, return 0, return)
#else
DEFINEFUNC2(int, ASN1_STRING_print, BIO *a, a, ASN1_STRING *b, b, return 0, return)
#endif
DEFINEFUNC2(int, X509_check_issued, X509 *a, a, X509 *b, b, return -1, return)
DEFINEFUNC(X509_NAME *, X509_get_issuer_name, X509 *a, a, return 0, return)
DEFINEFUNC(X509_NAME *, X509_get_subject_name, X509 *a, a, return 0, return)
DEFINEFUNC(int, X509_verify_cert, X509_STORE_CTX *a, a, return -1, return)
DEFINEFUNC(int, X509_NAME_entry_count, X509_NAME *a, a, return 0, return)
DEFINEFUNC2(X509_NAME_ENTRY *, X509_NAME_get_entry, X509_NAME *a, a, int b, b, return 0, return)
DEFINEFUNC(ASN1_STRING *, X509_NAME_ENTRY_get_data, X509_NAME_ENTRY *a, a, return 0, return)
DEFINEFUNC(ASN1_OBJECT *, X509_NAME_ENTRY_get_object, X509_NAME_ENTRY *a, a, return 0, return)
DEFINEFUNC(EVP_PKEY *, X509_PUBKEY_get, X509_PUBKEY *a, a, return 0, return)
DEFINEFUNC(void, X509_STORE_free, X509_STORE *a, a, return, DUMMYARG)
DEFINEFUNC(X509_STORE *, X509_STORE_new, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC2(int, X509_STORE_add_cert, X509_STORE *a, a, X509 *b, b, return 0, return)
DEFINEFUNC(void, X509_STORE_CTX_free, X509_STORE_CTX *a, a, return, DUMMYARG)
DEFINEFUNC4(int, X509_STORE_CTX_init, X509_STORE_CTX *a, a, X509_STORE *b, b, X509 *c, c, STACK_OF(X509) * d, d, return -1, return)
DEFINEFUNC2(int, X509_STORE_CTX_set_purpose, X509_STORE_CTX *a, a, int b, b, return -1, return)
DEFINEFUNC(int, X509_STORE_CTX_get_error, X509_STORE_CTX *a, a, return -1, return)
DEFINEFUNC(int, X509_STORE_CTX_get_error_depth, X509_STORE_CTX *a, a, return -1, return)
DEFINEFUNC(X509 *, X509_STORE_CTX_get_current_cert, X509_STORE_CTX *a, a, return 0, return)
DEFINEFUNC(STACK_OF(X509) *, X509_STORE_CTX_get_chain, X509_STORE_CTX *a, a, return 0, return)
DEFINEFUNC(X509_STORE_CTX *, X509_STORE_CTX_new, DUMMYARG, DUMMYARG, return 0, return)
#ifdef SSLEAY_MACROS
DEFINEFUNC2(int, i2d_DSAPrivateKey, const DSA *a, a, unsigned char **b, b, return -1, return)
DEFINEFUNC2(int, i2d_RSAPrivateKey, const RSA *a, a, unsigned char **b, b, return -1, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC2(int, i2d_ECPrivateKey, const EC_KEY *a, a, unsigned char **b, b, return -1, return)
#endif
DEFINEFUNC3(RSA *, d2i_RSAPrivateKey, RSA **a, a, unsigned char **b, b, long c, c, return 0, return)
DEFINEFUNC3(DSA *, d2i_DSAPrivateKey, DSA **a, a, unsigned char **b, b, long c, c, return 0, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC3(EC_KEY *, d2i_ECPrivateKey, EC_KEY **a, a, unsigned char **b, b, long c, c, return 0, return)
#endif
#endif
DEFINEFUNC(void, OPENSSL_add_all_algorithms_noconf, void, DUMMYARG, return, DUMMYARG)
DEFINEFUNC(void, OPENSSL_add_all_algorithms_conf, void, DUMMYARG, return, DUMMYARG)
DEFINEFUNC3(int, SSL_CTX_load_verify_locations, SSL_CTX *ctx, ctx, const char *CAfile, CAfile, const char *CApath, CApath, return 0, return)
DEFINEFUNC(long, SSLeay, void, DUMMYARG, return 0, return)
DEFINEFUNC(const char *, SSLeay_version, int a, a, return 0, return)
DEFINEFUNC2(int, i2d_SSL_SESSION, SSL_SESSION *in, in, unsigned char **pp, pp, return 0, return)
DEFINEFUNC3(SSL_SESSION *, d2i_SSL_SESSION, SSL_SESSION **a, a, const unsigned char **pp, pp, long length, length, return 0, return)
#if OPENSSL_VERSION_NUMBER >= 0x1000100fL && !defined(OPENSSL_NO_NEXTPROTONEG)
DEFINEFUNC6(int, SSL_select_next_proto, unsigned char **out, out, unsigned char *outlen, outlen,
            const unsigned char *in, in, unsigned int inlen, inlen,
            const unsigned char *client, client, unsigned int client_len, client_len,
            return -1, return)
DEFINEFUNC3(void, SSL_CTX_set_next_proto_select_cb, SSL_CTX *s, s,
            int (*cb) (SSL *ssl, unsigned char **out,
                       unsigned char *outlen,
                       const unsigned char *in,
                       unsigned int inlen, void *arg), cb,
            void *arg, arg, return, DUMMYARG)
DEFINEFUNC3(void, SSL_get0_next_proto_negotiated, const SSL *s, s,
            const unsigned char **data, data, unsigned *len, len, return, DUMMYARG)
#endif // OPENSSL_VERSION_NUMBER >= 0x1000100fL ...
DEFINEFUNC(DH *, DH_new, DUMMYARG, DUMMYARG, return 0, return)
DEFINEFUNC(void, DH_free, DH *dh, dh, return, DUMMYARG)
DEFINEFUNC3(DH *, d2i_DHparams, DH **a, a, const unsigned char **pp, pp, long length, length, return 0, return)
DEFINEFUNC3(BIGNUM *, BN_bin2bn, const unsigned char *s, s, int len, len, BIGNUM *ret, ret, return 0, return)
#ifndef OPENSSL_NO_EC
DEFINEFUNC(EC_KEY *, EC_KEY_dup, const EC_KEY *ec, ec, return 0, return)
DEFINEFUNC(EC_KEY *, EC_KEY_new_by_curve_name, int nid, nid, return 0, return)
DEFINEFUNC(void, EC_KEY_free, EC_KEY *ecdh, ecdh, return, DUMMYARG)
DEFINEFUNC2(size_t, EC_get_builtin_curves, EC_builtin_curve *r, r, size_t nitems, nitems, return 0, return)
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
DEFINEFUNC(int, EC_curve_nist2nid, const char *name, name, return 0, return)
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L
#endif // OPENSSL_NO_EC

DEFINEFUNC5(int, PKCS12_parse, PKCS12 *p12, p12, const char *pass, pass, EVP_PKEY **pkey, pkey, \
            X509 **cert, cert, STACK_OF(X509) **ca, ca, return 1, return);
DEFINEFUNC2(PKCS12 *, d2i_PKCS12_bio, BIO *bio, bio, PKCS12 **pkcs12, pkcs12, return 0, return);
DEFINEFUNC(void, PKCS12_free, PKCS12 *pkcs12, pkcs12, return, DUMMYARG)

#define RESOLVEFUNC(func) \
    if (!(_q_##func = _q_PTR_##func(libs.first->resolve(#func)))     \
        && !(_q_##func = _q_PTR_##func(libs.second->resolve(#func)))) \
        qsslSocketCannotResolveSymbolWarning(#func);

#if ! defined QT_LINKED_OPENSSL


#ifdef Q_OS_UNIX
struct NumericallyLess {
   typedef bool result_type;
   result_type operator()(const QString &lhs, const QString &rhs) const {
      bool ok = false;
      int b = 0;
      int a = lhs.toInteger<int>(&ok);

      if (ok) {
         b = rhs.toInteger<int>(&ok);
      }

      if (ok) {
         // both toInt succeeded
         return a < b;

      } else {
         // compare as strings;
         return lhs < rhs;
      }
   }
};

struct LibGreaterThan {
   typedef bool result_type;

   result_type operator()(const QString &lhs, const QString &rhs) const {
      const QStringList lhsparts = lhs.split(QLatin1Char('.'));
      const QStringList rhsparts = rhs.split(QLatin1Char('.'));

      Q_ASSERT(lhsparts.count() > 1 && rhsparts.count() > 1);

      // note: checking rhs < lhs, the same as lhs > rhs
      return std::lexicographical_compare(rhsparts.begin() + 1, rhsparts.end(),
                                          lhsparts.begin() + 1, lhsparts.end(), NumericallyLess());
   }
};

#if defined(Q_OS_LINUX)
static int dlIterateCallback(struct dl_phdr_info *info, size_t size, void *data)
{
   if (size < sizeof (info->dlpi_addr) + sizeof (info->dlpi_name)) {
      return 1;
   }

   QSet<QString> *paths = (QSet<QString> *)data;
   QString path = QString::fromUtf8(info->dlpi_name);

   if (! path.isEmpty()) {

      QFileInfo fi(path);
      path = fi.absolutePath();

      if (!path.isEmpty()) {
         paths->insert(path);
      }
   }

   return 0;
}
#endif

static QStringList libraryPathList()
{
   QStringList paths;

#  ifdef Q_OS_DARWIN
   paths = QString::fromLatin1(qgetenv("DYLD_LIBRARY_PATH")).split(':', QStringParser::SkipEmptyParts);

   // search in .app/Contents/Frameworks
   UInt32 packageType;
   CFBundleGetPackageInfo(CFBundleGetMainBundle(), &packageType, NULL);

   if (packageType == FOUR_CHAR_CODE('APPL')) {
      QUrl bundleUrl     = QUrl::fromCFURL(QCFType<CFURLRef>(CFBundleCopyBundleURL(CFBundleGetMainBundle())));
      QUrl frameworksUrl = QUrl::fromCFURL(QCFType<CFURLRef>(CFBundleCopyPrivateFrameworksURL(CFBundleGetMainBundle())));
      paths << bundleUrl.resolved(frameworksUrl).path();
   }

#  else
   paths = QString::fromLatin1(qgetenv("LD_LIBRARY_PATH")).split(':', QStringParser::SkipEmptyParts);

#  endif
   paths << QLatin1String("/lib")   << QLatin1String("/usr/lib")   << QLatin1String("/usr/local/lib");
   paths << QLatin1String("/lib64") << QLatin1String("/usr/lib64") << QLatin1String("/usr/local/lib64");
   paths << QLatin1String("/lib32") << QLatin1String("/usr/lib32") << QLatin1String("/usr/local/lib32");

#if defined(Q_OS_ANDROID)
   paths << QLatin1String("/system/lib");

#elif defined(Q_OS_LINUX)
   // discover paths of already loaded libraries
   QSet<QString> loadedPaths;
   dl_iterate_phdr(dlIterateCallback, &loadedPaths);
   paths.append(loadedPaths.toList());

#endif

   return paths;
}

static QStringList findAllLibs(QLatin1String filter)
{
   QStringList paths = libraryPathList();
   QStringList found;
   const QStringList filters((QString(filter)));

   for (const QString &path : paths) {
      QDir dir(path);
      QStringList entryList = dir.entryList(filters, QDir::Files);

      std::sort(entryList.begin(), entryList.end(), LibGreaterThan());
      for (const QString &entry : entryList) {
         found << path + QLatin1Char('/') + entry;
      }
   }

   return found;
}

static QStringList findAllLibSsl()
{
   return findAllLibs(QLatin1String("libssl.*"));
}

static QStringList findAllLibCrypto()
{
   return findAllLibs(QLatin1String("libcrypto.*"));
}
# endif

#ifdef Q_OS_WIN
static bool tryToLoadOpenSslWin32Library(QLatin1String ssleay32LibName, QLatin1String libeay32LibName, QPair<QSystemLibrary *, QSystemLibrary *> &pair)
{
   pair.first = 0;
   pair.second = 0;

   QSystemLibrary *ssleay32 = new QSystemLibrary(ssleay32LibName);
   if (!ssleay32->load(false)) {
      delete ssleay32;
      return FALSE;
   }

   QSystemLibrary *libeay32 = new QSystemLibrary(libeay32LibName);
   if (!libeay32->load(false)) {
      delete ssleay32;
      delete libeay32;
      return FALSE;
   }

   pair.first = ssleay32;
   pair.second = libeay32;
   return TRUE;
}

static QPair<QSystemLibrary *, QSystemLibrary *> loadOpenSslWin32()
{
   QPair<QSystemLibrary *, QSystemLibrary *> pair;
   pair.first = 0;
   pair.second = 0;

   // When OpenSSL is built using MSVC then the libraries are named 'ssleay32.dll' and 'libeay32'dll'.
   // When OpenSSL is built using GCC then different library names are used (depending on the OpenSSL version)
   // The oldest version of a GCC-based OpenSSL which can be detected by the code below is 0.9.8g (released in 2007)
   if (!tryToLoadOpenSslWin32Library(QLatin1String("ssleay32"), QLatin1String("libeay32"), pair)) {
      if (!tryToLoadOpenSslWin32Library(QLatin1String("libssl-10"), QLatin1String("libcrypto-10"), pair)) {
         if (!tryToLoadOpenSslWin32Library(QLatin1String("libssl-8"), QLatin1String("libcrypto-8"), pair)) {
            tryToLoadOpenSslWin32Library(QLatin1String("libssl-7"), QLatin1String("libcrypto-7"), pair);
         }
      }
   }

   return pair;
}
#else

static QPair<QLibrary *, QLibrary *> loadOpenSsl()
{
   QPair<QLibrary *, QLibrary *> pair;

# if defined(Q_OS_UNIX)
   QLibrary *&libssl = pair.first;
   QLibrary *&libcrypto = pair.second;
   libssl = new QLibrary;
   libcrypto = new QLibrary;

   // find the libssl library on the system.

#ifdef Q_OS_OPENBSD
   libcrypto->setLoadHints(QLibrary::ExportExternalSymbolsHint);
#endif

#if defined(SHLIB_VERSION_NUMBER) && !defined(Q_OS_QNX) // on QNX, the libs are always libssl.so and libcrypto.so
   // first attempt: the canonical name is libssl.so.<SHLIB_VERSION_NUMBER>
   libssl->setFileNameAndVersion(QLatin1String("ssl"), QLatin1String(SHLIB_VERSION_NUMBER));
   libcrypto->setFileNameAndVersion(QLatin1String("crypto"), QLatin1String(SHLIB_VERSION_NUMBER));

   if (libcrypto->load() && libssl->load()) {
      // libssl.so.<SHLIB_VERSION_NUMBER> and libcrypto.so.<SHLIB_VERSION_NUMBER> found
      return pair;
   } else {
      libssl->unload();
      libcrypto->unload();
   }
#endif

#ifndef Q_OS_DARWIN
   // second attempt: find the development files libssl.so and libcrypto.so
   //
   // disabled on OS X/iOS:
   //  OS X's /usr/lib/libssl.dylib, /usr/lib/libcrypto.dylib will be picked up in the third
   //    attempt, _after_ <bundle>/Contents/Frameworks has been searched.
   //  iOS does not ship a system libssl.dylib, libcrypto.dylib in the first place.
   libssl->setFileNameAndVersion(QLatin1String("ssl"), -1);
   libcrypto->setFileNameAndVersion(QLatin1String("crypto"), -1);
   if (libcrypto->load() && libssl->load()) {
      // libssl.so.0 and libcrypto.so.0 found
      return pair;
   } else {
      libssl->unload();
      libcrypto->unload();
   }
#endif

   // third attempt: loop on the most common library paths and find libssl
   QStringList sslList = findAllLibSsl();
   QStringList cryptoList = findAllLibCrypto();

   for (const QString &crypto : cryptoList) {
      libcrypto->setFileNameAndVersion(crypto, -1);

      if (libcrypto->load()) {
         QFileInfo fi(crypto);
         QString version = fi.completeSuffix();

         for (const QString &ssl : sslList) {
            if (!ssl.endsWith(version)) {
               continue;
            }

            libssl->setFileNameAndVersion(ssl, -1);

            if (libssl->load()) {
               // libssl.so.x and libcrypto.so.x found
               return pair;
            } else {
               libssl->unload();
            }
         }
      }
      libcrypto->unload();
   }

   // failed to load anything
   delete libssl;
   delete libcrypto;
   libssl = libcrypto = 0;
   return pair;

# else
   // not implemented for this platform yet
   return pair;
# endif
}
#endif

bool q_resolveOpenSslSymbols()
{
   static bool symbolsResolved = false;
   static bool triedToResolveSymbols = false;
#ifndef QT_NO_THREAD
   QMutexLocker locker(QMutexPool::globalInstanceGet((void *)&q_SSL_library_init));
#endif
   if (symbolsResolved) {
      return true;
   }
   if (triedToResolveSymbols) {
      return false;
   }
   triedToResolveSymbols = true;

#ifdef Q_OS_WIN
   QPair<QSystemLibrary *, QSystemLibrary *> libs = loadOpenSslWin32();
#else
   QPair<QLibrary *, QLibrary *> libs = loadOpenSsl();
#endif
   if (!libs.first || !libs.second)
      // failed to load them
   {
      return false;
   }

#ifdef SSLEAY_MACROS
   RESOLVEFUNC(ASN1_dup)
#endif
   RESOLVEFUNC(ASN1_INTEGER_get)
   RESOLVEFUNC(ASN1_STRING_data)
   RESOLVEFUNC(ASN1_STRING_length)
   RESOLVEFUNC(ASN1_STRING_to_UTF8)
   RESOLVEFUNC(BIO_ctrl)
   RESOLVEFUNC(BIO_free)
   RESOLVEFUNC(BIO_new)
   RESOLVEFUNC(BIO_new_mem_buf)
   RESOLVEFUNC(BIO_read)
   RESOLVEFUNC(BIO_s_mem)
   RESOLVEFUNC(BIO_write)
#ifndef OPENSSL_NO_EC
   RESOLVEFUNC(EC_KEY_get0_group)
   RESOLVEFUNC(EC_GROUP_get_degree)
#endif
   RESOLVEFUNC(BN_num_bits)
   RESOLVEFUNC(CRYPTO_free)
   RESOLVEFUNC(CRYPTO_num_locks)
   RESOLVEFUNC(CRYPTO_set_id_callback)
   RESOLVEFUNC(CRYPTO_set_locking_callback)
   RESOLVEFUNC(DSA_new)
   RESOLVEFUNC(DSA_free)
   RESOLVEFUNC(ERR_error_string)
   RESOLVEFUNC(ERR_get_error)
   RESOLVEFUNC(ERR_free_strings)
   RESOLVEFUNC(EVP_CIPHER_CTX_cleanup)
   RESOLVEFUNC(EVP_CIPHER_CTX_init)
   RESOLVEFUNC(EVP_CIPHER_CTX_ctrl)
   RESOLVEFUNC(EVP_CIPHER_CTX_set_key_length)
   RESOLVEFUNC(EVP_CipherInit)
   RESOLVEFUNC(EVP_CipherUpdate)
   RESOLVEFUNC(EVP_CipherFinal)
   RESOLVEFUNC(EVP_des_cbc)
   RESOLVEFUNC(EVP_des_ede3_cbc)
   RESOLVEFUNC(EVP_rc2_cbc)
   RESOLVEFUNC(EVP_PKEY_assign)
   RESOLVEFUNC(EVP_PKEY_set1_RSA)
   RESOLVEFUNC(EVP_PKEY_set1_DSA)
#ifndef OPENSSL_NO_EC
   RESOLVEFUNC(EVP_PKEY_set1_EC_KEY)
#endif
   RESOLVEFUNC(EVP_PKEY_free)
   RESOLVEFUNC(EVP_PKEY_get1_DSA)
   RESOLVEFUNC(EVP_PKEY_get1_RSA)
#ifndef OPENSSL_NO_EC
   RESOLVEFUNC(EVP_PKEY_get1_EC_KEY)
#endif
   RESOLVEFUNC(EVP_PKEY_new)
   RESOLVEFUNC(EVP_PKEY_type)
   RESOLVEFUNC(OBJ_nid2sn)
   RESOLVEFUNC(OBJ_nid2ln)
   RESOLVEFUNC(OBJ_sn2nid)
   RESOLVEFUNC(OBJ_ln2nid)
   RESOLVEFUNC(i2t_ASN1_OBJECT)
   RESOLVEFUNC(OBJ_obj2txt)
   RESOLVEFUNC(OBJ_obj2nid)
#ifdef SSLEAY_MACROS // ### verify
   RESOLVEFUNC(PEM_ASN1_read_bio)
#else
   RESOLVEFUNC(PEM_read_bio_DSAPrivateKey)
   RESOLVEFUNC(PEM_read_bio_RSAPrivateKey)
#ifndef OPENSSL_NO_EC
   RESOLVEFUNC(PEM_read_bio_ECPrivateKey)
#endif
   RESOLVEFUNC(PEM_write_bio_DSAPrivateKey)
   RESOLVEFUNC(PEM_write_bio_RSAPrivateKey)
#ifndef OPENSSL_NO_EC
   RESOLVEFUNC(PEM_write_bio_ECPrivateKey)
#endif
#endif
   RESOLVEFUNC(PEM_read_bio_DSA_PUBKEY)
   RESOLVEFUNC(PEM_read_bio_RSA_PUBKEY)
#ifndef OPENSSL_NO_EC
   RESOLVEFUNC(PEM_read_bio_EC_PUBKEY)
#endif
   RESOLVEFUNC(PEM_write_bio_DSA_PUBKEY)
   RESOLVEFUNC(PEM_write_bio_RSA_PUBKEY)
#ifndef OPENSSL_NO_EC
   RESOLVEFUNC(PEM_write_bio_EC_PUBKEY)
#endif
   RESOLVEFUNC(RAND_seed)
   RESOLVEFUNC(RAND_status)
   RESOLVEFUNC(RSA_new)
   RESOLVEFUNC(RSA_free)
   RESOLVEFUNC(sk_new_null)
   RESOLVEFUNC(sk_push)
   RESOLVEFUNC(sk_free)
   RESOLVEFUNC(sk_num)
   RESOLVEFUNC(sk_pop_free)
   RESOLVEFUNC(sk_value)
   RESOLVEFUNC(SSL_CIPHER_description)
   RESOLVEFUNC(SSL_CIPHER_get_bits)
   RESOLVEFUNC(SSL_CTX_check_private_key)
   RESOLVEFUNC(SSL_CTX_ctrl)
   RESOLVEFUNC(SSL_CTX_free)
   RESOLVEFUNC(SSL_CTX_new)
   RESOLVEFUNC(SSL_CTX_set_cipher_list)
   RESOLVEFUNC(SSL_CTX_set_default_verify_paths)
   RESOLVEFUNC(SSL_CTX_set_verify)
   RESOLVEFUNC(SSL_CTX_set_verify_depth)
   RESOLVEFUNC(SSL_CTX_use_certificate)
   RESOLVEFUNC(SSL_CTX_use_certificate_file)
   RESOLVEFUNC(SSL_CTX_use_PrivateKey)
   RESOLVEFUNC(SSL_CTX_use_RSAPrivateKey)
   RESOLVEFUNC(SSL_CTX_use_PrivateKey_file)
   RESOLVEFUNC(SSL_CTX_get_cert_store);
   RESOLVEFUNC(SSL_accept)
   RESOLVEFUNC(SSL_clear)
   RESOLVEFUNC(SSL_connect)
   RESOLVEFUNC(SSL_free)
   RESOLVEFUNC(SSL_get_ciphers)
   RESOLVEFUNC(SSL_get_current_cipher)
   RESOLVEFUNC(SSL_version)
   RESOLVEFUNC(SSL_get_error)
   RESOLVEFUNC(SSL_get_peer_cert_chain)
   RESOLVEFUNC(SSL_get_peer_certificate)
   RESOLVEFUNC(SSL_get_verify_result)
   RESOLVEFUNC(SSL_library_init)
   RESOLVEFUNC(SSL_load_error_strings)
   RESOLVEFUNC(SSL_new)
   RESOLVEFUNC(SSL_ctrl)
   RESOLVEFUNC(SSL_read)
   RESOLVEFUNC(SSL_set_accept_state)
   RESOLVEFUNC(SSL_set_bio)
   RESOLVEFUNC(SSL_set_connect_state)
   RESOLVEFUNC(SSL_shutdown)
   RESOLVEFUNC(SSL_set_session)
   RESOLVEFUNC(SSL_SESSION_free)
   RESOLVEFUNC(SSL_get1_session)
   RESOLVEFUNC(SSL_get_session)
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
   RESOLVEFUNC(SSL_get_ex_new_index)
   RESOLVEFUNC(SSL_set_ex_data)
   RESOLVEFUNC(SSL_get_ex_data)
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10001000L && !defined(OPENSSL_NO_PSK)
   RESOLVEFUNC(SSL_set_psk_client_callback)
#endif
   RESOLVEFUNC(SSL_write)
#ifndef OPENSSL_NO_SSL2
   RESOLVEFUNC(SSLv2_client_method)
#endif
#ifndef OPENSSL_NO_SSL3_METHOD
   RESOLVEFUNC(SSLv3_client_method)
#endif
   RESOLVEFUNC(SSLv23_client_method)
   RESOLVEFUNC(TLSv1_client_method)
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
   RESOLVEFUNC(TLSv1_1_client_method)
   RESOLVEFUNC(TLSv1_2_client_method)
#endif
#ifndef OPENSSL_NO_SSL2
   RESOLVEFUNC(SSLv2_server_method)
#endif
#ifndef OPENSSL_NO_SSL3_METHOD
   RESOLVEFUNC(SSLv3_server_method)
#endif
   RESOLVEFUNC(SSLv23_server_method)
   RESOLVEFUNC(TLSv1_server_method)
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
   RESOLVEFUNC(TLSv1_1_server_method)
   RESOLVEFUNC(TLSv1_2_server_method)
#endif
   RESOLVEFUNC(X509_NAME_entry_count)
   RESOLVEFUNC(X509_NAME_get_entry)
   RESOLVEFUNC(X509_NAME_ENTRY_get_data)
   RESOLVEFUNC(X509_NAME_ENTRY_get_object)
   RESOLVEFUNC(X509_PUBKEY_get)
   RESOLVEFUNC(X509_STORE_free)
   RESOLVEFUNC(X509_STORE_new)
   RESOLVEFUNC(X509_STORE_add_cert)
   RESOLVEFUNC(X509_STORE_CTX_free)
   RESOLVEFUNC(X509_STORE_CTX_init)
   RESOLVEFUNC(X509_STORE_CTX_new)
   RESOLVEFUNC(X509_STORE_CTX_set_purpose)
   RESOLVEFUNC(X509_STORE_CTX_get_error)
   RESOLVEFUNC(X509_STORE_CTX_get_error_depth)
   RESOLVEFUNC(X509_STORE_CTX_get_current_cert)
   RESOLVEFUNC(X509_STORE_CTX_get_chain)
   RESOLVEFUNC(X509_cmp)
#ifndef SSLEAY_MACROS
   RESOLVEFUNC(X509_dup)
#endif
   RESOLVEFUNC(X509_print)
   RESOLVEFUNC(X509_EXTENSION_get_object)
   RESOLVEFUNC(X509_free)
   RESOLVEFUNC(X509_get_ext)
   RESOLVEFUNC(X509_get_ext_count)
   RESOLVEFUNC(X509_get_ext_d2i)
   RESOLVEFUNC(X509V3_EXT_get)
   RESOLVEFUNC(X509V3_EXT_d2i)
   RESOLVEFUNC(X509_EXTENSION_get_critical)
   RESOLVEFUNC(X509_EXTENSION_get_data)
   RESOLVEFUNC(BASIC_CONSTRAINTS_free)
   RESOLVEFUNC(AUTHORITY_KEYID_free)
   RESOLVEFUNC(ASN1_STRING_print)
   RESOLVEFUNC(X509_check_issued)
   RESOLVEFUNC(X509_get_issuer_name)
   RESOLVEFUNC(X509_get_subject_name)
   RESOLVEFUNC(X509_verify_cert)
   RESOLVEFUNC(d2i_X509)
   RESOLVEFUNC(i2d_X509)
#ifdef SSLEAY_MACROS
   RESOLVEFUNC(i2d_DSAPrivateKey)
   RESOLVEFUNC(i2d_RSAPrivateKey)
   RESOLVEFUNC(d2i_DSAPrivateKey)
   RESOLVEFUNC(d2i_RSAPrivateKey)
#endif
   RESOLVEFUNC(OPENSSL_add_all_algorithms_noconf)
   RESOLVEFUNC(OPENSSL_add_all_algorithms_conf)
   RESOLVEFUNC(SSL_CTX_load_verify_locations)
   RESOLVEFUNC(SSLeay)
   RESOLVEFUNC(SSLeay_version)
   RESOLVEFUNC(i2d_SSL_SESSION)
   RESOLVEFUNC(d2i_SSL_SESSION)
#if OPENSSL_VERSION_NUMBER >= 0x1000100fL && !defined(OPENSSL_NO_NEXTPROTONEG)
   RESOLVEFUNC(SSL_select_next_proto)
   RESOLVEFUNC(SSL_CTX_set_next_proto_select_cb)
   RESOLVEFUNC(SSL_get0_next_proto_negotiated)
#endif // OPENSSL_VERSION_NUMBER >= 0x1000100fL ...
   RESOLVEFUNC(DH_new)
   RESOLVEFUNC(DH_free)
   RESOLVEFUNC(d2i_DHparams)
   RESOLVEFUNC(BN_bin2bn)
#ifndef OPENSSL_NO_EC
   RESOLVEFUNC(EC_KEY_dup)
   RESOLVEFUNC(EC_KEY_new_by_curve_name)
   RESOLVEFUNC(EC_KEY_free)
   RESOLVEFUNC(EC_get_builtin_curves)
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
   if (q_SSLeay() >= 0x10002000L)
      RESOLVEFUNC(EC_curve_nist2nid)
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L
#endif // OPENSSL_NO_EC
      RESOLVEFUNC(PKCS12_parse)
      RESOLVEFUNC(d2i_PKCS12_bio)
      RESOLVEFUNC(PKCS12_free)

      symbolsResolved = true;
   delete libs.first;
   delete libs.second;
   return true;
}


#else // !defined QT_LINKED_OPENSSL

bool q_resolveOpenSslSymbols()
{
#if ! defined(QT_OPENSSL)
   return false;
#endif
   return true;
}
#endif

//==============================================================================
// contributed by Jay Case of Sarvega, Inc.; http://sarvega.com/
// Based on X509_cmp_time() for intitial buffer hacking.
//==============================================================================
QDateTime q_getTimeFromASN1(const ASN1_TIME *aTime)
{
   size_t lTimeLength = aTime->length;
   char *pString = (char *) aTime->data;

   if (aTime->type == V_ASN1_UTCTIME) {

      char lBuffer[24];
      char *pBuffer = lBuffer;

      if ((lTimeLength < 11) || (lTimeLength > 17)) {
         return QDateTime();
      }

      memcpy(pBuffer, pString, 10);
      pBuffer += 10;
      pString += 10;

      if ((*pString == 'Z') || (*pString == '-') || (*pString == '+')) {
         *pBuffer++ = '0';
         *pBuffer++ = '0';
      } else {
         *pBuffer++ = *pString++;
         *pBuffer++ = *pString++;
         // Skip any fractional seconds...
         if (*pString == '.') {
            pString++;
            while ((*pString >= '0') && (*pString <= '9')) {
               pString++;
            }
         }
      }

      *pBuffer++ = 'Z';
      *pBuffer++ = '\0';

      time_t lSecondsFromUCT;
      if (*pString == 'Z') {
         lSecondsFromUCT = 0;
      } else {
         if ((*pString != '+') && (*pString != '-')) {
            return QDateTime();
         }

         lSecondsFromUCT = ((pString[1] - '0') * 10 + (pString[2] - '0')) * 60;
         lSecondsFromUCT += (pString[3] - '0') * 10 + (pString[4] - '0');
         lSecondsFromUCT *= 60;
         if (*pString == '-') {
            lSecondsFromUCT = -lSecondsFromUCT;
         }
      }

      tm lTime;
      lTime.tm_sec  = ((lBuffer[10] - '0') * 10) + (lBuffer[11] - '0');
      lTime.tm_min  = ((lBuffer[8] - '0') * 10) + (lBuffer[9] - '0');
      lTime.tm_hour = ((lBuffer[6] - '0') * 10) + (lBuffer[7] - '0');
      lTime.tm_mday = ((lBuffer[4] - '0') * 10) + (lBuffer[5] - '0');
      lTime.tm_mon  = (((lBuffer[2] - '0') * 10) + (lBuffer[3] - '0')) - 1;
      lTime.tm_year = ((lBuffer[0] - '0') * 10) + (lBuffer[1] - '0');

      if (lTime.tm_year < 50) {
         lTime.tm_year += 100;   // RFC 2459
      }

      QDate resDate(lTime.tm_year + 1900, lTime.tm_mon + 1, lTime.tm_mday);
      QTime resTime(lTime.tm_hour, lTime.tm_min, lTime.tm_sec);

      QDateTime result(resDate, resTime, Qt::UTC);
      result = result.addSecs(lSecondsFromUCT);
      return result;

   } else if (aTime->type == V_ASN1_GENERALIZEDTIME) {

      if (lTimeLength < 15) {
         return QDateTime();   // hopefully never triggered
      }

      // generalized time is always YYYYMMDDHHMMSSZ (RFC 2459, section 4.1.2.5.2)
      tm lTime;
      lTime.tm_sec  = ((pString[12] - '0') * 10) + (pString[13] - '0');
      lTime.tm_min  = ((pString[10] - '0') * 10) + (pString[11] - '0');
      lTime.tm_hour = ((pString[8] - '0') * 10) + (pString[9] - '0');
      lTime.tm_mday = ((pString[6] - '0') * 10) + (pString[7] - '0');
      lTime.tm_mon  = (((pString[4] - '0') * 10) + (pString[5] - '0'));
      lTime.tm_year = ((pString[0] - '0') * 1000) + ((pString[1] - '0') * 100) +
                      ((pString[2] - '0') * 10) + (pString[3] - '0');

      QDate resDate(lTime.tm_year, lTime.tm_mon, lTime.tm_mday);
      QTime resTime(lTime.tm_hour, lTime.tm_min, lTime.tm_sec);

      QDateTime result(resDate, resTime, Qt::UTC);
      return result;

   } else {
      qWarning("unsupported date format detected");
      return QDateTime();
   }

}
