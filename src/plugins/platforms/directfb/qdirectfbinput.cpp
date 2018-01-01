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

#include "qdirectfbinput.h"
#include "qdirectfbconvenience.h"

#include <QThread>
#include <QDebug>
#include <QWindowSystemInterface>
#include <QMouseEvent>
#include <QEvent>

#include <directfb.h>

QT_BEGIN_NAMESPACE

QDirectFbInput::QDirectFbInput(IDirectFB *dfb, IDirectFBDisplayLayer *dfbLayer)
    : m_dfbInterface(dfb)
    , m_dfbDisplayLayer(dfbLayer)
    , m_shouldStop(false)
{
    DFBResult ok = m_dfbInterface->CreateEventBuffer(m_dfbInterface, m_eventBuffer.outPtr());
    if (ok != DFB_OK)
        DirectFBError("Failed to initialise eventbuffer", ok);
}

void QDirectFbInput::run()
{
    while (!m_shouldStop) {
        if (m_eventBuffer->WaitForEvent(m_eventBuffer.data()) == DFB_OK)
            handleEvents();
    }
}

void QDirectFbInput::stopInputEventLoop()
{
    m_shouldStop = true;
    m_eventBuffer->WakeUp(m_eventBuffer.data());
}

void QDirectFbInput::addWindow(IDirectFBWindow *window, QWidget *platformWindow)
{
    DFBResult res;
    DFBWindowID id;

    res = window->GetID(window, &id);
    if (res != DFB_OK) {
        DirectFBError("QDirectFbInput::addWindow", res);
        return;
    }

    m_tlwMap.insert(id, platformWindow);
    window->AttachEventBuffer(window, m_eventBuffer.data());
}

void QDirectFbInput::removeWindow(IDirectFBWindow *window)
{
    DFBResult res;
    DFBWindowID id;

    res = window->GetID(window, &id);
    if (res != DFB_OK) {
        DirectFBError("QDirectFbInput::removeWindow", res);
        return;
    }

    window->DetachEventBuffer(window, m_eventBuffer.data());
    m_tlwMap.remove(id);
}

void QDirectFbInput::handleEvents()
{
    DFBResult hasEvent = m_eventBuffer->HasEvent(m_eventBuffer.data());
    while(hasEvent == DFB_OK){
        DFBEvent event;
        DFBResult ok = m_eventBuffer->GetEvent(m_eventBuffer.data(), &event);
        if (ok != DFB_OK)
            DirectFBError("Failed to get event",ok);
        if (event.clazz == DFEC_WINDOW) {
            switch (event.window.type) {
            case DWET_BUTTONDOWN:
            case DWET_BUTTONUP:
            case DWET_MOTION:
                handleMouseEvents(event);
                break;
            case DWET_WHEEL:
                handleWheelEvent(event);
                break;
            case DWET_KEYDOWN:
            case DWET_KEYUP:
                handleKeyEvents(event);
                break;
            case DWET_ENTER:
            case DWET_LEAVE:
                handleEnterLeaveEvents(event);
            default:
                break;
            }

        }

        hasEvent = m_eventBuffer->HasEvent(m_eventBuffer.data());
    }
}

void QDirectFbInput::handleMouseEvents(const DFBEvent &event)
{
    QPoint p(event.window.x, event.window.y);
    QPoint globalPos = globalPoint(event);
    Qt::MouseButtons buttons = QDirectFbConvenience::mouseButtons(event.window.buttons);

    QDirectFBPointer<IDirectFBDisplayLayer> layer(QDirectFbConvenience::dfbDisplayLayer());
    QDirectFBPointer<IDirectFBWindow> window;
    layer->GetWindow(layer.data(), event.window.window_id, window.outPtr());

    long timestamp = (event.window.timestamp.tv_sec*1000) + (event.window.timestamp.tv_usec/1000);

    QWidget *tlw = m_tlwMap.value(event.window.window_id);
    QWindowSystemInterface::handleMouseEvent(tlw, timestamp, p, globalPos, buttons);
}

void QDirectFbInput::handleWheelEvent(const DFBEvent &event)
{
    QPoint p(event.window.cx, event.window.cy);
    QPoint globalPos = globalPoint(event);
    long timestamp = (event.window.timestamp.tv_sec*1000) + (event.window.timestamp.tv_usec/1000);
    QWidget *tlw = m_tlwMap.value(event.window.window_id);
    QWindowSystemInterface::handleWheelEvent(tlw, timestamp, p, globalPos,
                                          event.window.step*120,
                                          Qt::Vertical);
}

void QDirectFbInput::handleKeyEvents(const DFBEvent &event)
{
    QEvent::Type type = QDirectFbConvenience::eventType(event.window.type);
    Qt::Key key = QDirectFbConvenience::keyMap()->value(event.window.key_symbol);
    Qt::KeyboardModifiers modifiers = QDirectFbConvenience::keyboardModifiers(event.window.modifiers);

    long timestamp = (event.window.timestamp.tv_sec*1000) + (event.window.timestamp.tv_usec/1000);

    QChar character;
    if (DFB_KEY_TYPE(event.window.key_symbol) == DIKT_UNICODE)
        character = QChar(event.window.key_symbol);
    QWidget *tlw = m_tlwMap.value(event.window.window_id);
    QWindowSystemInterface::handleKeyEvent(tlw, timestamp, type, key, modifiers, character);
}

void QDirectFbInput::handleEnterLeaveEvents(const DFBEvent &event)
{
    QWidget *tlw = m_tlwMap.value(event.window.window_id);
    switch (event.window.type) {
    case DWET_ENTER:
        QWindowSystemInterface::handleEnterEvent(tlw);
        break;
    case DWET_LEAVE:
        QWindowSystemInterface::handleLeaveEvent(tlw);
        break;
    default:
        break;
    }
}

inline QPoint QDirectFbInput::globalPoint(const DFBEvent &event) const
{
    QDirectFBPointer<IDirectFBWindow> window;
    m_dfbDisplayLayer->GetWindow(m_dfbDisplayLayer, event.window.window_id, window.outPtr());
    int x,y;
    window->GetPosition(window.data(), &x, &y);
    return QPoint(event.window.cx +x, event.window.cy + y);
}

QT_END_NAMESPACE
