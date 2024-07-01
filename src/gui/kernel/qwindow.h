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

#ifndef QWINDOW_H
#define QWINDOW_H

#include <qobject.h>
#include <qevent.h>
#include <qmargins.h>
#include <qrect.h>
#include <qnamespace.h>
#include <qsurface.h>
#include <qsurfaceformat.h>
#include <qwindowdefs.h>
#include <qicon.h>

#ifndef QT_NO_CURSOR
#include <qcursor.h>
#endif

class QWindowPrivate;
class QExposeEvent;
class QFocusEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;
class QHideEvent;
class QKeyEvent;
class QMouseEvent;

#ifndef QT_NO_WHEELEVENT
class QWheelEvent;
#endif

class QTouchEvent;

#ifndef QT_NO_TABLETEVENT
class QTabletEvent;
#endif

class QPlatformSurface;
class QPlatformWindow;
class QBackingStore;
class QScreen;
class QAccessibleInterface;
class QWindowContainer;
class QDebug;
class QVulkanInstance;

class Q_GUI_EXPORT QWindow : public QObject, public QSurface
{
   GUI_CS_OBJECT_MULTIPLE(QWindow, QObject)

   GUI_CS_ENUM(Visibility)

   GUI_CS_PROPERTY_READ(title, title)
   GUI_CS_PROPERTY_WRITE(title, setTitle)
   GUI_CS_PROPERTY_NOTIFY(title, windowTitleChanged)

   GUI_CS_PROPERTY_READ(modality, modality)
   GUI_CS_PROPERTY_WRITE(modality, setModality)
   GUI_CS_PROPERTY_NOTIFY(modality, modalityChanged)

   GUI_CS_PROPERTY_READ(flags, flags)
   GUI_CS_PROPERTY_WRITE(flags, setFlags)

   GUI_CS_PROPERTY_READ(x, x)
   GUI_CS_PROPERTY_WRITE(x, setX)
   GUI_CS_PROPERTY_NOTIFY(x, xChanged)

   GUI_CS_PROPERTY_READ(y, y)
   GUI_CS_PROPERTY_WRITE(y, setY)
   GUI_CS_PROPERTY_NOTIFY(y, yChanged)

   GUI_CS_PROPERTY_READ(width, width)
   GUI_CS_PROPERTY_WRITE(width, setWidth)
   GUI_CS_PROPERTY_NOTIFY(width, widthChanged)
   GUI_CS_PROPERTY_READ(height, height)
   GUI_CS_PROPERTY_WRITE(height, setHeight)
   GUI_CS_PROPERTY_NOTIFY(height, heightChanged)
   GUI_CS_PROPERTY_READ(minimumWidth, minimumWidth)
   GUI_CS_PROPERTY_WRITE(minimumWidth, setMinimumWidth)
   GUI_CS_PROPERTY_NOTIFY(minimumWidth, minimumWidthChanged)
   GUI_CS_PROPERTY_READ(minimumHeight, minimumHeight)
   GUI_CS_PROPERTY_WRITE(minimumHeight, setMinimumHeight)
   GUI_CS_PROPERTY_NOTIFY(minimumHeight, minimumHeightChanged)
   GUI_CS_PROPERTY_READ(maximumWidth, maximumWidth)
   GUI_CS_PROPERTY_WRITE(maximumWidth, setMaximumWidth)
   GUI_CS_PROPERTY_NOTIFY(maximumWidth, maximumWidthChanged)
   GUI_CS_PROPERTY_READ(maximumHeight, maximumHeight)
   GUI_CS_PROPERTY_WRITE(maximumHeight, setMaximumHeight)
   GUI_CS_PROPERTY_NOTIFY(maximumHeight, maximumHeightChanged)
   GUI_CS_PROPERTY_READ(visible, isVisible)
   GUI_CS_PROPERTY_WRITE(visible, setVisible)
   GUI_CS_PROPERTY_NOTIFY(visible, visibleChanged)
   GUI_CS_PROPERTY_READ(active, isActive)
   GUI_CS_PROPERTY_NOTIFY(active, activeChanged)
   GUI_CS_PROPERTY_REVISION(active, 1)
   GUI_CS_PROPERTY_READ(visibility, visibility)
   GUI_CS_PROPERTY_WRITE(visibility, setVisibility)
   GUI_CS_PROPERTY_NOTIFY(visibility, visibilityChanged)
   GUI_CS_PROPERTY_REVISION(visibility, 1)
   GUI_CS_PROPERTY_READ(contentOrientation, contentOrientation)
   GUI_CS_PROPERTY_WRITE(contentOrientation, reportContentOrientationChange)
   GUI_CS_PROPERTY_NOTIFY(contentOrientation, contentOrientationChanged)
   GUI_CS_PROPERTY_READ(opacity, opacity)
   GUI_CS_PROPERTY_WRITE(opacity, setOpacity)
   GUI_CS_PROPERTY_NOTIFY(opacity, opacityChanged)
   GUI_CS_PROPERTY_REVISION(opacity, 1)

