/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qatomic.h>
#include <qbytearray.h>
#include <qdebug.h>
#include <qiodevice.h>
#include <qsslkey.h>
#include <qsslsocket.h>

#include <qsslkey_p.h>
#include <qsslsocket_openssl_symbols_p.h>
#include <qsslsocket_p.h>

void QSslKeyPrivate::clear(bool deep)
{
   isNull = true;

   if (! QSslSocket::supportsSsl()) {
      return;
   }

   if (algorithm == QSsl::Rsa && rsa) {
      if (deep) {
         q_RSA_free(rsa);
      }
      rsa = nullptr;
   }

   if (algorithm == QSsl::Dsa && dsa) {
      if (deep) {
         q_DSA_free(dsa);
      }

      dsa = nullptr;
   }

#ifndef OPENSSL_NO_EC
   if (algorithm == QSsl::Ec && ec) {
      if (deep) {
         q_EC_KEY_free(ec);
      }
      ec = nullptr;
   }
#endif

   if (algorithm == QSsl::Opaque && opaque) {
      if (deep) {
         q_EVP_PKEY_free(opaque);
      }
      opaque = nullptr;
   }
}

bool QSslKeyPrivate::fromEVP_PKEY(EVP_PKEY *pkey)
{
   int pkey_type;

#if OPENSSL_VERSION_NUMBER > 0x10100000L
   pkey_type = q_EVP_PKEY_base_id(pkey);
#else
   pkey_type = pkey->type;
#endif

   if (pkey_type == EVP_PKEY_RSA) {
      isNull = false;
      algorithm = QSsl::Rsa;
      type = QSsl::PrivateKey;

#if OPENSSL_VERSION_NUMBER > 0x10100000L
      // Using the _get1_ version of this function means we have to also free
      // the struct using the apropriate functions. This is done in clear().
      rsa = q_EVP_PKEY_get1_RSA(pkey);
#else
      rsa = q_RSA_new();
      memcpy(rsa, q_EVP_PKEY_get1_RSA(pkey), sizeof(RSA));
#endif

      return true;

   } else if (pkey_type == EVP_PKEY_DSA) {
      isNull = false;
      algorithm = QSsl::Dsa;
      type = QSsl::PrivateKey;

#if OPENSSL_VERSION_NUMBER > 0x10100000L
      // Using the _get1_ version of this function means we have to also free
      // the struct using the apropriate functions. This is done in clear().
      dsa = q_EVP_PKEY_get1_DSA(pkey);
#else
      dsa = q_DSA_new();
      memcpy(dsa, q_EVP_PKEY_get1_DSA(pkey), sizeof(DSA));
#endif

      return true;
   }

#ifndef OPENSSL_NO_EC
   else if (pkey_type == EVP_PKEY_EC) {
      isNull = false;
      algorithm = QSsl::Ec;
      type = QSsl::PrivateKey;
      ec = q_EC_KEY_dup(q_EVP_PKEY_get1_EC_KEY(pkey));

      return true;
   }
#endif

   else {
      // Unknown key type. This could be handled as opaque, but then
      // we'd eventually leak memory since we wouldn't be able to free
      // the underlying EVP_PKEY structure. For now, we won't support this.
   }

   return false;
}

void QSslKeyPrivate::decodeDer(const QByteArray &der, bool deepClear)
{
   QMap<QByteArray, QByteArray> headers;
   decodePem(pemFromDer(der, headers), QByteArray(), deepClear);
}

void QSslKeyPrivate::decodePem(const QByteArray &pem, const QByteArray &passPhrase, bool deepClear)
{
   if (pem.isEmpty()) {
      return;
   }

   clear(deepClear);

   if (!QSslSocket::supportsSsl()) {
      return;
   }

   BIO *bio = q_BIO_new_mem_buf(const_cast<char *>(pem.data()), pem.size());
   if (!bio) {
      return;
   }

   void *phrase = const_cast<char *>(passPhrase.constData());

   if (algorithm == QSsl::Rsa) {
      RSA *result = (type == QSsl::PublicKey) ? q_PEM_read_bio_RSA_PUBKEY(bio, &rsa, nullptr, phrase)
                    : q_PEM_read_bio_RSAPrivateKey(bio, &rsa, nullptr, phrase);

      if (rsa && rsa == result) {
         isNull = false;
      }

   } else if (algorithm == QSsl::Dsa) {
      DSA *result = (type == QSsl::PublicKey) ? q_PEM_read_bio_DSA_PUBKEY(bio, &dsa, nullptr, phrase)
                    : q_PEM_read_bio_DSAPrivateKey(bio, &dsa, nullptr, phrase);

      if (dsa && dsa == result) {
         isNull = false;
      }

#ifndef OPENSSL_NO_EC
   } else if (algorithm == QSsl::Ec) {
      EC_KEY *result = (type == QSsl::PublicKey) ? q_PEM_read_bio_EC_PUBKEY(bio, &ec, nullptr, phrase)
                       : q_PEM_read_bio_ECPrivateKey(bio, &ec, nullptr, phrase);

      if (ec && ec == result) {
         isNull = false;
      }
#endif

   }

   q_BIO_free(bio);
}

int QSslKeyPrivate::length() const
{
   if (isNull || algorithm == QSsl::Opaque) {
      return -1;
   }

   switch (algorithm) {
      case QSsl::Rsa:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
         return q_RSA_size(rsa) * std::numeric_limits<uint8_t>::digits;
#else
         return q_BN_num_bits(rsa->n);
#endif

      case QSsl::Dsa:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
         return q_DSA_bits(dsa);
#else
         return q_BN_num_bits(dsa->p);
#endif

#ifndef OPENSSL_NO_EC
      case QSsl::Ec:
         return q_EC_GROUP_get_degree(q_EC_KEY_get0_group(ec));
#endif
      default:
         return -1;
   }
}

