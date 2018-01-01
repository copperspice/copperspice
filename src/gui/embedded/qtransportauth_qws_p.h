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

#ifndef QTRANSPORTAUTH_QWS_P_H
#define QTRANSPORTAUTH_QWS_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_SXE

#include <qtransportauth_qws.h>
#include <qtransportauthdefs_qws.h>
#include <qbuffer.h>
#include <qmutex.h>
#include <qdatetime.h>
#include <QtCore/qcache.h>

QT_BEGIN_NAMESPACE

// Uncomment to generate debug output
// #define QTRANSPORTAUTH_DEBUG 1

#ifdef QTRANSPORTAUTH_DEBUG
void hexstring( char *buf, const unsigned char *key, size_t sz );
#endif

// proj id for ftok usage in sxe
#define SXE_PROJ 10022


void *guaranteed_memset(void *v, int c, size_t n);

class QUnixSocketMessage;

#define QSXE_HEADER_LEN 24

/*
  \macro AUTH_ID
  Macro to manage authentication header.  Format of header is:

  \table
  \header \i BYTES  \i  CONTENT
     \row \i 0-3    \i  magic numbers
     \row \i 4      \i  length of authenticated data (max 255 bytes)
     \row i\ 5      \i  reserved
     \row \i 6-21   \i  MAC digest, or shared secret in case of simple auth
     \row \i 22     \i  program id
     \row \i 23     \i  sequence number
  \endtable
  Total length of the header is 24 bytes

  However this may change.  Instead of coding these numbers use the AUTH_ID,
  AUTH_KEY, AUTH_DATA and AUTH_SPACE macros.
*/

#define AUTH_ID(k) ((unsigned char)(k[QSXE_KEY_LEN]))
#define AUTH_KEY(k) ((unsigned char *)(k))

#define AUTH_DATA(x) (unsigned char *)((x) + QSXE_HEADER_LEN)
#define AUTH_SPACE(x) ((x) + QSXE_HEADER_LEN)
#define QSXE_LEN_IDX 4
#define QSXE_KEY_IDX 6
#define QSXE_PROG_IDX 22
#define QSXE_SEQ_IDX 23

class SxeRegistryLocker : public QObject
{
   GUI_CS_OBJECT(SxeRegistryLocker)

 public:
   SxeRegistryLocker( QObject *);
   ~SxeRegistryLocker();
   bool success() const {
      return m_success;
   }

 private:
   bool m_success;
   QObject *m_reg;
};

class QTransportAuthPrivate
{
   Q_DECLARE_PUBLIC(QTransportAuth)

 public:
   QTransportAuthPrivate();
   virtual ~QTransportAuthPrivate();

   const unsigned char *getClientKey( unsigned char progId );
   void invalidateClientKeyCache();

   bool keyInitialised;
   QString m_logFilePath;
   QString m_keyFilePath;
   QObject *m_packageRegistry;
   AuthCookie authKey;
   QCache<unsigned char, char> keyCache;
   QHash< QObject *, QIODevice *> buffersByClient;
   QMutex keyfileMutex;
};

class FAREnforcer
{
 public:
   static FAREnforcer *getInstance();
   void logAuthAttempt( QDateTime time = QDateTime::currentDateTime() );
   void reset();

 private:
   FAREnforcer();
   FAREnforcer( const FAREnforcer &);
   FAREnforcer &operator=(FAREnforcer const &);

   static const QString FARMessage;
   static const int minutelyRate;
   static const QString SxeTag;
   static const int minute;

   QList<QDateTime> authAttempts;
};

QT_END_NAMESPACE

#endif // QT_NO_SXE
#endif // QTRANSPORTAUTH_QWS_P_H