 public:
   enum Visibility {
      Hidden = 0,
      AutomaticVisibility,
      Windowed,
      Minimized,
      Maximized,
      FullScreen
   };

   enum AncestorMode {
      ExcludeTransients,
      IncludeTransients
   };

   explicit QWindow(QScreen *screen = nullptr);
   explicit QWindow(QWindow *parent);

   QWindow(const QWindow &) = delete;
   QWindow &operator=(const QWindow &) = delete;

   virtual ~QWindow();

   void setSurfaceType(SurfaceType surfaceType);
   SurfaceType surfaceType() const override;

   bool isVisible() const;

   Visibility visibility() const;
   void setVisibility(Visibility v);

   void create();

   WId winId() const;

   QWindow *parent() const;
   void setParent(QWindow *parent);

   bool isTopLevel() const;

   bool isModal() const;
   Qt::WindowModality modality() const;
   void setModality(Qt::WindowModality modality);

   void setFormat(const QSurfaceFormat &format);
   QSurfaceFormat format() const override;
   QSurfaceFormat requestedFormat() const;

   void setFlags(Qt::WindowFlags flags);
   Qt::WindowFlags flags() const;
   Qt::WindowType type() const;

   QString title() const;

   void setOpacity(qreal level);
   qreal opacity() const;

   void setMask(const QRegion &region);
   QRegion mask() const;

   bool isActive() const;

   void reportContentOrientationChange(Qt::ScreenOrientation orientation);
   Qt::ScreenOrientation contentOrientation() const;

   qreal devicePixelRatio() const;

   Qt::WindowState windowState() const;
   void setWindowState(Qt::WindowState state);

   void setTransientParent(QWindow *parent);
   QWindow *transientParent() const;

   bool isAncestorOf(const QWindow *child, AncestorMode mode = IncludeTransients) const;
   bool isExposed() const;

   int minimumWidth() const {
      return minimumSize().width();
   }

   int minimumHeight() const {
      return minimumSize().height();
   }

   int maximumWidth() const {
      return maximumSize().width();
   }

   int maximumHeight() const {
      return maximumSize().height();
   }

   QSize minimumSize() const;
   QSize maximumSize() const;
   QSize baseSize() const;
   QSize sizeIncrement() const;

   void setMinimumSize(const QSize &size);
   void setMaximumSize(const QSize &size);
   void setBaseSize(const QSize &size);
   void setSizeIncrement(const QSize &size);

   void setGeometry(int x_pos, int y_pos, int w, int h);
   void setGeometry(const QRect &rect);
   QRect geometry() const;

   QMargins frameMargins() const;
   QRect frameGeometry() const;

   QPoint framePosition() const;
   void setFramePosition(const QPoint &point);

   int width() const {
      return geometry().width();
   }

   int height() const {
      return geometry().height();
   }

   int x() const {
      return geometry().x();
   }

   int y() const {
      return geometry().y();
   }

   QSize size() const override {
      return geometry().size();
   }

   QPoint position() const {
      return geometry().topLeft();
   }

   void setPosition(const QPoint &pt);
   void setPosition(int x_pos, int y_pos);

   void resize(const QSize &newSize);
   void resize(int w, int h);

   void setFilePath(const QString &filePath);
   QString filePath() const;

   void setIcon(const QIcon &icon);
   QIcon icon() const;

   void destroy();

   QPlatformWindow *handle() const;

