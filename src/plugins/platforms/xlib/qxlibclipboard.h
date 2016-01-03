/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QTESTLITECLIPBOARD_H
#define QTESTLITECLIPBOARD_H

#include <QPlatformClipboard>
#include "qxlibstatic.h"

class QXlibScreen;
class QXlibClipboard : public QPlatformClipboard
{
public:
    QXlibClipboard(QXlibScreen *screen);

    QMimeData *mimeData(QClipboard::Mode mode);
    void setMimeData(QMimeData *data, QClipboard::Mode mode);

    bool supportsMode(QClipboard::Mode mode) const;

    QXlibScreen *screen() const;

    Window requestor() const;
    void setRequestor(Window window);

    Window owner() const;

    void handleSelectionRequest(XEvent *event);

    bool clipboardReadProperty(Window win, Atom property, bool deleteProperty, QByteArray *buffer, int *size, Atom *type, int *format) const;
    QByteArray clipboardReadIncrementalProperty(Window win, Atom property, int nbytes, bool nullterm);

    QByteArray getDataInFormat(Atom modeAtom, Atom fmtatom);

private:
    void setOwner(Window window);

    Atom sendTargetsSelection(QMimeData *d, Window window, Atom property);
    Atom sendSelection(QMimeData *d, Atom target, Window window, Atom property);

    QXlibScreen *m_screen;

    QMimeData *m_xClipboard;
    QMimeData *m_clientClipboard;

    QMimeData *m_xSelection;
    QMimeData *m_clientSelection;

    Window m_requestor;
    Window m_owner;

    static const int clipboard_timeout;

};

#endif // QTESTLITECLIPBOARD_H
