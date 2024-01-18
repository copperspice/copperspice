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

#ifndef QLINUXINPUT_H
#define QLINUXINPUT_H

#include <qobject.h>
#include <qnamespace.h>
#include <termios.h>

class QSocketNotifier;
class QLinuxInputMouseHandlerData;

class QLinuxInputMouseHandler : public QObject
{
    Q_OBJECT

public:
    QLinuxInputMouseHandler(const QString &key, const QString &specification);
    ~QLinuxInputMouseHandler();

private slots:
    void readMouseData();

private:
    void sendMouseEvent(int x, int y, Qt::MouseButtons buttons);
    QSocketNotifier *          m_notify;
    int                        m_fd;
    int                        m_x, m_y;
    int m_prevx, m_prevy;
    int m_xoffset, m_yoffset;
    int m_smoothx, m_smoothy;
    Qt::MouseButtons           m_buttons;
    bool m_compression;
    bool m_smooth;
    int m_jitterLimitSquared;
    QLinuxInputMouseHandlerData *d;
};


class QWSLinuxInputKeyboardHandler;

class QLinuxInputKeyboardHandler : public QObject
{
    Q_OBJECT
public:
    QLinuxInputKeyboardHandler(const QString &key, const QString &specification);
    ~QLinuxInputKeyboardHandler();


private:
    void switchLed(int, bool);

private slots:
    void readKeycode();

private:
    QWSLinuxInputKeyboardHandler *m_handler;
    int                           m_fd;
    int                           m_tty_fd;
    struct termios                m_tty_attr;
    int                           m_orig_kbmode;
};

#endif // QLINUXINPUT_H
