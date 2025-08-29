/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef WRITE_INCLUDES_H
#define WRITE_INCLUDES_H

#include <qhash.h>
#include <qmap.h>
#include <qset.h>
#include <qstring.h>

#include <treewalker.h>

class QTextStream;

class Driver;
class Uic;

namespace CPP {

struct WriteIncludes : public TreeWalker {
   WriteIncludes(Uic *uic);

   void acceptUI(DomUI *node) override;
   void acceptWidget(DomWidget *node) override;
   void acceptLayout(DomLayout *node) override;
   void acceptSpacer(DomSpacer *node) override;
   void acceptProperty(DomProperty *node) override;

   // custom widgets
   void acceptCustomWidgets(DomCustomWidgets *node) override;
   void acceptCustomWidget(DomCustomWidget *node) override;

   // include hints
   void acceptIncludes(DomIncludes *node) override;
   void acceptInclude(DomInclude *node) override;

 private:
   void add(const QString &className, bool determineHeader = true, const QString &header = QString(), bool global = false);

   void insertIncludeForClass(const QString &className, QString header = QString(), bool global = false);
   void insertInclude(const QString &header, bool global);
   void writeHeaders(const QMap<QString, bool> &headers, bool global);
   QString headerForClassName(const QString &className) const;

   const Uic *m_uic;
   QTextStream &m_output;

   QMap<QString, bool> m_localIncludes;
   QMap<QString, bool> m_globalIncludes;
   QSet<QString> m_includeBaseNames;

   QSet<QString> m_knownClasses;

   QMap<QString, QString> m_classToHeader;
   QMap<QString, QString> m_oldHeaderToNewHeader;

   bool m_laidOut;
};

}   // namespace

#endif
