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

#ifndef QSIMPLEDRAG_P_H
#define QSIMPLEDRAG_P_H

#include <qplatform_drag.h>
#include <qobject.h>



#ifndef QT_NO_DRAGANDDROP

class QMouseEvent;
class QWindow;
class QEventLoop;
class QDropData;
class QShapedPixmapWindow;
class QScreen;

class Q_GUI_EXPORT QBasicDrag : public QPlatformDrag, public QObject
{
public:
    virtual ~QBasicDrag();

    virtual Qt::DropAction drag(QDrag *drag) override;

    virtual bool eventFilter(QObject *o, QEvent *e) override;

protected:
    QBasicDrag();

    virtual void startDrag();
    virtual void cancel();
    virtual void move(const QPoint &globalPos) = 0;
    virtual void drop(const QPoint &globalPos) = 0;
    virtual void endDrag();


    void moveShapedPixmapWindow(const QPoint &deviceIndependentPosition);
    QShapedPixmapWindow *shapedPixmapWindow() const { return m_drag_icon_window; }
    void recreateShapedPixmapWindow(QScreen *screen, const QPoint &pos);
    void updateCursor(Qt::DropAction action);

    bool canDrop() const { return m_can_drop; }
    void setCanDrop(bool c) { m_can_drop = c; }

    bool useCompositing() const { return m_useCompositing; }
    void setUseCompositing(bool on) { m_useCompositing = on; }

    void setScreen(QScreen *screen) { m_screen = screen; }

    Qt::DropAction executedDropAction() const { return m_executed_drop_action; }
    void  setExecutedDropAction(Qt::DropAction da) { m_executed_drop_action = da; }

    QDrag *drag() const { return m_drag; }

private:
    void enableEventFilter();
    void disableEventFilter();
    void restoreCursor();
    void exitDndEventLoop();

    bool m_restoreCursor;
    QEventLoop *m_eventLoop;
    Qt::DropAction m_executed_drop_action;
    bool m_can_drop;
    QDrag *m_drag;
    QShapedPixmapWindow *m_drag_icon_window;
    bool m_useCompositing;
    QScreen *m_screen;
};

class Q_GUI_EXPORT QSimpleDrag : public QBasicDrag
{
public:
    QSimpleDrag();
    virtual QMimeData *platformDropData() override;

protected:
    virtual void startDrag() override;
    virtual void cancel() override;
    virtual void move(const QPoint &globalPos) override;
    virtual void drop(const QPoint &globalPos) override;

private:
    QWindow *m_current_window;
};

#endif // QT_NO_DRAGANDDROP


#endif
