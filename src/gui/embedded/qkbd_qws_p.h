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

#ifndef QKBD_QWS_P_H
#define QKBD_QWS_P_H

#include <QDataStream>

QT_BEGIN_NAMESPACE

namespace QWSKeyboard {
const quint32 FileMagic = 0x514d4150; // 'QMAP'

struct Mapping {
   quint16 keycode;
   quint16 unicode;
   quint32 qtcode;
   quint8 modifiers;
   quint8 flags;
   quint16 special;

};

enum Flags {
   IsDead     = 0x01,
   IsLetter   = 0x02,
   IsModifier = 0x04,
   IsSystem   = 0x08,
};

enum System {
   SystemConsoleFirst    = 0x0100,
   SystemConsoleMask     = 0x007f,
   SystemConsoleLast     = 0x017f,
   SystemConsolePrevious = 0x0180,
   SystemConsoleNext     = 0x0181,
   SystemReboot          = 0x0200,
   SystemZap             = 0x0300,
};

struct Composing {
   quint16 first;
   quint16 second;
   quint16 result;
};

enum Modifiers {
   ModPlain   = 0x00,
   ModShift   = 0x01,
   ModAltGr   = 0x02,
   ModControl = 0x04,
   ModAlt     = 0x08,
   ModShiftL  = 0x10,
   ModShiftR  = 0x20,
   ModCtrlL   = 0x40,
   ModCtrlR   = 0x80,
   // ModCapsShift = 0x100, // not supported!
};
};

#ifndef QT_NO_DATASTREAM
inline QDataStream &operator>>(QDataStream &ds, QWSKeyboard::Mapping &m)
{
   return ds >> m.keycode >> m.unicode >> m.qtcode >> m.modifiers >> m.flags >> m.special;
}

inline QDataStream &operator<<(QDataStream &ds, const QWSKeyboard::Mapping &m)
{
   return ds << m.keycode << m.unicode << m.qtcode << m.modifiers << m.flags << m.special;
}

inline QDataStream &operator>>(QDataStream &ds, QWSKeyboard::Composing &c)
{
   return ds >> c.first >> c.second >> c.result;
}

inline QDataStream &operator<<(QDataStream &ds, const QWSKeyboard::Composing &c)
{
   return ds << c.first << c.second << c.result;
}
#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE

#endif // QWSKEYBOARD_H
