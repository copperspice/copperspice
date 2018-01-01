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

#ifndef QTESTLITEKEYBOARD_H
#define QTESTLITEKEYBOARD_H

#include "qxlibintegration.h"

class QXlibKeyboard
{
public:
    QXlibKeyboard(QXlibScreen *screen);

    void changeLayout();

    void handleKeyEvent(QWidget *widget, QEvent::Type type, XKeyEvent *ev);

    Qt::KeyboardModifiers translateModifiers(int s);

private:

    void setMask(KeySym sym, uint mask);
    int translateKeySym(uint key) const;
    QString translateKeySym(KeySym keysym, uint xmodifiers,
                                   int &code, Qt::KeyboardModifiers &modifiers,
                                   QByteArray &chars, int &count);

    QXlibScreen *m_screen;

    uint m_alt_mask;
    uint m_super_mask;
    uint m_hyper_mask;
    uint m_meta_mask;
    uint m_mode_switch_mask;
    uint m_num_lock_mask;
};

#endif // QTESTLITEKEYBOARD_H
