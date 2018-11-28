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

#ifndef QOFFSCREENSURFACE_H
#define QOFFSCREENSURFACE_H

#include <QObject>
#include <qsurface.h>

class QOffscreenSurfacePrivate;
class QPlatformOffscreenSurface;
class QScreen;

class Q_GUI_EXPORT QOffscreenSurface : public QObject, public QSurface
{
    GUI_CS_OBJECT_MULTIPLE(QOffscreenSurface, QObject)
    Q_DECLARE_PRIVATE(QOffscreenSurface)

public:
    explicit QOffscreenSurface(QScreen *screen = nullptr);
    virtual ~QOffscreenSurface();

    SurfaceType surfaceType() const override;

    void create();
    void destroy();

    bool isValid() const;

    void setFormat(const QSurfaceFormat &format);
    QSurfaceFormat format() const override;
    QSurfaceFormat requestedFormat() const;

    QSize size() const override;

    QScreen *screen() const;
    void setScreen(QScreen *screen);

    QPlatformOffscreenSurface *handle() const;

    GUI_CS_SIGNAL_1(Public, void screenChanged(QScreen * screen))
    GUI_CS_SIGNAL_2(screenChanged,screen)

 protected:
   QScopedPointer<QOffscreenSurfacePrivate> d_ptr;

private:
    GUI_CS_SLOT_1(Private, void screenDestroyed(QObject * screen))
    GUI_CS_SLOT_2(screenDestroyed)

    QPlatformSurface *surfaceHandle() const override;

    Q_DISABLE_COPY(QOffscreenSurface)
};

#endif
