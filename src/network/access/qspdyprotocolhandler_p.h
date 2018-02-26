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

#ifndef QSPDYPROTOCOLHANDLER_H
#define QSPDYPROTOCOLHANDLER_H

#include <qabstractprotocolhandler_p.h>
#include <qbytedata_p.h>
#include <qhttpnetworkrequest_p.h>
#include <qnetworkreply.h>

#include <zlib.h>

#ifdef QT_SSL

class QHttpNetworkRequest;
using HttpMessagePair = QPair<QHttpNetworkRequest, QHttpNetworkReply*>;

class QSpdyProtocolHandler : public QObject, public QAbstractProtocolHandler {
    NET_CS_OBJECT(QSpdyProtocolHandler)

public:
    QSpdyProtocolHandler(QHttpNetworkConnectionChannel *channel);
    ~QSpdyProtocolHandler();

    enum DataFrameFlag {
        DataFrame_FLAG_FIN = 0x01,
        DataFrame_FLAG_COMPRESS = 0x02
    };
    using DataFrameFlags = QFlags<DataFrameFlag>;

    enum ControlFrameFlag {
        ControlFrame_FLAG_FIN = 0x01,
        ControlFrame_FLAG_UNIDIRECTIONAL = 0x02
    };
    using ControlFrameFlags = QFlags<ControlFrameFlag>;

    enum SETTINGS_Flag {
        FLAG_SETTINGS_CLEAR_SETTINGS = 0x01
    };
    using SETTINGS_Flags = QFlags<SETTINGS_Flag>;

    enum SETTINGS_ID_Flag {
        FLAG_SETTINGS_PERSIST_VALUE = 0x01,
        FLAG_SETTINGS_PERSISTED = 0x02
    };
    using SETTINGS_ID_Flags = QFlags<SETTINGS_ID_Flag>;

    virtual void _q_receiveReply() override;
    virtual void _q_readyRead() override;
    virtual bool sendRequest() override;

private:

    enum FrameType {
        FrameType_SYN_STREAM = 1,
        FrameType_SYN_REPLY = 2,
        FrameType_RST_STREAM = 3,
        FrameType_SETTINGS = 4,
        FrameType_PING = 6,
        FrameType_GOAWAY = 7,
        FrameType_HEADERS = 8,
        FrameType_WINDOW_UPDATE = 9,
        FrameType_CREDENTIAL // has a special type
    };

    enum StatusCode {
        StatusCode_PROTOCOL_ERROR = 1,
        StatusCode_INVALID_STREAM = 2,
        StatusCode_REFUSED_STREAM = 3,
        StatusCode_UNSUPPORTED_VERSION = 4,
        StatusCode_CANCEL = 5,
        StatusCode_INTERNAL_ERROR = 6,
        StatusCode_FLOW_CONTROL_ERROR = 7,
        StatusCode_STREAM_IN_USE = 8,
        StatusCode_STREAM_ALREADY_CLOSED = 9,
        StatusCode_INVALID_CREDENTIALS = 10,
        StatusCode_FRAME_TOO_LARGE = 11
    };

    enum SETTINGS_ID {
        SETTINGS_UPLOAD_BANDWIDTH = 1,
        SETTINGS_DOWNLOAD_BANDWIDTH = 2,
        SETTINGS_ROUND_TRIP_TIME = 3,
        SETTINGS_MAX_CONCURRENT_STREAMS = 4,
        SETTINGS_CURRENT_CWND = 5,
        SETTINGS_DOWNLOAD_RETRANS_RATE = 6,
        SETTINGS_INITIAL_WINDOW_SIZE = 7,
        SETTINGS_CLIENT_CERTIFICATE_VECTOR_SIZE = 8
    };

    enum GOAWAY_STATUS {
        GOAWAY_OK = 0,
        GOAWAY_PROTOCOL_ERROR = 1,
        GOAWAY_INTERNAL_ERROR = 11
    };

