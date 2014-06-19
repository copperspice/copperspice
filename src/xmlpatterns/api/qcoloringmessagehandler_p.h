/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef Patternist_ColoringMessageHandler_h
#define Patternist_ColoringMessageHandler_h

#include <QHash>
#include "qcoloroutput_p.h"
#include "qabstractmessagehandler.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ColoringMessageHandler : public QAbstractMessageHandler, private ColorOutput
{
 public:
   ColoringMessageHandler(QObject *parent = 0);

 protected:
   virtual void handleMessage(QtMsgType type,
                              const QString &description,
                              const QUrl &identifier,
                              const QSourceLocation &sourceLocation);

 private:
   QString colorifyDescription(const QString &in) const;

   enum ColorType {
      RunningText,
      Location,
      ErrorCode,
      Keyword,
      Data
   };

   QHash<QString, ColorType> m_classToColor;
};
}

QT_END_NAMESPACE

#endif
