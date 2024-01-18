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

#ifndef QMDISUBWINDOW_P_H
#define QMDISUBWINDOW_P_H

#include <qmdisubwindow.h>

#ifndef QT_NO_MDIAREA

#include <qstyle.h>
#include <qstyleoptiontitlebar.h>
#include <qmenubar.h>
#include <qsizegrip.h>
#include <qpointer.h>
#include <qdebug.h>

#include <qwidget_p.h>

class QVBoxLayout;
class QMouseEvent;

namespace QMdi {

template <typename T>
class ControlElement : public T
{
 public:
   ControlElement(QMdiSubWindow *child)
      : T(child, nullptr)
   {
      Q_ASSERT(child);
      mdiChild = child;
   }

   void *qt_metacast(const char *classname) {
      if (classname != nullptr && (strcmp(classname, "ControlElement") == 0)) {
         return this;
      }

      return nullptr;
   }

   QPointer<QMdiSubWindow> mdiChild;
};

class ControlContainer : public QObject
{
 public:
   ControlContainer(QMdiSubWindow *mdiChild);
   ~ControlContainer();

#ifndef QT_NO_MENUBAR
   void showButtonsInMenuBar(QMenuBar *menuBar);
   void removeButtonsFromMenuBar(QMenuBar *menuBar = nullptr);

   QMenuBar *menuBar() const {
      return m_menuBar;
   }
#endif

   void updateWindowIcon(const QIcon &windowIcon);
   QWidget *controllerWidget() const {
      return m_controllerWidget;
   }

   QWidget *systemMenuLabel() const {
      return m_menuLabel;
   }

 private:
   QPointer<QWidget> previousLeft;
   QPointer<QWidget> previousRight;

#ifndef QT_NO_MENUBAR
   QPointer<QMenuBar> m_menuBar;
#endif

   QPointer<QWidget> m_controllerWidget;
   QPointer<QWidget> m_menuLabel;
   QPointer<QMdiSubWindow> mdiChild;
};

} // namespace QMdi

class QMdiSubWindowPrivate : public QWidgetPrivate
{
 public:
   enum Operation {
      None,
      Move,
      TopResize,
      BottomResize,
      LeftResize,
      RightResize,
      TopLeftResize,
      TopRightResize,
      BottomLeftResize,
      BottomRightResize
   };

   enum ChangeFlag {
      HMove   = 0x01,
      VMove   = 0x02,
      HResize = 0x04,
      VResize = 0x08,
      HResizeReverse = 0x10,
      VResizeReverse = 0x20
   };

   enum WindowStateAction {
      RestoreAction,
      MoveAction,
      ResizeAction,
      MinimizeAction,
      MaximizeAction,
      StayOnTopAction,
      CloseAction,
      /* Add new states _above_ this line! */
      NumWindowStateActions
   };

   struct OperationInfo {
      uint changeFlags;
      Qt::CursorShape cursorShape;
      QRegion region;
      bool hover;

      OperationInfo(uint changeFlags, Qt::CursorShape cursorShape, bool hover = true)
         : changeFlags(changeFlags), cursorShape(cursorShape), hover(hover)
      {
      }
   };

   using OperationInfoMap = QMap<Operation, OperationInfo>;

   QMdiSubWindowPrivate();

   QPointer<QWidget> baseWidget;
   QPointer<QWidget> restoreFocusWidget;
   QPointer<QMdi::ControlContainer> controlContainer;

#ifndef QT_NO_SIZEGRIP
   QPointer<QSizeGrip> sizeGrip;
#endif

#ifndef QT_NO_RUBBERBAND
   QRubberBand *rubberBand;
#endif

   QPoint mousePressPosition;
   QRect oldGeometry;
   QSize internalMinimumSize;
   QSize userMinimumSize;
   QSize restoreSize;
   bool resizeEnabled;
   bool moveEnabled;
   bool isInInteractiveMode;

#ifndef QT_NO_RUBBERBAND
   bool isInRubberBandMode;
#endif

   bool isShadeMode;
   bool ignoreWindowTitleChange;
   bool ignoreNextActivationEvent;
   bool activationEnabled;
   bool isShadeRequestFromMinimizeMode;
   bool isMaximizeMode;
   bool isWidgetHiddenByUs;
   bool isActive;
   bool isExplicitlyDeactivated;
   int keyboardSingleStep;
   int keyboardPageStep;
   int resizeTimerId;
   Operation currentOperation;
   QStyle::SubControl hoveredSubControl;
   QStyle::SubControl activeSubControl;
   Qt::FocusReason focusInReason;
   OperationInfoMap operationMap;
   QPointer<QMenu> systemMenu;

#ifndef QT_NO_ACTIONS
   QPointer<QAction> actions[NumWindowStateActions];
#endif

