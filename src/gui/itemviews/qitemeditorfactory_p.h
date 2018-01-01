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

#ifndef QITEMEDITORFACTORY_P_H
#define QITEMEDITORFACTORY_P_H

#include <qlineedit.h>

#ifndef QT_NO_ITEMVIEWS

#ifndef QT_NO_LINEEDIT

QT_BEGIN_NAMESPACE

class QExpandingLineEdit : public QLineEdit
{
   GUI_CS_OBJECT(QExpandingLineEdit)

 public:
   QExpandingLineEdit(QWidget *parent);

   void setWidgetOwnsGeometry(bool value) {
      widgetOwnsGeometry = value;
   }

   GUI_CS_SLOT_1(Public, void resizeToContents())
   GUI_CS_SLOT_2(resizeToContents)

 protected:
   void changeEvent(QEvent *e) override;
 
 private:
   void updateMinimumWidth();

   int originalWidth;
   bool widgetOwnsGeometry;
};


QT_END_NAMESPACE

#endif // QT_NO_LINEEDIT

#endif //QT_NO_ITEMVIEWS

#endif //QITEMEDITORFACTORY_P_H
