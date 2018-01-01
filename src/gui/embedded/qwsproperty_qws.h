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

#ifndef QWSPROPERTY_QWS_H
#define QWSPROPERTY_QWS_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_PROPERTIES

class QWSPropertyManager
{
 public:
   enum Mode {
      PropReplace = 0,
      PropPrepend,
      PropAppend
   };

   // pre-defined properties
   enum Atom {
      PropSelection = 0
   };

   QWSPropertyManager();
   ~QWSPropertyManager();

   bool setProperty(int winId, int property, int mode, const char *data, int len);
   bool hasProperty(int winId, int property);
   bool removeProperty(int winId, int property);
   bool addProperty(int winId, int property);
   bool getProperty(int winId, int property, const char *&data, int &len);
   bool removeProperties(int winId);

 private:
   class Data;
   Data *d;
};

#endif // QT_NO_QWS_PROPERTIES

QT_END_NAMESPACE

#endif // QWSPROPERTY_QWS_H
