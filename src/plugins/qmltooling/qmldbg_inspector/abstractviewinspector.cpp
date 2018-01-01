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

#include <abstractviewinspector.h>
#include <abstracttool.h>
#include <qdeclarativeinspectorprotocol.h>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeComponent>
#include <qdeclarativedebughelper_p.h>
#include <qdeclarativeinspectorservice_p.h>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QWidget>

namespace QmlJSDebugger {

AbstractViewInspector::AbstractViewInspector(QObject *parent) :
    QObject(parent),
    m_currentTool(0),
    m_showAppOnTop(false),
    m_designModeBehavior(false),
    m_animationPaused(false),
    m_slowDownFactor(1.0),
    m_debugService(0)
{
    m_debugService = QDeclarativeInspectorService::instance();
    connect(m_debugService, SIGNAL(gotMessage(QByteArray)),
            this, SLOT(handleMessage(QByteArray)));
}

void AbstractViewInspector::createQmlObject(const QString &qml, QObject *parent,
                                            const QStringList &importList,
                                            const QString &filename)
{
    if (!parent)
        return;

    QString imports;
    foreach (const QString &s, importList) {
        imports += s;
        imports += QLatin1Char('\n');
    }

    QDeclarativeContext *parentContext = declarativeEngine()->contextForObject(parent);
    QDeclarativeComponent component(declarativeEngine());
    QByteArray constructedQml = QString(imports + qml).toLatin1();

    component.setData(constructedQml, QUrl::fromLocalFile(filename));
    QObject *newObject = component.create(parentContext);
    if (newObject)
        reparentQmlObject(newObject, parent);
}

void AbstractViewInspector::clearComponentCache()
{
    declarativeEngine()->clearComponentCache();
}

void AbstractViewInspector::setDesignModeBehavior(bool value)
{
    if (m_designModeBehavior == value)
        return;

    m_designModeBehavior = value;
    emit designModeBehaviorChanged(value);
    sendDesignModeBehavior(value);
}

void AbstractViewInspector::setAnimationSpeed(qreal slowDownFactor)
{
    Q_ASSERT(slowDownFactor > 0);
    if (m_slowDownFactor == slowDownFactor)
        return;

    animationSpeedChangeRequested(slowDownFactor);
    sendAnimationSpeed(slowDownFactor);
}

void AbstractViewInspector::setAnimationPaused(bool paused)
{
    if (m_animationPaused == paused)
        return;

    animationPausedChangeRequested(paused);
    sendAnimationPaused(paused);
}

void AbstractViewInspector::animationSpeedChangeRequested(qreal factor)
{
    if (m_slowDownFactor != factor) {
        m_slowDownFactor = factor;
        emit animationSpeedChanged(factor);
    }

    const float effectiveFactor = m_animationPaused ? 0 : factor;
    QDeclarativeDebugHelper::setAnimationSlowDownFactor(effectiveFactor);
}

void AbstractViewInspector::animationPausedChangeRequested(bool paused)
{
    if (m_animationPaused != paused) {
        m_animationPaused = paused;
        emit animationPausedChanged(paused);
    }

    const float effectiveFactor = paused ? 0 : m_slowDownFactor;
    QDeclarativeDebugHelper::setAnimationSlowDownFactor(effectiveFactor);
}

void AbstractViewInspector::setShowAppOnTop(bool appOnTop)
{
    if (viewWidget()) {
        QWidget *window = viewWidget()->window();
        Qt::WindowFlags flags = window->windowFlags();
        if (appOnTop)
            flags |= Qt::WindowStaysOnTopHint;
        else
            flags &= ~Qt::WindowStaysOnTopHint;

        window->setWindowFlags(flags);
        window->show();
    }

    m_showAppOnTop = appOnTop;
    sendShowAppOnTop(appOnTop);

    emit showAppOnTopChanged(appOnTop);
}

void AbstractViewInspector::changeToColorPickerTool()
{
    changeTool(InspectorProtocol::ColorPickerTool);
}

void AbstractViewInspector::changeToZoomTool()
{
    changeTool(InspectorProtocol::ZoomTool);
}

void AbstractViewInspector::changeToSingleSelectTool()
{
    changeTool(InspectorProtocol::SelectTool);
}

void AbstractViewInspector::changeToMarqueeSelectTool()
{
    changeTool(InspectorProtocol::SelectMarqueeTool);
}

bool AbstractViewInspector::eventFilter(QObject *obj, QEvent *event)
{
    if (!designModeBehavior())
        return QObject::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::Leave:
        if (leaveEvent(event))
            return true;
        break;
    case QEvent::MouseButtonPress:
        if (mousePressEvent(static_cast<QMouseEvent*>(event)))
            return true;
        break;
    case QEvent::MouseMove:
        if (mouseMoveEvent(static_cast<QMouseEvent*>(event)))
            return true;
        break;
    case QEvent::MouseButtonRelease:
        if (mouseReleaseEvent(static_cast<QMouseEvent*>(event)))
            return true;
        break;
    case QEvent::KeyPress:
        if (keyPressEvent(static_cast<QKeyEvent*>(event)))
            return true;
        break;
    case QEvent::KeyRelease:
        if (keyReleaseEvent(static_cast<QKeyEvent*>(event)))
            return true;
        break;
    case QEvent::MouseButtonDblClick:
        if (mouseDoubleClickEvent(static_cast<QMouseEvent*>(event)))
            return true;
        break;
    case QEvent::Wheel:
        if (wheelEvent(static_cast<QWheelEvent*>(event)))
            return true;
        break;
    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

bool AbstractViewInspector::leaveEvent(QEvent *event)
{
    m_currentTool->leaveEvent(event);
    return true;
}

bool AbstractViewInspector::mousePressEvent(QMouseEvent *event)
{
    m_currentTool->mousePressEvent(event);
    return true;
}

bool AbstractViewInspector::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons()) {
        m_currentTool->mouseMoveEvent(event);
    } else {
        m_currentTool->hoverMoveEvent(event);
    }
    return true;
}

