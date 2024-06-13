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

#ifndef ERRORSVIEW_H
#define ERRORSVIEW_H

#include <qlistview.h>

class QStandardItemModel;
class MultiDataModel;

class ErrorsView : public QListView
{
   CS_OBJECT(ErrorsView)

 public:
   enum ErrorType {
      SuperfluousAccelerator,
      MissingAccelerator,
      PunctuationDiffer,
      IgnoredPhrasebook,
      PlaceMarkersDiffer,
      NumerusMarkerMissing
   };

   ErrorsView(MultiDataModel *dataModel, QWidget *parent = nullptr);
   void clear();
   void addError(int model, const ErrorType type, const QString &arg = QString());
   QString firstError();

 private:
   void addError(int model, const QString &error);
   QStandardItemModel *m_list;
   MultiDataModel *m_dataModel;
};

#endif
