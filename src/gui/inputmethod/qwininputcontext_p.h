/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QWININPUTCONTEXT_P_H
#define QWININPUTCONTEXT_P_H

#ifndef QT_NO_IM

#include <QtGui/qinputcontext.h>
#include <QtCore/qt_windows.h>

#if !defined(IMR_RECONVERTSTRING)
typedef struct tagRECONVERTSTRING {
   DWORD dwSize;
   DWORD dwVersion;
   DWORD dwStrLen;
   DWORD dwStrOffset;
   DWORD dwCompStrLen;
   DWORD dwCompStrOffset;
   DWORD dwTargetStrLen;
   DWORD dwTargetStrOffset;
} RECONVERTSTRING, *PRECONVERTSTRING;
#endif

QT_BEGIN_NAMESPACE

class QWinInputContext : public QInputContext
{
   GUI_CS_OBJECT(QWinInputContext)

 public:
   explicit QWinInputContext(QObject *parent = nullptr);
   virtual ~QWinInputContext();

   QString identifierName() override {
      return QLatin1String("win");
   }
   QString language() override;

   void reset() override;
   void update() override;

   void mouseHandler(int x, QMouseEvent *event) override;
   bool isComposing() const override;

   void setFocusWidget(QWidget *w) override;

   bool startComposition();
   bool endComposition();
   bool composition(LPARAM lparam);
   int reconvertString(RECONVERTSTRING *reconv);

   static void TranslateMessage(const MSG *msg);
   static LRESULT DefWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

   static void updateImeStatus(QWidget *w, bool hasFocus);
   static void enablePopupChild(QWidget *w, bool e);
   static void enable(QWidget *w, bool e);

 private:
   void init();
   bool recursionGuard;
};

QT_END_NAMESPACE
#endif // QT_NO_IM
#endif // QWININPUTCONTEXT_P_H
