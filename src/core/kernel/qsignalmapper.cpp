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

#include <qsignalmapper.h>

#ifndef QT_NO_SIGNALMAPPER
#include <qhash.h>

QT_BEGIN_NAMESPACE

class QSignalMapperPrivate
{
   Q_DECLARE_PUBLIC(QSignalMapper)

 public:
   virtual ~QSignalMapperPrivate() {}

   void _q_senderDestroyed() {
      Q_Q(QSignalMapper);
      q->removeMappings(q->sender());
   }

   QHash<QObject *, int> intHash;
   QHash<QObject *, QString> stringHash;
   QHash<QObject *, QWidget *> widgetHash;
   QHash<QObject *, QObject *> objectHash;

 protected:
   QSignalMapper *q_ptr;
};

void QSignalMapper::_q_senderDestroyed()
{
   Q_D(QSignalMapper);
   d->_q_senderDestroyed();
}


/*!
    \class QSignalMapper
    \brief The QSignalMapper class bundles signals from identifiable senders.

    \ingroup objectmodel


    This class collects a set of parameterless signals, and re-emits
    them with integer, string or widget parameters corresponding to
    the object that sent the signal.

    The class supports the mapping of particular strings or integers
    with particular objects using setMapping(). The objects' signals
    can then be connected to the map() slot which will emit the
    mapped() signal with the string or integer associated with the
    original signalling object. Mappings can be removed later using
    removeMappings().

    Example: Suppose we want to create a custom widget that contains
    a group of buttons (like a tool palette). One approach is to
    connect each button's \c clicked() signal to its own custom slot;
    but in this example we want to connect all the buttons to a
    single slot and parameterize the slot by the button that was
    clicked.

    Here's the definition of a simple custom widget that has a single
    signal, \c clicked(), which is emitted with the text of the button
    that was clicked:

    \snippet doc/src/snippets/qsignalmapper/buttonwidget.h 0
    \snippet doc/src/snippets/qsignalmapper/buttonwidget.h 1

    The only function that we need to implement is the constructor:

    \snippet doc/src/snippets/qsignalmapper/buttonwidget.cpp 0
    \snippet doc/src/snippets/qsignalmapper/buttonwidget.cpp 1
    \snippet doc/src/snippets/qsignalmapper/buttonwidget.cpp 2

    A list of texts is passed to the constructor. A signal mapper is
    constructed and for each text in the list a QPushButton is
    created. We connect each button's \c clicked() signal to the
    signal mapper's map() slot, and create a mapping in the signal
    mapper from each button to the button's text. Finally we connect
    the signal mapper's mapped() signal to the custom widget's \c
    clicked() signal. When the user clicks a button, the custom
    widget will emit a single \c clicked() signal whose argument is
    the text of the button the user clicked.

    \sa QObject, QButtonGroup, QActionGroup
*/

/*!
    Constructs a QSignalMapper with parent \a parent.
*/
QSignalMapper::QSignalMapper(QObject *parent)
   : QObject(parent), d_ptr(new QSignalMapperPrivate)
{
   d_ptr->q_ptr = this;
}


/*!
    Destroys the QSignalMapper.
*/
QSignalMapper::~QSignalMapper()
{
}

/*!
    Adds a mapping so that when map() is signalled from the given \a
    sender, the signal mapped(\a id) is emitted.

    There may be at most one integer ID for each sender.

    \sa mapping()
*/
void QSignalMapper::setMapping(QObject *sender, int id)
{
   Q_D(QSignalMapper);
   d->intHash.insert(sender, id);
   connect(sender, SIGNAL(destroyed()), this, SLOT(_q_senderDestroyed()));
}

/*!
    Adds a mapping so that when map() is signalled from the \a sender,
    the signal mapped(\a text ) is emitted.

    There may be at most one text for each sender.
*/
void QSignalMapper::setMapping(QObject *sender, const QString &text)
{
   Q_D(QSignalMapper);
   d->stringHash.insert(sender, text);
   connect(sender, SIGNAL(destroyed()), this, SLOT(_q_senderDestroyed()));
}

