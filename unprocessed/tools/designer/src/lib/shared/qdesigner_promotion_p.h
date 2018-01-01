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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNERPROMOTION_H
#define QDESIGNERPROMOTION_H

#include "shared_global_p.h"

#include <QtDesigner/QDesignerPromotionInterface>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

    class  QDESIGNER_SHARED_EXPORT  QDesignerPromotion : public QDesignerPromotionInterface
    {
    public:
        explicit QDesignerPromotion(QDesignerFormEditorInterface *core);

        virtual PromotedClasses promotedClasses() const;

        virtual QSet<QString> referencedPromotedClassNames() const;

        virtual bool addPromotedClass(const QString &baseClass,
                                      const QString &className,
                                      const QString &includeFile,
                                      QString *errorMessage);

        virtual bool removePromotedClass(const QString &className, QString *errorMessage);

        virtual bool changePromotedClassName(const QString &oldclassName, const QString &newClassName, QString *errorMessage);

        virtual bool setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage);

        virtual QList<QDesignerWidgetDataBaseItemInterface *> promotionBaseClasses() const;

    private:
        bool canBePromoted(const QDesignerWidgetDataBaseItemInterface *) const;
        void refreshObjectInspector();

        QDesignerFormEditorInterface *m_core;
    };
}

QT_END_NAMESPACE

#endif // QDESIGNERPROMOTION_H
