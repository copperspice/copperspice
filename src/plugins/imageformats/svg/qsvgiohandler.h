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

#ifndef QSVGIOHANDLER_H
#define QSVGIOHANDLER_H

#include <qimageiohandler.h>

#ifndef QT_NO_SVGRENDERER

class QImage;
class QByteArray;
class QIODevice;
class QVariant;
class QSvgIOHandlerPrivate;

class QSvgIOHandler : public QImageIOHandler
{
 public:
    QSvgIOHandler();
    ~QSvgIOHandler();

    bool canRead() override;
    QString name() const override;
    bool read(QImage *image) override;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) override;
    void setOption(ImageOption option, const QVariant & value) override;
    bool supportsOption(ImageOption option) const override;

 private:
    QSvgIOHandlerPrivate *d;
};

#endif // QT_NO_SVGRENDERER
#endif