/*!
    Adds a mapping so that when map() is signalled from the \a sender,
    the signal mapped(\a widget ) is emitted.

    There may be at most one widget for each sender.
*/
void QSignalMapper::setMapping(QObject *sender, QWidget *widget)
{
   Q_D(QSignalMapper);
   d->widgetHash.insert(sender, widget);
   connect(sender, SIGNAL(destroyed()), this, SLOT(_q_senderDestroyed()));
}

/*!
    Adds a mapping so that when map() is signalled from the \a sender,
    the signal mapped(\a object ) is emitted.

    There may be at most one object for each sender.
*/
void QSignalMapper::setMapping(QObject *sender, QObject *object)
{
   Q_D(QSignalMapper);
   d->objectHash.insert(sender, object);
   connect(sender, SIGNAL(destroyed()), this, SLOT(_q_senderDestroyed()));
}

/*!
    Returns the sender QObject that is associated with the \a id.

    \sa setMapping()
*/
QObject *QSignalMapper::mapping(int id) const
{
   Q_D(const QSignalMapper);
   return d->intHash.key(id);
}

/*!
    \overload mapping()
*/
QObject *QSignalMapper::mapping(const QString &id) const
{
   Q_D(const QSignalMapper);
   return d->stringHash.key(id);
}

/*!
    \overload mapping()

    Returns the sender QObject that is associated with the \a widget.
*/
QObject *QSignalMapper::mapping(QWidget *widget) const
{
   Q_D(const QSignalMapper);
   return d->widgetHash.key(widget);
}

/*!
    \overload mapping()

    Returns the sender QObject that is associated with the \a object.
*/
QObject *QSignalMapper::mapping(QObject *object) const
{
   Q_D(const QSignalMapper);
   return d->objectHash.key(object);
}

/*!
    Removes all mappings for \a sender.

    This is done automatically when mapped objects are destroyed.
*/
void QSignalMapper::removeMappings(QObject *sender)
{
   Q_D(QSignalMapper);

   d->intHash.remove(sender);
   d->stringHash.remove(sender);
   d->widgetHash.remove(sender);
   d->objectHash.remove(sender);
}

/*!
    This slot emits signals based on which object sends signals to it.
*/
void QSignalMapper::map()
{
   map(sender());
}

/*!
    This slot emits signals based on the \a sender object.
*/
void QSignalMapper::map(QObject *sender)
{
   Q_D(QSignalMapper);
   if (d->intHash.contains(sender)) {
      emit mapped(d->intHash.value(sender));
   }
   if (d->stringHash.contains(sender)) {
      emit mapped(d->stringHash.value(sender));
   }
   if (d->widgetHash.contains(sender)) {
      emit mapped(d->widgetHash.value(sender));
   }
   if (d->objectHash.contains(sender)) {
      emit mapped(d->objectHash.value(sender));
   }
}


/*!
    \fn void QSignalMapper::mapped(int i)

    This signal is emitted when map() is signalled from an object that
    has an integer mapping set. The object's mapped integer is passed
    in \a i.

    \sa setMapping()
*/

/*!
    \fn void QSignalMapper::mapped(const QString &text)

    This signal is emitted when map() is signalled from an object that
    has a string mapping set. The object's mapped string is passed in
    \a text.

    \sa setMapping()
*/

/*!
    \fn void QSignalMapper::mapped(QWidget *widget)

    This signal is emitted when map() is signalled from an object that
    has a widget mapping set. The object's mapped widget is passed in
    \a widget.

    \sa setMapping()
*/

/*!
    \fn void QSignalMapper::mapped(QObject *object)

    This signal is emitted when map() is signalled from an object that
    has an object mapping set. The object provided by the map is passed in
    \a object.

    \sa setMapping()
*/

QT_END_NAMESPACE

#endif // QT_NO_SIGNALMAPPER

