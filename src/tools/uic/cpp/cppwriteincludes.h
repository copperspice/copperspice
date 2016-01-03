/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef CPPWRITEINCLUDES_H
#define CPPWRITEINCLUDES_H

#include "treewalker.h"

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QTextStream;
class Driver;
class Uic;

namespace CPP {

struct WriteIncludes : public TreeWalker {
   WriteIncludes(Uic *uic);

   void acceptUI(DomUI *node);
   void acceptWidget(DomWidget *node);
   void acceptLayout(DomLayout *node);
   void acceptSpacer(DomSpacer *node);
   void acceptProperty(DomProperty *node);
   void acceptWidgetScripts(const DomScripts &, DomWidget *, const DomWidgets &);

   //
   // custom widgets
   //
   void acceptCustomWidgets(DomCustomWidgets *node);
   void acceptCustomWidget(DomCustomWidget *node);

   //
   // include hints
   //
   void acceptIncludes(DomIncludes *node);
   void acceptInclude(DomInclude *node);

   bool scriptsActivated() const {
      return m_scriptsActivated;
   }

 private:
   void add(const QString &className, bool determineHeader = true, const QString &header = QString(), bool global = false);

 private:
   typedef QMap<QString, bool> OrderedSet;
   void insertIncludeForClass(const QString &className, QString header = QString(), bool global = false);
   void insertInclude(const QString &header, bool global);
   void writeHeaders(const OrderedSet &headers, bool global);
   QString headerForClassName(const QString &className) const;
   void activateScripts();

   const Uic *m_uic;
   QTextStream &m_output;

   OrderedSet m_localIncludes;
   OrderedSet m_globalIncludes;
   QSet<QString> m_includeBaseNames;

   QSet<QString> m_knownClasses;

   typedef QMap<QString, QString> StringMap;
   StringMap m_classToHeader;
   StringMap m_oldHeaderToNewHeader;

   bool m_scriptsActivated;
   bool m_laidOut;
};

} // namespace CPP

QT_END_NAMESPACE

#endif // CPPWRITEINCLUDES_H
