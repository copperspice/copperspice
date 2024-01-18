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

#ifndef QCOCOAINPUTCONTEXT_H
#define QCOCOAINPUTCONTEXT_H

#include <qplatform_inputcontext.h>
#include <QLocale>
#include <QPointer>

class QCocoaInputContext : public QPlatformInputContext
{
   CS_OBJECT(QCocoaInputContext)

 public:
   explicit QCocoaInputContext();
   ~QCocoaInputContext();

   bool isValid() const override {
      return true;
   }

   void reset() override;

   QLocale locale() const override {
      return m_locale;
   }
   void updateLocale();

 private:
   CS_SLOT_1(Private, void connectSignals())
   CS_SLOT_2(connectSignals)

   CS_SLOT_1(Private, void focusObjectChanged(QObject *focusObject))
   CS_SLOT_2(focusObjectChanged)

   QPointer<QWindow> mWindow;
   QLocale m_locale;
};


#endif // QCOCOAINPUTCONTEXT_H
