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

#ifndef QTRANSPORTAUTH_QWS_H
#define QTRANSPORTAUTH_QWS_H

#include <QtCore/qglobal.h>

#if !defined(QT_NO_SXE) || defined(SXE_INSTALLER)

#include <QtCore/qobject.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qpointer.h>

#include <sys/types.h>

QT_BEGIN_NAMESPACE

class QAuthDevice;
class QWSClient;
class QIODevice;
class QTransportAuthPrivate;
class QMutex;

class Q_GUI_EXPORT QTransportAuth : public QObject
{
   GUI_CS_OBJECT(QTransportAuth)

 public:
   static QTransportAuth *getInstance();

   enum Result {
      // Error codes
      Pending = 0x00,
      TooSmall = 0x01,
      CacheMiss = 0x02,
      NoMagic = 0x03,
      NoSuchKey = 0x04,
      FailMatch = 0x05,
      OutOfDate = 0x06,
      // reserved for expansion
      Success = 0x1e,
      ErrMask = 0x1f,

      // Verification codes
      Allow = 0x20,
      Deny = 0x40,
      Ask = 0x60,
      // reserved
      StatusMask = 0xe0
   };

   enum Properties {
      Trusted = 0x01,
      Connection = 0x02,
      UnixStreamSock = 0x04,
      SharedMemory = 0x08,
      MessageQueue = 0x10,
      UDP = 0x20,
      TCP = 0x40,
      UserDefined = 0x80,
      TransportType = 0xfc
   };

   struct Data {
      Data() {
         processId = -1;
      }
      Data( unsigned char p, int d )
         : properties( p )
         , descriptor( d )
         , processId( -1 ) {
         if (( properties & TransportType ) == TCP ||
               ( properties & TransportType ) == UnixStreamSock ) {
            properties |= Connection;
         }
      }

      unsigned char properties;
      unsigned char progId;
      unsigned char status;
      unsigned int descriptor;   // socket fd or shmget key
      pid_t processId;

      bool trusted() const;
      void setTrusted( bool );
      bool connection() const;
      void setConnection( bool );
   };

   static const char *errorString( const QTransportAuth::Data &);

   QTransportAuth::Data *connectTransport( unsigned char, int );

   QAuthDevice *authBuf( QTransportAuth::Data *, QIODevice *);
   QAuthDevice *recvBuf( QTransportAuth::Data *, QIODevice *);
   QIODevice *passThroughByClient( QWSClient *) const;

   void setKeyFilePath( const QString &);
   QString keyFilePath() const;
   const unsigned char *getClientKey( unsigned char progId );
   void invalidateClientKeyCache();
   QMutex *getKeyFileMutex();
   void setLogFilePath( const QString &);
   QString logFilePath() const;
   void setPackageRegistry( QObject *registry );
   bool isDiscoveryMode() const;
   void setProcessKey( const char *);
   void setProcessKey( const char *, const char *);
   void registerPolicyReceiver( QObject *);
   void unregisterPolicyReceiver( QObject *);

   bool authToMessage( QTransportAuth::Data &d, char *hdr, const char *msg, int msgLen );
   bool authFromMessage( QTransportAuth::Data &d, const char *msg, int msgLen );

   bool authorizeRequest( QTransportAuth::Data &d, const QString &request );

   GUI_CS_SIGNAL_1(Public, void policyCheck(QTransportAuth::Data &un_named_arg1, const QString &un_named_arg2))
   GUI_CS_SIGNAL_2(policyCheck, un_named_arg1, un_named_arg2)
   GUI_CS_SIGNAL_1(Public, void authViolation(QTransportAuth::Data &un_named_arg1))
   GUI_CS_SIGNAL_2(authViolation, un_named_arg1)

 private :
   GUI_CS_SLOT_1(Private, void bufferDestroyed(QObject *un_named_arg1))
   GUI_CS_SLOT_2(bufferDestroyed)

   // users should never construct their own
   QTransportAuth();
   ~QTransportAuth();

   friend class QAuthDevice;
   Q_DECLARE_PRIVATE(QTransportAuth)
};

class Q_GUI_EXPORT RequestAnalyzer
{
 public:
   RequestAnalyzer();
   virtual ~RequestAnalyzer();
   QString operator()( QByteArray *data ) {
      return analyze( data );
   }
   bool requireMoreData() const {
      return moreData;
   }
   qint64 bytesAnalyzed() const {
      return dataSize;
   }

 protected:
   virtual QString analyze( QByteArray *);
   bool moreData;
   qint64 dataSize;
};

/*!
  \internal
  \class QAuthDevice

  \brief Pass-through QIODevice sub-class for authentication.

   Use this class to forward on or receive forwarded data over a real
   device for authentication.
*/
class Q_GUI_EXPORT QAuthDevice : public QIODevice
{
   GUI_CS_OBJECT(QAuthDevice)

 public:
   enum AuthDirection {
      Receive,
      Send
   };
   QAuthDevice( QIODevice *, QTransportAuth::Data *, AuthDirection );
   ~QAuthDevice();
   void setTarget( QIODevice *t ) {
      m_target = t;
   }
   QIODevice *target() const {
      return m_target;
   }
   void setClient( QObject *);
   QObject *client() const;
   void setRequestAnalyzer( RequestAnalyzer *);
   bool isSequential() const;
   bool atEnd() const;
   qint64 bytesAvailable() const;
   qint64 bytesToWrite() const;
   bool seek( qint64 );
   QByteArray &buffer();

 protected:
   qint64 readData( char *, qint64 );
   qint64 writeData(const char *, qint64 );

 private :
   GUI_CS_SLOT_1(Private, void recvReadyRead())
   GUI_CS_SLOT_2(recvReadyRead)
   GUI_CS_SLOT_1(Private, void targetBytesWritten(qint64 un_named_arg1))
   GUI_CS_SLOT_2(targetBytesWritten)

   bool authorizeMessage();

   QTransportAuth::Data *d;
   AuthDirection way;
   QIODevice *m_target;
   QObject *m_client;
   QByteArray msgQueue;
   qint64 m_bytesAvailable;
   qint64 m_skipWritten;

   RequestAnalyzer *analyzer;
};

inline bool QAuthDevice::isSequential() const
{
   return true;
}

inline bool QAuthDevice::seek( qint64 )
{
   return false;
}

inline bool QAuthDevice::atEnd() const
{
   return msgQueue.isEmpty();
}

inline qint64 QAuthDevice::bytesAvailable() const
{
   if ( way == Receive ) {
      return m_bytesAvailable;
   } else {
      return ( m_target ? m_target->bytesAvailable() : 0 );
   }
}

inline qint64 QAuthDevice::bytesToWrite() const
{
   return msgQueue.size();
}

inline QByteArray &QAuthDevice::buffer()
{
   return msgQueue;
}

QT_END_NAMESPACE

#endif // QT_NO_SXE
#endif // QTRANSPORTAUTH_QWS_H
