/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qcryptographichash.h>
#include <qiodevice.h>

#include "../../3rdparty/md5/md5.h"
#include "../../3rdparty/md5/md5.cpp"
#include "../../3rdparty/md4/md4.h"
#include "../../3rdparty/md4/md4.cpp"
#include "../../3rdparty/sha1/sha1.cpp"

typedef unsigned char BitSequence;
typedef unsigned long long DataLength;
typedef enum { SUCCESS = 0, FAIL = 1, BAD_HASHLEN = 2 } HashReturn;
#include "../../3rdparty/sha3/KeccakSponge.c"

typedef spongeState hashState;
#include "../../3rdparty/sha3/KeccakNISTInterface.c"

typedef spongeState SHA3Context;
typedef HashReturn (SHA3Init)(hashState *state, int hashbitlen);
typedef HashReturn (SHA3Update)(hashState *state, const BitSequence *data, DataLength databitlen);
typedef HashReturn (SHA3Final)(hashState *state, BitSequence *hashval);

#if Q_PROCESSOR_WORDSIZE == 8
   // 64 bit version

#include "../../3rdparty/sha3/KeccakF-1600-opt64.c"

   static SHA3Init * const sha3Init = Init;
   static SHA3Update * const sha3Update = Update;
   static SHA3Final * const sha3Final = Final;

#else
   // 32 bit optimised fallback

#include "../../3rdparty/sha3/KeccakF-1600-opt32.c"

   static SHA3Init * const sha3Init = Init;
   static SHA3Update * const sha3Update = Update;
   static SHA3Final * const sha3Final = Final;

#endif

#ifdef uint64_t
#undef uint64_t
#endif

#ifdef uint32_t
#undef uint32_t
#endif

#ifdef uint8_t
#undef uint8_t
#endif

#ifdef int_least16_t
#undef int_least16_t
#endif

#define uint64_t       quint64
#define uint32_t       quint32
#define uint8_t        quint8
#define int_least16_t  qint16

#include "../../3rdparty/rfc6234/sha.h"

static int SHA224_256AddLength(SHA256Context *context, unsigned int length);
static int SHA384_512AddLength(SHA512Context *context, unsigned int length);

#include "../../3rdparty/rfc6234/sha224-256.c"
#include "../../3rdparty/rfc6234/sha384-512.c"

#undef uint64_t
#undef uint32_t
#undef uint68_t
#undef int_least16_t

static inline int SHA224_256AddLength(SHA256Context *context, unsigned int length)
{
  quint32 addTemp;
  return SHA224_256AddLengthM(context, length);
}

static inline int SHA384_512AddLength(SHA512Context *context, unsigned int length)
{
  quint64 addTemp;
  return SHA384_512AddLengthM(context, length);
}

class QCryptographicHashPrivate
{
 public:
   QCryptographicHash::Algorithm method;

   union {
      Sha1State sha1Context;

      MD5Context md5Context;
      md4_context md4Context;

      SHA224Context sha224Context;
      SHA256Context sha256Context;
      SHA384Context sha384Context;
      SHA512Context sha512Context;

      SHA3Context sha3Context;
   };

   QByteArray result;
};

QCryptographicHash::QCryptographicHash(Algorithm method)
   : d(new QCryptographicHashPrivate)
{
   d->method = method;
   reset();
}

QCryptographicHash::~QCryptographicHash()
{
   delete d;
}

void QCryptographicHash::reset()
{
   switch (d->method) {
      case Sha1:
         sha1InitState(&d->sha1Context);
         break;

      case Md4:
         md4_init(&d->md4Context);
         break;

      case Md5:
         MD5Init(&d->md5Context);
         break;

      // sha 2
      case Sha224:
         SHA224Reset(&d->sha224Context);
         break;

      case Sha256:
         SHA256Reset(&d->sha256Context);
         break;

      case Sha384:
         SHA384Reset(&d->sha384Context);
         break;

      case Sha512:
         SHA512Reset(&d->sha512Context);
         break;

      // Keccak and sha 3
      case Keccak_224:
      case Sha3_224:
         sha3Init(&d->sha3Context, 224);
         break;

      case Keccak_256:
      case Sha3_256:
         sha3Init(&d->sha3Context, 256);
         break;

      case Keccak_384:
      case Sha3_384:
         sha3Init(&d->sha3Context, 384);
         break;

      case Keccak_512:
      case Sha3_512:
         sha3Init(&d->sha3Context, 512);
         break;
    }

    d->result.clear();
}

