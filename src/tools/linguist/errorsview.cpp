/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "errorsview.h"

#include "messagemodel.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include <QtGui/QListView>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

ErrorsView::ErrorsView(MultiDataModel *dataModel, QWidget *parent) :
   QListView(parent),
   m_dataModel(dataModel)
{
   m_list = new QStandardItemModel(this);
   setModel(m_list);
}

void ErrorsView::clear()
{
   m_list->clear();
}

void ErrorsView::addError(int model, const ErrorType type, const QString &arg)
{
   QString error;
   switch (type) {
      case SuperfluousAccelerator:
         addError(model, tr("Accelerator possibly superfluous in translation."));
         break;
      case MissingAccelerator:
         addError(model, tr("Accelerator possibly missing in translation."));
         break;
      case PunctuationDiffer:
         addError(model, tr("Translation does not end with the same punctuation as the source text."));
         break;
      case IgnoredPhrasebook:
         addError(model, tr("A phrase book suggestion for '%1' was ignored.").arg(arg));
         break;
      case PlaceMarkersDiffer:
         addError(model, tr("Translation does not refer to the same place markers as in the source text."));
         break;
      case NumerusMarkerMissing:
         addError(model, tr("Translation does not contain the necessary %n place marker."));
         break;
      default:
         addError(model, tr("Unknown error"));
         break;
   }
}

QString ErrorsView::firstError()
{
   return (m_list->rowCount() == 0) ? QString() : m_list->item(0)->text();
}

void ErrorsView::addError(int model, const QString &error)
{
   // NOTE: Three statics instead of one just for GCC 3.3.5
   static QLatin1String imageLocation(":/images/s_check_danger.png");
   static QPixmap image(imageLocation);
   static QIcon pxDanger(image);
   QString lang;
   if (m_dataModel->modelCount() > 1) {
      lang = m_dataModel->model(model)->localizedLanguage() + QLatin1String(": ");
   }
   QStandardItem *item = new QStandardItem(pxDanger, lang + error);
   item->setEditable(false);
   m_list->appendRow(QList<QStandardItem *>() << item);
}

QT_END_NAMESPACE