   bool setKeyboardGrabEnabled(bool grab);
   bool setMouseGrabEnabled(bool grab);

   QScreen *screen() const;
   void setScreen(QScreen *screen);

   virtual QAccessibleInterface *accessibleRoot() const;
   virtual QObject *focusObject() const;

   QPoint mapToGlobal(const QPoint &pos) const;
   QPoint mapFromGlobal(const QPoint &pos) const;

#ifndef QT_NO_CURSOR
   QCursor cursor() const;
   void setCursor(const QCursor &cursor);
   void unsetCursor();
#endif

   void setVulkanInstance(QVulkanInstance* instance);
   QVulkanInstance* vulkanInstance() const;

   void cs_internal_updateTimer(int value);

   static QWindow *fromWinId(WId id);

   GUI_CS_SLOT_1(Public, void requestActivate())
   GUI_CS_SLOT_2(requestActivate)

   GUI_CS_SLOT_1(Public, void setVisible(bool visible))
   GUI_CS_SLOT_2(setVisible)

   GUI_CS_SLOT_1(Public, void show())
   GUI_CS_SLOT_2(show)
   GUI_CS_SLOT_1(Public, void hide())
   GUI_CS_SLOT_2(hide)

   GUI_CS_SLOT_1(Public, void showMinimized())
   GUI_CS_SLOT_2(showMinimized)
   GUI_CS_SLOT_1(Public, void showMaximized())
   GUI_CS_SLOT_2(showMaximized)
   GUI_CS_SLOT_1(Public, void showFullScreen())
   GUI_CS_SLOT_2(showFullScreen)
   GUI_CS_SLOT_1(Public, void showNormal())
   GUI_CS_SLOT_2(showNormal)

   GUI_CS_SLOT_1(Public, bool close())
   GUI_CS_SLOT_2(close)
   GUI_CS_SLOT_1(Public, void raise())
   GUI_CS_SLOT_2(raise)
   GUI_CS_SLOT_1(Public, void lower())
   GUI_CS_SLOT_2(lower)

   GUI_CS_SLOT_1(Public, void setTitle(const QString &title))
   GUI_CS_SLOT_2(setTitle)

   GUI_CS_SLOT_1(Public, void setX(int x_value))
   GUI_CS_SLOT_2(setX)
   GUI_CS_SLOT_1(Public, void setY(int y_value))
   GUI_CS_SLOT_2(setY)
   GUI_CS_SLOT_1(Public, void setWidth(int width))
   GUI_CS_SLOT_2(setWidth)
   GUI_CS_SLOT_1(Public, void setHeight(int height))
   GUI_CS_SLOT_2(setHeight)

   GUI_CS_SLOT_1(Public, void setMinimumWidth(int width))
   GUI_CS_SLOT_2(setMinimumWidth)
   GUI_CS_SLOT_1(Public, void setMinimumHeight(int height))
   GUI_CS_SLOT_2(setMinimumHeight)
   GUI_CS_SLOT_1(Public, void setMaximumWidth(int width))
   GUI_CS_SLOT_2(setMaximumWidth)
   GUI_CS_SLOT_1(Public, void setMaximumHeight(int height))
   GUI_CS_SLOT_2(setMaximumHeight)

   GUI_CS_SLOT_1(Public, void alert(int msec))
   GUI_CS_SLOT_2(alert)

   GUI_CS_SLOT_1(Public, void requestUpdate())
   GUI_CS_SLOT_2(requestUpdate)

   GUI_CS_SIGNAL_1(Public, void screenChanged(QScreen *screen))
   GUI_CS_SIGNAL_2(screenChanged, screen)
   GUI_CS_SIGNAL_1(Public, void modalityChanged(Qt::WindowModality modality))
   GUI_CS_SIGNAL_2(modalityChanged, modality)
   GUI_CS_SIGNAL_1(Public, void windowStateChanged(Qt::WindowState windowState))
   GUI_CS_SIGNAL_2(windowStateChanged, windowState)
   GUI_CS_SIGNAL_1(Public, void windowTitleChanged(const QString &newTitle))
   GUI_CS_SIGNAL_2(windowTitleChanged, newTitle)

