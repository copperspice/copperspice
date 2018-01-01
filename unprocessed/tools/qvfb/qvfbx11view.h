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

#ifndef QVFBX11VIEW_H
#define QVFBX11VIEW_H

#include "qvfbview.h"

QT_BEGIN_NAMESPACE

class X11KeyFaker;
class QProcess;
class QTemporaryFile;

class QVFbX11View : public QVFbAbstractView
{
    Q_OBJECT
public:
    QVFbX11View( int id, int w, int h, int d, Rotation r, QWidget *parent = 0);
    virtual ~QVFbX11View();

    QString xServerPath() const { return xserver; }
    void setXServerPath(const QString& path) { xserver = path; }

    int displayId() const;
    int displayWidth() const;
    int displayHeight() const;
    int displayDepth() const;
    Rotation displayRotation() const;

    void skinKeyPressEvent( int code, const QString& text, bool autorep=FALSE );
    void skinKeyReleaseEvent( int code, const QString& text, bool autorep=FALSE );

    void setGamma(double gr, double gg, double gb);
    double gammaRed() const;
    double gammaGreen() const;
    double gammaBlue() const;
    void getGamma(int i, QRgb& rgb);

    bool touchScreenEmulation() const;
    bool lcdScreenEmulation() const;
    int rate();
    bool animating() const;
    QImage image() const;
    void setRate(int);

    double zoomH() const;
    double zoomV() const;

    QSize sizeHint() const;

public slots:
    void setTouchscreenEmulation( bool );
    void setLcdScreenEmulation( bool );
    void setZoom( double, double );
    void setRotation(Rotation);
    void startAnimation( const QString& );
    void stopAnimation();

protected:
    void showEvent(QShowEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

private slots:
    void startXnest();
    void xnestStopped();
    void startKeyFaker();

private:
    int id, w, h, d;
    Rotation rotation;
    double gr, gg, gb;
    bool touchscreen, lcd;
    X11KeyFaker *keyFaker;
    QProcess *xnest;
    QTemporaryFile *serverAuthFile;
    bool shutdown;
    QString xserver;
};

QT_END_NAMESPACE

#endif