    enum RST_STREAM_STATUS_CODE {
        RST_STREAM_PROTOCOL_ERROR = 1,
        RST_STREAM_INVALID_STREAM = 2,
        RST_STREAM_REFUSED_STREAM = 3,
        RST_STREAM_UNSUPPORTED_VERSION = 4,
        RST_STREAM_CANCEL = 5,
        RST_STREAM_INTERNAL_ERROR = 6,
        RST_STREAM_FLOW_CONTROL_ERROR = 7,
        RST_STREAM_STREAM_IN_USE = 8,
        RST_STREAM_STREAM_ALREADY_CLOSED = 9,
        RST_STREAM_INVALID_CREDENTIALS = 10,
        RST_STREAM_FRAME_TOO_LARGE = 11
    };

    quint64 bytesAvailable() const;
    bool readNextChunk(qint64 length, char *sink);

    void sendControlFrame(FrameType type, ControlFrameFlags flags, const char *data, quint32 length);

    void sendSYN_STREAM(HttpMessagePair pair, qint32 streamID, qint32 associatedToStreamID);
    void sendRST_STREAM(qint32 streamID, RST_STREAM_STATUS_CODE statusCode);
    void sendPING(quint32 pingID);

    bool uploadData(qint32 streamID);

    NET_CS_INVOKABLE_METHOD_1(Private, void sendWINDOW_UPDATE(qint32 streamID, quint32 deltaWindowSize));
    NET_CS_INVOKABLE_METHOD_2(sendWINDOW_UPDATE);

    qint64 sendDataFrame(qint32 streamID, DataFrameFlags flags, quint32 length, const char *data);

    QByteArray composeHeader(const QHttpNetworkRequest &request);
    bool uncompressHeader(const QByteArray &input, QByteArray *output);

    void handleControlFrame(const QByteArray &frameHeaders);
    void handleDataFrame(const QByteArray &frameHeaders);

    void handleSYN_STREAM(char, quint32, const QByteArray &frameData);
    void handleSYN_REPLY(char flags, quint32, const QByteArray &frameData);
    void handleRST_STREAM(char flags, quint32 length, const QByteArray &frameData);
    void handleSETTINGS(char flags, quint32 length, const QByteArray &frameData);
    void handlePING(char, quint32 length, const QByteArray &frameData);
    void handleGOAWAY(char flags, quint32, const QByteArray &frameData);
    void handleHEADERS(char flags, quint32, const QByteArray &frameData);
    void handleWINDOW_UPDATE(char, quint32, const QByteArray &frameData);

    qint32 generateNextStreamID();
    void parseHttpHeaders(char flags, const QByteArray &frameData);

    void replyFinished(QHttpNetworkReply *httpReply, qint32 streamID);
    void replyFinishedWithError(QHttpNetworkReply *httpReply, qint32 streamID,
                                QNetworkReply::NetworkError errorCode, const char *errorMessage);

    qint32 m_nextStreamID;
    QHash<quint32, HttpMessagePair> m_inFlightStreams;

    qint32 m_maxConcurrentStreams;
    quint32 m_initialWindowSize;

    QByteDataBuffer m_spdyBuffer;
    bool m_waitingForCompleteStream;
    z_stream m_deflateStream;
    z_stream m_inflateStream;

    NET_CS_SLOT_1(Private, void _q_uploadDataReadyRead())
    NET_CS_SLOT_2(_q_uploadDataReadyRead)

    NET_CS_SLOT_1(Private, void _q_replyDestroyed(QObject *data))
    NET_CS_SLOT_2(_q_replyDestroyed)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSpdyProtocolHandler::DataFrameFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QSpdyProtocolHandler::ControlFrameFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QSpdyProtocolHandler::SETTINGS_Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QSpdyProtocolHandler::SETTINGS_ID_Flags)


#endif

#endif // QSPDYPROTOCOLHANDLER_H