   QMdiSubWindow::SubWindowOptions options;
   QString lastChildWindowTitle;
   QPalette titleBarPalette;
   QString windowTitle;
   QFont font;
   QIcon menuIcon;
   QStyleOptionTitleBar cachedStyleOptions;
   QString originalTitle;

   void _q_updateStaysOnTopHint();
   void _q_enterInteractiveMode();
   void _q_processFocusChanged(QWidget *old, QWidget *now);

   void leaveInteractiveMode();
   void removeBaseWidget();
   void initOperationMap();

#ifndef QT_NO_MENU
   void createSystemMenu();
#endif

   void updateCursor();
   void updateDirtyRegions();
   void updateGeometryConstraints();
   void updateMask();
   void setNewGeometry(const QPoint &pos);
   void setMinimizeMode();
   void setNormalMode();
   void setMaximizeMode();
   void setActive(bool activate, bool changeFocus = true);
   void processClickedSubControl();
   QRegion getRegion(Operation operation) const;
   Operation getOperation(const QPoint &pos) const;
   QStyleOptionTitleBar titleBarOptions() const;
   void ensureWindowState(Qt::WindowState state);
   int titleBarHeight(const QStyleOptionTitleBar &options) const;
   void sizeParameters(int *margin, int *minWidth) const;
   bool drawTitleBarWhenMaximized() const;

#ifndef QT_NO_MENUBAR
   QMenuBar *menuBar() const;
   void showButtonsInMenuBar(QMenuBar *menuBar);
   void removeButtonsFromMenuBar();
#endif

   void updateWindowTitle(bool requestFromChild);

#ifndef QT_NO_RUBBERBAND
   void enterRubberBandMode();
   void leaveRubberBandMode();
#endif

   QPalette desktopPalette() const;
   void updateActions();
   void setFocusWidget();

   bool restoreFocus();
   void storeFocusWidget();

   void setWindowFlags(Qt::WindowFlags flags) override;
   void setVisible(WindowStateAction, bool visible = true);

#ifndef QT_NO_ACTION
   void setEnabled(WindowStateAction, bool enable = true);

#ifndef QT_NO_MENU
   void addToSystemMenu(WindowStateAction, const QString &text, const QString &slot);
#endif

#endif  // action

   QSize iconSize() const;

#ifndef QT_NO_SIZEGRIP
   void setSizeGrip(QSizeGrip *sizeGrip);
   void setSizeGripVisible(bool visible = true) const;
#endif

   void updateInternalWindowTitle();
   QString originalWindowTitle();
   void setNewWindowTitle();

   int titleBarHeight() const {
      Q_Q(const QMdiSubWindow);

      if (! q->parent() || q->windowFlags() & Qt::FramelessWindowHint
         || (q->isMaximized() && ! drawTitleBarWhenMaximized())) {
         return 0;
      }

      QStyleOptionTitleBar options = titleBarOptions();
      int height = options.rect.height();

      if (hasBorder(options)) {
         height += q->isMinimized() ? 8 : 4;
      }
      return height;
   }

   QStyle::SubControl getSubControl(const QPoint &pos) const {
      Q_Q(const QMdiSubWindow);
      QStyleOptionTitleBar titleBarOptions = this->titleBarOptions();
      return q->style()->hitTestComplexControl(QStyle::CC_TitleBar, &titleBarOptions, pos, q);
   }

   void setNewGeometry(QRect *geometry) {
      Q_Q(QMdiSubWindow);
      Q_ASSERT(q->parent());

      geometry->setSize(geometry->size().expandedTo(internalMinimumSize));

#ifndef QT_NO_RUBBERBAND
      if (isInRubberBandMode) {
         rubberBand->setGeometry(*geometry);
      } else
#endif
         q->setGeometry(*geometry);
   }

   bool hasBorder(const QStyleOptionTitleBar &options) const {
      Q_Q(const QMdiSubWindow);
      return !q->style()->styleHint(QStyle::SH_TitleBar_NoBorder, &options, q);
   }

   bool autoRaise() const {
      Q_Q(const QMdiSubWindow);
      return q->style()->styleHint(QStyle::SH_TitleBar_AutoRaise, nullptr, q);
   }

   bool isResizeOperation() const {
      return currentOperation != None && currentOperation != Move;
   }

   bool isMoveOperation() const {
      return currentOperation == Move;
   }

 private:
   Q_DECLARE_PUBLIC(QMdiSubWindow)
};

#endif // QT_NO_MDIAREA

#endif
