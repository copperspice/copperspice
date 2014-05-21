/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//! [environment]
export QT_ACCESSIBILITY=1
//! [environment]

//! [0]
class MyWidgetInterface : public QAccessibleWidget
{
public:
    MyWidgetInterface(QWidget *widget, Role role);

    QString text(Text text, int child) const;
    State state(int child) const;
    QString actionText(int action, Text text, int child) const;
    bool doAction(int action, int child, const QVariantList &params);
    ...
};
//! [0]


//! [1]
bool MyWidgetInterface::doAction(int action, int child,
                                 const QVariantList &params)
{
    if (child || !widget()->isEnabled())
        return false;

    switch (action) {
    case DefaultAction:
    case Press:
        {
            MyWidget *widget = qobject_cast<MyWidget *>(object());
            if (widget)
                widget->click();
        }
        return true;
    }
    return QAccessibleWidget::doAction(action, child, params);
}
//! [1]


//! [2]
QStringList MyFactory::keys() const
{
    return QStringList() << "MyWidget" << "MyOtherWidget";
}

QAccessibleInterface *MyFactory::create(const QString &className,
                                        QObject *object)
{
    if (classname == "MyWidget")
        return new MyWidgetInterface(object);
    if (classname == "MyOtherWidget")
        return new MyOtherWidgetInterface(object);
    return 0;
}

Q_EXPORT_PLUGIN2(myfactory, MyFactory)
//! [2]
