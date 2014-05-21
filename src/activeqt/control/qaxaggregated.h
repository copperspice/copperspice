/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the ActiveQt framework of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QAXAGGREGATED_H
#define QAXAGGREGATED_H

#include <QtCore/qobject.h>

struct IUnknown;

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(ActiveQt)

#ifndef QT_NO_WIN_ACTIVEQT

struct QUuid;

class QAxAggregated
{
    friend class QAxServerBase;
    friend class QAxClientSite;
public:
    virtual long queryInterface(const QUuid &iid, void **iface) = 0;

protected:
    virtual ~QAxAggregated()
    {}

    inline IUnknown *controllingUnknown() const
    { return controlling_unknown; }
    inline QWidget *widget() const 
    {
        return qobject_cast<QWidget*>(the_object);
    }
    inline QObject *object() const { return the_object; }

private:
    IUnknown *controlling_unknown;
    QObject *the_object;
};

#define QAXAGG_IUNKNOWN \
    HRESULT WINAPI QueryInterface(REFIID iid, LPVOID *iface) { \
    return controllingUnknown()->QueryInterface(iid, iface); } \
    ULONG WINAPI AddRef() {return controllingUnknown()->AddRef(); } \
    ULONG WINAPI Release() {return controllingUnknown()->Release(); } \

#endif // QT_NO_WIN_ACTIVEQT

QT_END_NAMESPACE

QT_END_HEADER

#endif // QAXAGGREGATED_H