QByteArray QSslKeyPrivate::toPem(const QByteArray &passPhrase) const
{
   if (!QSslSocket::supportsSsl() || isNull || algorithm == QSsl::Opaque) {
      return QByteArray();
   }

   BIO *bio = q_BIO_new(q_BIO_s_mem());
   if (! bio) {
      return QByteArray();
   }

   bool fail = false;

   if (algorithm == QSsl::Rsa) {
      if (type == QSsl::PublicKey) {
         if (!q_PEM_write_bio_RSA_PUBKEY(bio, rsa)) {
            fail = true;
         }
      } else {
         if (!q_PEM_write_bio_RSAPrivateKey(
                  bio, rsa,
                  // the cipher should be selectable in the API:
                  passPhrase.isEmpty() ? (const EVP_CIPHER *)nullptr : q_EVP_des_ede3_cbc(),
                  const_cast<uchar *>((const uchar *)passPhrase.data()), passPhrase.size(), nullptr, nullptr)) {

            fail = true;
         }
      }

   } else if (algorithm == QSsl::Dsa) {
      if (type == QSsl::PublicKey) {
         if (!q_PEM_write_bio_DSA_PUBKEY(bio, dsa)) {
            fail = true;
         }

      } else {
         if (! q_PEM_write_bio_DSAPrivateKey(bio, dsa,
                  // ### the cipher should be selectable in the API:
                  passPhrase.isEmpty() ? (const EVP_CIPHER *)nullptr : q_EVP_des_ede3_cbc(),
                  const_cast<uchar *>((const uchar *)passPhrase.data()), passPhrase.size(), nullptr, nullptr)) {

            fail = true;
         }
      }

#ifndef OPENSSL_NO_EC
   } else if (algorithm == QSsl::Ec) {
      if (type == QSsl::PublicKey) {
         if (! q_PEM_write_bio_EC_PUBKEY(bio, ec)) {
            fail = true;
         }

      } else {
         if (! q_PEM_write_bio_ECPrivateKey(bio, ec,
                  // ### the cipher should be selectable in the API:
                  passPhrase.isEmpty() ? (const EVP_CIPHER *)nullptr : q_EVP_des_ede3_cbc(),
                  const_cast<uchar *>((const uchar *)passPhrase.data()), passPhrase.size(), nullptr, nullptr)) {

            fail = true;
         }
      }
#endif

   } else {
      fail = true;
   }

   QByteArray pem;
   if (! fail) {
      char *data;
      long size = q_BIO_get_mem_data(bio, &data);
      pem = QByteArray(data, size);
   }

   q_BIO_free(bio);

   return pem;
}

Qt::HANDLE QSslKeyPrivate::handle() const
{
   switch (algorithm) {
      case QSsl::Opaque:
         return Qt::HANDLE(opaque);

      case QSsl::Rsa:
         return Qt::HANDLE(rsa);

      case QSsl::Dsa:
         return Qt::HANDLE(dsa);

#ifndef OPENSSL_NO_EC
      case QSsl::Ec:
         return Qt::HANDLE(ec);
#endif

      default:
         return Qt::HANDLE(nullptr);
   }
}

static QByteArray doCrypt(QSslKeyPrivate::Cipher cipher, const QByteArray &data, const QByteArray &key, const QByteArray &iv, int enc)
{
   EVP_CIPHER_CTX *ctx;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
   ctx = q_EVP_CIPHER_CTX_new();
#else
   EVP_CIPHER_CTX local_ctx;
   ctx = &local_ctx;
#endif

   const EVP_CIPHER *type = nullptr;

   int i   = 0;
   int len = 0;

   switch (cipher) {
      case QSslKeyPrivate::DesCbc:
         type = q_EVP_des_cbc();
         break;

      case QSslKeyPrivate::DesEde3Cbc:
         type = q_EVP_des_ede3_cbc();
         break;

      case QSslKeyPrivate::Rc2Cbc:
         type = q_EVP_rc2_cbc();
         break;
   }

   QByteArray output;
   output.resize(data.size() + EVP_MAX_BLOCK_LENGTH);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
   q_EVP_CIPHER_CTX_reset(ctx);
#else
   q_EVP_CIPHER_CTX_init(ctx);
#endif

   q_EVP_CipherInit(ctx, type, nullptr, nullptr, enc);
   q_EVP_CIPHER_CTX_set_key_length(ctx, key.size());

   if (cipher == QSslKeyPrivate::Rc2Cbc) {
      q_EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_SET_RC2_KEY_BITS, 8 * key.size(), nullptr);
   }

   q_EVP_CipherInit(ctx, nullptr, reinterpret_cast<const unsigned char *>(key.constData()),
                    reinterpret_cast<const unsigned char *>(iv.constData()), enc);

   q_EVP_CipherUpdate(ctx, reinterpret_cast<unsigned char *>(output.data()), &len,
                    reinterpret_cast<const unsigned char *>(data.constData()), data.size());

   q_EVP_CipherFinal(ctx, reinterpret_cast<unsigned char *>(output.data()) + len, &i);

   len += i;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
   q_EVP_CIPHER_CTX_reset(ctx);
#else
   q_EVP_CIPHER_CTX_cleanup(ctx);
#endif

   return output.left(len);
}

QByteArray QSslKeyPrivate::decrypt(Cipher cipher, const QByteArray &data, const QByteArray &key, const QByteArray &iv)
{
   return doCrypt(cipher, data, key, iv, 0);
}

QByteArray QSslKeyPrivate::encrypt(Cipher cipher, const QByteArray &data, const QByteArray &key, const QByteArray &iv)
{
   return doCrypt(cipher, data, key, iv, 1);
}