   GUI_CS_SIGNAL_1(Public, void xChanged(int newX))
   GUI_CS_SIGNAL_2(xChanged, newX)
   GUI_CS_SIGNAL_1(Public, void yChanged(int newY))
   GUI_CS_SIGNAL_2(yChanged, newY)

   GUI_CS_SIGNAL_1(Public, void widthChanged(int newWidth))
   GUI_CS_SIGNAL_2(widthChanged, newWidth)
   GUI_CS_SIGNAL_1(Public, void heightChanged(int newHeight))
   GUI_CS_SIGNAL_2(heightChanged, newHeight)

   GUI_CS_SIGNAL_1(Public, void minimumWidthChanged(int newMinWidth))
   GUI_CS_SIGNAL_2(minimumWidthChanged, newMinWidth)
   GUI_CS_SIGNAL_1(Public, void minimumHeightChanged(int newMinHeight))
   GUI_CS_SIGNAL_2(minimumHeightChanged, newMinHeight)
   GUI_CS_SIGNAL_1(Public, void maximumWidthChanged(int newMaxWidth))
   GUI_CS_SIGNAL_2(maximumWidthChanged, newMaxWidth)
   GUI_CS_SIGNAL_1(Public, void maximumHeightChanged(int newMaxHeight))
   GUI_CS_SIGNAL_2(maximumHeightChanged, newMaxHeight)

   GUI_CS_SIGNAL_1(Public, void visibleChanged(bool newVisible))
   GUI_CS_SIGNAL_2(visibleChanged, newVisible)
   GUI_CS_SIGNAL_1(Public, void visibilityChanged(QWindow::Visibility newVisibility))
   GUI_CS_SIGNAL_2(visibilityChanged, newVisibility)
   GUI_CS_SIGNAL_1(Public, void activeChanged())
   GUI_CS_SIGNAL_2(activeChanged)
   GUI_CS_SIGNAL_1(Public, void contentOrientationChanged(Qt::ScreenOrientation orientation))
   GUI_CS_SIGNAL_2(contentOrientationChanged, orientation)

   GUI_CS_SIGNAL_1(Public, void focusObjectChanged(QObject *object))
   GUI_CS_SIGNAL_2(focusObjectChanged, object)

   GUI_CS_SIGNAL_1(Public, void opacityChanged(qreal newOpacity))
   GUI_CS_SIGNAL_2(opacityChanged, newOpacity)

 protected:
   virtual void exposeEvent(QExposeEvent *event);
   virtual void resizeEvent(QResizeEvent *event);
   virtual void moveEvent(QMoveEvent *event);
   virtual void focusInEvent(QFocusEvent *event);
   virtual void focusOutEvent(QFocusEvent *event);

   virtual void showEvent(QShowEvent *event);
   virtual void hideEvent(QHideEvent *event);

   // emerald  add closeEvent virtual handler

   bool event(QEvent *event) override;
   bool cs_isWindowType() const override;

   virtual void keyPressEvent(QKeyEvent *event);
   virtual void keyReleaseEvent(QKeyEvent *event);
   virtual void mousePressEvent(QMouseEvent *event);
   virtual void mouseReleaseEvent(QMouseEvent *event);
   virtual void mouseDoubleClickEvent(QMouseEvent *event);
   virtual void mouseMoveEvent(QMouseEvent *event);

#ifndef QT_NO_WHEELEVENT
   virtual void wheelEvent(QWheelEvent *event);
#endif

   virtual void touchEvent(QTouchEvent *event);

#ifndef QT_NO_TABLETEVENT
   virtual void tabletEvent(QTabletEvent *event);
#endif

   virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);

   QWindow(QWindowPrivate &dd, QWindow *parent);

 protected:
   QScopedPointer<QWindowPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QWindow)

   QPlatformSurface *surfaceHandle() const override;

   QVulkanInstance* m_vulkanInstance;

   GUI_CS_SLOT_1(Private, void _q_clearAlert())
   GUI_CS_SLOT_2(_q_clearAlert)

   friend class QApplication;
   friend class QApplicationPrivate;
   friend class QWindowContainer;

   friend Q_GUI_EXPORT QWindowPrivate *qt_window_private(QWindow *window);
};

Q_GUI_EXPORT QDebug operator<<(QDebug, const QWindow *);

#endif
