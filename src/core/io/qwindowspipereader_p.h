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

#ifndef QWINDOWSPIPEREADER_P_H
#define QWINDOWSPIPEREADER_P_H

#include <qobject.h>
#include <qringbuffer_p.h>
#include <qt_windows.h>

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QWindowsPipeReader : public QObject
{
    CORE_CS_OBJECT(QWindowsPipeReader)

public:
    explicit QWindowsPipeReader(QObject *parent = nullptr);
    ~QWindowsPipeReader();

    void setHandle(HANDLE hPipeReadEnd);
    void startAsyncRead();
    void stop();

    void setMaxReadBufferSize(qint64 size) { readBufferMaxSize = size; }
    qint64 maxReadBufferSize() const { return readBufferMaxSize; }

    bool isPipeClosed() const { return pipeBroken; }
    qint64 bytesAvailable() const;
    qint64 read(char *data, qint64 maxlen);
    bool canReadLine() const;
    bool waitForReadyRead(int msecs);
    bool waitForPipeClosed(int msecs);

    bool isReadOperationActive() const { return readSequenceStarted; }

    CORE_CS_SIGNAL_1(Public, void winError(ulong un_named_arg1,const QString & un_named_arg2))
    CORE_CS_SIGNAL_2(winError,un_named_arg1,un_named_arg2)

    CORE_CS_SIGNAL_1(Public, void readyRead())
    CORE_CS_SIGNAL_2(readyRead)

    CORE_CS_SIGNAL_1(Public, void pipeClosed())
    CORE_CS_SIGNAL_2(pipeClosed)

    CORE_CS_SIGNAL_1(Public, void _q_queueReadyRead())
    CORE_CS_SIGNAL_2(_q_queueReadyRead)

private:
    static void CALLBACK readFileCompleted(DWORD errorCode, DWORD numberOfBytesTransfered, OVERLAPPED *overlappedBase);
    void notified(DWORD errorCode, DWORD numberOfBytesRead);
    DWORD checkPipeState();
    bool waitForNotification(int timeout);
    void emitPendingReadyRead();

    class Overlapped : public OVERLAPPED
    {
        Q_DISABLE_COPY(Overlapped)

    public:
        explicit Overlapped(QWindowsPipeReader *reader);
        void clear();
        QWindowsPipeReader *pipeReader;
    };

    HANDLE handle;
    Overlapped overlapped;
    qint64 readBufferMaxSize;
    QRingBuffer readBuffer;
    qint64 actualReadBufferSize;
    bool stopped;
    bool readSequenceStarted;
    bool notifiedCalled;
    bool pipeBroken;
    bool readyReadPending;
    bool inReadyRead;
};

#endif