bool AbstractViewInspector::mouseReleaseEvent(QMouseEvent *event)
{
    m_currentTool->mouseReleaseEvent(event);
    return true;
}

bool AbstractViewInspector::keyPressEvent(QKeyEvent *event)
{
    m_currentTool->keyPressEvent(event);
    return true;
}

bool AbstractViewInspector::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_V:
        changeTool(InspectorProtocol::SelectTool);
        break;
// disabled because multiselection does not do anything useful without design mode
//    case Qt::Key_M:
//        changeTool(InspectorProtocol::SelectMarqueeTool);
//        break;
    case Qt::Key_I:
        changeTool(InspectorProtocol::ColorPickerTool);
        break;
    case Qt::Key_Z:
        changeTool(InspectorProtocol::ZoomTool);
        break;
    case Qt::Key_Space:
        setAnimationPaused(!animationPaused());
        break;
    default:
        break;
    }

    m_currentTool->keyReleaseEvent(event);
    return true;
}

bool AbstractViewInspector::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_currentTool->mouseDoubleClickEvent(event);
    return true;
}

bool AbstractViewInspector::wheelEvent(QWheelEvent *event)
{
    m_currentTool->wheelEvent(event);
    return true;
}

void AbstractViewInspector::handleMessage(const QByteArray &message)
{
    QDataStream ds(message);

    InspectorProtocol::Message type;
    ds >> type;

    switch (type) {
    case InspectorProtocol::SetCurrentObjects: {
        int itemCount = 0;
        ds >> itemCount;

        QList<QObject*> selectedObjects;
        for (int i = 0; i < itemCount; ++i) {
            int debugId = -1;
            ds >> debugId;
            if (QObject *obj = QDeclarativeDebugService::objectForId(debugId))
                selectedObjects << obj;
        }

        changeCurrentObjects(selectedObjects);
        break;
    }
    case InspectorProtocol::Reload: {
        reloadView();
        break;
    }
    case InspectorProtocol::SetAnimationSpeed: {
        qreal speed;
        ds >> speed;
        animationSpeedChangeRequested(speed);
        break;
    }
    case InspectorProtocol::SetAnimationPaused: {
        bool paused;
        ds >> paused;
        animationPausedChangeRequested(paused);
        break;
    }
    case InspectorProtocol::ChangeTool: {
        InspectorProtocol::Tool tool;
        ds >> tool;
        changeTool(tool);
        break;
    }
    case InspectorProtocol::SetDesignMode: {
        bool inDesignMode;
        ds >> inDesignMode;
        setDesignModeBehavior(inDesignMode);
        break;
    }
    case InspectorProtocol::ShowAppOnTop: {
        bool showOnTop;
        ds >> showOnTop;
        setShowAppOnTop(showOnTop);
        break;
    }
    case InspectorProtocol::CreateObject: {
        QString qml;
        int parentId;
        QString filename;
        QStringList imports;
        ds >> qml >> parentId >> imports >> filename;
        createQmlObject(qml, QDeclarativeDebugService::objectForId(parentId),
                        imports, filename);
        break;
    }
    case InspectorProtocol::DestroyObject: {
        int debugId;
        ds >> debugId;
        if (QObject *obj = QDeclarativeDebugService::objectForId(debugId))
            obj->deleteLater();
        break;
    }
    case InspectorProtocol::MoveObject: {
        int debugId, newParent;
        ds >> debugId >> newParent;
        reparentQmlObject(QDeclarativeDebugService::objectForId(debugId),
                          QDeclarativeDebugService::objectForId(newParent));
        break;
    }
    case InspectorProtocol::ObjectIdList: {
        int itemCount;
        ds >> itemCount;
        m_stringIdForObjectId.clear();
        for (int i = 0; i < itemCount; ++i) {
            int itemDebugId;
            QString itemIdString;
            ds >> itemDebugId
               >> itemIdString;

            m_stringIdForObjectId.insert(itemDebugId, itemIdString);
        }
        break;
    }
    case InspectorProtocol::ClearComponentCache: {
        clearComponentCache();
        break;
    }
    default:
        qWarning() << "Warning: Not handling message:" << type;
    }
}

