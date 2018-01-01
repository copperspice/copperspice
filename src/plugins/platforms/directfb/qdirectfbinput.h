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

#ifndef QDIRECTFBINPUT_H
#define QDIRECTFBINPUT_H

#include <QThread>
#include <QHash>
#include <QPoint>
#include <QEvent>

#include <QtGui/qwindowdefs.h>

#include "qdirectfbconvenience.h"

class QDirectFbInput : public QThread
{
    Q_OBJECT
public:
    QDirectFbInput(IDirectFB *dfb, IDirectFBDisplayLayer *dfbLayer);
    void addWindow(IDirectFBWindow *window, QWidget *platformWindow);
    void removeWindow(IDirectFBWindow *window);

    void stopInputEventLoop();

protected:
    void run();

private:
    void handleEvents();
    void handleMouseEvents(const DFBEvent &event);
    void handleWheelEvent(const DFBEvent &event);
    void handleKeyEvents(const DFBEvent &event);
    void handleEnterLeaveEvents(const DFBEvent &event);
    inline QPoint globalPoint(const DFBEvent &event) const;


    IDirectFB *m_dfbInterface;
    IDirectFBDisplayLayer *m_dfbDisplayLayer;
    QDirectFBPointer<IDirectFBEventBuffer> m_eventBuffer;

    bool m_shouldStop;
    QHash<DFBWindowID,QWidget *>m_tlwMap;
};

#endif // QDIRECTFBINPUT_H
