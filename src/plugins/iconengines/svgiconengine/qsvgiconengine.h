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

#ifndef QSVGICONENGINE_H
#define QSVGICONENGINE_H

#include <QtGui/qiconengine.h>
#include <QtCore/qshareddata.h>

#ifndef QT_NO_SVG

QT_BEGIN_NAMESPACE

class QSvgIconEnginePrivate;

class QSvgIconEngine : public QIconEngineV2
{
public:
    QSvgIconEngine();
    QSvgIconEngine(const QSvgIconEngine &other);
    ~QSvgIconEngine();
    void paint(QPainter *painter, const QRect &rect,
               QIcon::Mode mode, QIcon::State state);
    QSize actualSize(const QSize &size, QIcon::Mode mode,
                     QIcon::State state);
    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state);

    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                   QIcon::State state);
    void addFile(const QString &fileName, const QSize &size,
                 QIcon::Mode mode, QIcon::State state);

    QString key() const;
    QIconEngineV2 *clone() const;
    bool read(QDataStream &in);
    bool write(QDataStream &out) const;

private:
    QSharedDataPointer<QSvgIconEnginePrivate> d;
};

QT_END_NAMESPACE

#endif // QT_NO_SVG
#endif