void AbstractViewInspector::sendDesignModeBehavior(bool inDesignMode)
{
    QByteArray message;
    QDataStream ds(&message, QIODevice::WriteOnly);

    ds << InspectorProtocol::SetDesignMode
       << inDesignMode;

    m_debugService->sendMessage(message);
}

void AbstractViewInspector::sendCurrentObjects(const QList<QObject*> &objects)
{
    QByteArray message;
    QDataStream ds(&message, QIODevice::WriteOnly);

    ds << InspectorProtocol::CurrentObjectsChanged
       << objects.length();

    foreach (QObject *object, objects) {
        int id = QDeclarativeDebugService::idForObject(object);
        ds << id;
    }

    m_debugService->sendMessage(message);
}

void AbstractViewInspector::sendCurrentTool(Constants::DesignTool toolId)
{
    QByteArray message;
    QDataStream ds(&message, QIODevice::WriteOnly);

    ds << InspectorProtocol::ToolChanged
       << toolId;

    m_debugService->sendMessage(message);
}

void AbstractViewInspector::sendAnimationSpeed(qreal slowDownFactor)
{
    QByteArray message;
    QDataStream ds(&message, QIODevice::WriteOnly);

    ds << InspectorProtocol::AnimationSpeedChanged
       << slowDownFactor;

    m_debugService->sendMessage(message);
}

void AbstractViewInspector::sendAnimationPaused(bool paused)
{
    QByteArray message;
    QDataStream ds(&message, QIODevice::WriteOnly);

    ds << InspectorProtocol::AnimationPausedChanged
       << paused;

    m_debugService->sendMessage(message);
}

void AbstractViewInspector::sendReloaded()
{
    QByteArray message;
    QDataStream ds(&message, QIODevice::WriteOnly);

    ds << InspectorProtocol::Reloaded;

    m_debugService->sendMessage(message);
}

void AbstractViewInspector::sendShowAppOnTop(bool showAppOnTop)
{
    QByteArray message;
    QDataStream ds(&message, QIODevice::WriteOnly);

    ds << InspectorProtocol::ShowAppOnTop << showAppOnTop;

    m_debugService->sendMessage(message);
}

void AbstractViewInspector::sendColorChanged(const QColor &color)
{
    QByteArray message;
    QDataStream ds(&message, QIODevice::WriteOnly);

    ds << InspectorProtocol::ColorChanged
       << color;

    m_debugService->sendMessage(message);
}

QString AbstractViewInspector::idStringForObject(QObject *obj) const
{
    const int id = QDeclarativeDebugService::idForObject(obj);
    return m_stringIdForObjectId.value(id);
}

} // namespace QmlJSDebugger