void QCryptographicHash::addData(const char *data, int length)
{
    switch (d->method) {
       case Sha1:
           sha1Update(&d->sha1Context, (const unsigned char *)data, length);
           break;

       case Md4:
           md4_update(&d->md4Context, (const unsigned char *)data, length);
           break;

       case Md5:
           MD5Update(&d->md5Context, (const unsigned char *)data, length);
           break;

       // sha 2
       case Sha224:
           SHA224Input(&d->sha224Context, reinterpret_cast<const unsigned char *>(data), length);
           break;

       case Sha256:
           SHA256Input(&d->sha256Context, reinterpret_cast<const unsigned char *>(data), length);
           break;

       case Sha384:
           SHA384Input(&d->sha384Context, reinterpret_cast<const unsigned char *>(data), length);
           break;

       case Sha512:
           SHA512Input(&d->sha512Context, reinterpret_cast<const unsigned char *>(data), length);
           break;

       // Keccak and sha 3
       case Keccak_224:
       case Sha3_224:
           sha3Update(&d->sha3Context, reinterpret_cast<const BitSequence *>(data), length*8);
           break;

       case Keccak_256:
       case Sha3_256:
           sha3Update(&d->sha3Context, reinterpret_cast<const BitSequence *>(data), length*8);
           break;

       case Keccak_384:
       case Sha3_384:
           sha3Update(&d->sha3Context, reinterpret_cast<const BitSequence *>(data), length*8);
           break;

       case Keccak_512:
       case Sha3_512:
           sha3Update(&d->sha3Context, reinterpret_cast<const BitSequence *>(data), length*8);
           break;
    }

    d->result.clear();
}

void QCryptographicHash::addData(const QByteArray &data)
{
   addData(data.constData(), data.length());
}

bool QCryptographicHash::addData(QIODevice *device)
{
   if (! device->isReadable()) {
      return false;
   }

   if (!device->isOpen()) {
      return false;
   }

   char buffer[1024];
   int length;

   while ((length = device->read(buffer, sizeof(buffer))) > 0) {
      addData(buffer, length);
   }

   return device->atEnd();
}

QByteArray QCryptographicHash::result() const
{
   constexpr unsigned char sha3Domain = 0x80;

   if (!d->result.isEmpty()) {
      return d->result;
   }

   switch (d->method) {

      case Sha1: {
         Sha1State copy = d->sha1Context;
         d->result.resize(20);
         sha1FinalizeState(&copy);
         sha1ToHash(&copy, (unsigned char *)d->result.data());
         break;
      }

      case Md4: {
         md4_context copy = d->md4Context;
         d->result.resize(MD4_RESULTLEN);
         md4_final(&copy, (unsigned char *)d->result.data());
         break;
      }

      case Md5: {
         MD5Context copy = d->md5Context;
         d->result.resize(16);
         MD5Final(&copy, (unsigned char *)d->result.data());
         break;
      }

      // sha 2
      case Sha224: {
         SHA224Context copy = d->sha224Context;
         d->result.resize(SHA224HashSize);
         SHA224Result(&copy, reinterpret_cast<unsigned char *>(d->result.data()));
         break;
      }

      case Sha256:{
         SHA256Context copy = d->sha256Context;
         d->result.resize(SHA256HashSize);
         SHA256Result(&copy, reinterpret_cast<unsigned char *>(d->result.data()));
         break;
      }

      case Sha384:{
         SHA384Context copy = d->sha384Context;
         d->result.resize(SHA384HashSize);
         SHA384Result(&copy, reinterpret_cast<unsigned char *>(d->result.data()));
         break;
      }

      case Sha512:{
         SHA512Context copy = d->sha512Context;
         d->result.resize(SHA512HashSize);
         SHA512Result(&copy, reinterpret_cast<unsigned char *>(d->result.data()));
         break;
      }

      // keccak
      case Keccak_224: {
         SHA3Context copy = d->sha3Context;
         d->result.resize(224/8);
         sha3Final(&copy, reinterpret_cast<BitSequence *>(d->result.data()));
         break;
      }

      case Keccak_256: {
         SHA3Context copy = d->sha3Context;
         d->result.resize(256/8);
         sha3Final(&copy, reinterpret_cast<BitSequence *>(d->result.data()));
         break;
      }

      case Keccak_384: {
         SHA3Context copy = d->sha3Context;
         d->result.resize(384/8);
         sha3Final(&copy, reinterpret_cast<BitSequence *>(d->result.data()));
         break;
      }

      case Keccak_512: {
         SHA3Context copy = d->sha3Context;
         d->result.resize(512/8);
         sha3Final(&copy, reinterpret_cast<BitSequence *>(d->result.data()));
         break;
      }

      // sha 3
      case Sha3_224: {
         SHA3Context copy = d->sha3Context;
         d->result.resize(224/8);

         sha3Update(&copy, &sha3Domain, 2);
         sha3Final(&copy, reinterpret_cast<BitSequence *>(d->result.data()));
         break;
      }

      case Sha3_256: {
         SHA3Context copy = d->sha3Context;
         d->result.resize(256/8);

         sha3Update(&copy, &sha3Domain, 2);
         sha3Final(&copy, reinterpret_cast<BitSequence *>(d->result.data()));
         break;
      }

      case Sha3_384: {
         SHA3Context copy = d->sha3Context;
         d->result.resize(384/8);

         sha3Update(&copy, &sha3Domain, 2);
         sha3Final(&copy, reinterpret_cast<BitSequence *>(d->result.data()));
         break;
      }

      case Sha3_512: {
         SHA3Context copy = d->sha3Context;
         d->result.resize(512/8);

         sha3Update(&copy, &sha3Domain, 2);
         sha3Final(&copy, reinterpret_cast<BitSequence *>(d->result.data()));
         break;
      }

    }

    return d->result;
}

QByteArray QCryptographicHash::hash(const QByteArray &data, Algorithm method)
{
   QCryptographicHash hash(method);
   hash.addData(data);
   return hash.result();
}


