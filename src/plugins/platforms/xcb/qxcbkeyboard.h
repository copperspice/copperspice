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

#ifndef QXCBKEYBOARD_H
#define QXCBKEYBOARD_H

#include "qxcbobject.h"

#include "xcb/xcb_keysyms.h"

#include <QEvent>

class QXcbKeyboard : public QXcbObject
{
public:
    QXcbKeyboard(QXcbConnection *connection);
    ~QXcbKeyboard();

    void handleKeyPressEvent(QWidget *widget, const xcb_key_press_event_t *event);
    void handleKeyReleaseEvent(QWidget *widget, const xcb_key_release_event_t *event);

    void handleMappingNotifyEvent(const xcb_mapping_notify_event_t *event);

    Qt::KeyboardModifiers translateModifiers(int s);

private:
    void handleKeyEvent(QWidget *widget, QEvent::Type type, xcb_keycode_t code, quint16 state, xcb_timestamp_t time);

    int translateKeySym(uint key) const;
    QString translateKeySym(xcb_keysym_t keysym, uint xmodifiers,
                            int &code, Qt::KeyboardModifiers &modifiers,
                            QByteArray &chars, int &count);

    uint m_alt_mask;
    uint m_super_mask;
    uint m_hyper_mask;
    uint m_meta_mask;
    uint m_mode_switch_mask;
    uint m_num_lock_mask;

    xcb_key_symbols_t *m_key_symbols;
};

#endif
