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

#include <qaccessible.h>

#ifndef QT_NO_ACCESSIBILITY

#include <qaccessibleplugin.h>
#include <qaccessiblewidget.h>
#include <qapplication.h>
#include <qhash.h>
#include <qmetaobject.h>
#include <qmutex.h>
#include <qfactoryloader_p.h>
#include <qwidget.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QAccessibleFactoryInterface_iid, QLatin1String("/accessible")))
Q_GLOBAL_STATIC(QList<QAccessible::InterfaceFactory>, qAccessibleFactories)

QAccessible::UpdateHandler QAccessible::updateHandler = 0;
QAccessible::RootObjectHandler QAccessible::rootObjectHandler = 0;

static bool accessibility_active = false;
static bool cleanupAdded = false;
static void qAccessibleCleanup()
{
   qAccessibleFactories()->clear();
}

void QAccessible::installFactory(InterfaceFactory factory)
{
   if (!factory) {
      return;
   }

   if (!cleanupAdded) {
      qAddPostRoutine(qAccessibleCleanup);
      cleanupAdded = true;
   }
   if (qAccessibleFactories()->contains(factory)) {
      return;
   }
   qAccessibleFactories()->append(factory);
}

/*!
    Removes \a factory from the list of installed InterfaceFactories.
*/
void QAccessible::removeFactory(InterfaceFactory factory)
{
   qAccessibleFactories()->removeAll(factory);
}

/*!
    \internal

    Installs the given \a handler as the function to be used by
    updateAccessibility(), and returns the previously installed
    handler.
*/
QAccessible::UpdateHandler QAccessible::installUpdateHandler(UpdateHandler handler)
{
   UpdateHandler old = updateHandler;
   updateHandler = handler;
   return old;
}

/*!
    Installs the given \a handler as the function to be used by setRootObject(),
    and returns the previously installed handler.
*/
QAccessible::RootObjectHandler QAccessible::installRootObjectHandler(RootObjectHandler handler)
{
   RootObjectHandler old = rootObjectHandler;
   rootObjectHandler = handler;
   return old;
}

/*!
    If a QAccessibleInterface implementation exists for the given \a object,
    this function returns a pointer to the implementation; otherwise it
    returns 0.

    The function calls all installed factory functions (from most
    recently installed to least recently installed) until one is found
    that provides an interface for the class of \a object. If no
    factory can provide an accessibility implementation for the class
    the function loads installed accessibility plugins, and tests if
    any of the plugins can provide the implementation.

    If no implementation for the object's class is available, the
    function tries to find an implementation for the object's parent
    class, using the above strategy.

    \warning The caller is responsible for deleting the returned
    interface after use.
*/
QAccessibleInterface *QAccessible::queryAccessibleInterface(QObject *object)
{
   accessibility_active = true;
   QAccessibleInterface *iface = 0;
   if (!object) {
      return 0;
   }

   const QMetaObject *mo = object->metaObject();
   while (mo) {
      const QLatin1String cn(mo->className());
      for (int i = qAccessibleFactories()->count(); i > 0; --i) {
         InterfaceFactory factory = qAccessibleFactories()->at(i - 1);
         iface = factory(cn, object);
         if (iface) {
            return iface;
         }
      }

      QAccessibleFactoryInterface *factory = qobject_cast<QAccessibleFactoryInterface *>(loader()->instance(cn));
      if (factory) {
         iface = factory->create(cn, object);
         if (iface) {
            return iface;
         }
      }

      mo = mo->superClass();
   }

   QWidget *widget = qobject_cast<QWidget *>(object);
   if (widget) {
      return new QAccessibleWidget(widget);
   } else if (object == qApp) {
      return new QAccessibleApplication();
   }

   return 0;
}

/*!
    Returns true if an accessibility implementation has been requested
    during the runtime of the application; otherwise returns false.

    Use this function to prevent potentially expensive notifications via
    updateAccessibility().
*/
bool QAccessible::isActive()
{
   return accessibility_active;
}

/*!
  \fn void QAccessible::setRootObject(QObject *object)

  Sets the root accessible object of this application to \a object.
  All other accessible objects in the application can be reached by the
  client using object navigation.

  You should never need to call this function. Qt sets the QApplication
  object as the root object immediately before the event loop is entered
  in QApplication::exec().

  Use QAccessible::installRootObjectHandler() to redirect the function
  call to a customized handler function.

  \sa queryAccessibleInterface()
*/

/*!
  \fn void QAccessible::updateAccessibility(QObject *object, int child, Event reason)

  Notifies accessibility clients about a change in \a object's
  accessibility information.

  \a reason specifies the cause of the change, for example,
  \c ValueChange when the position of a slider has been changed. \a
  child is the (1-based) index of the child element that has changed.
  When \a child is 0, the object itself has changed.

  Call this function whenever the state of your accessible object or
  one of its sub-elements has been changed either programmatically
  (e.g. by calling QLabel::setText()) or by user interaction.

  If there are no accessibility tools listening to this event, the
  performance penalty for calling this function is small, but if determining
  the parameters of the call is expensive you can test isActive() to
  avoid unnecessary computations.
*/


/*!
    \class QAccessibleInterface
    \brief The QAccessibleInterface class defines an interface that exposes information
    about accessible objects.

    \ingroup accessibility

    Accessibility tools (also called AT Clients), such as screen readers
    or braille displays, require high-level information about
    accessible objects in an application. Accessible objects provide
    specialized input and output methods, making it possible for users
    to use accessibility tools with enabled applications (AT Servers).

    Every element that the user needs to interact with or react to is
    an accessible object, and should provide this information. These
    are mainly visual objects, such as widgets and widget elements, but
    can also be content, such as sounds.

    The AT client uses three basic concepts to acquire information
    about any accessible object in an application:
    \list
    \i \e Properties The client can read information about
    accessible objects. In some cases the client can also modify these
    properties; such as text in a line edit.
    \i \e Actions The client can invoke actions like pressing a button
    or .
    \i \e{Relationships and Navigation} The client can traverse from one
    accessible object to another, using the relationships between objects.
    \endlist

    The QAccessibleInterface defines the API for these three concepts.

    \section1 Relationships and Navigation

    The functions childCount() and indexOfChild() return the number of
    children of an accessible object and the index a child object has
    in its parent. The childAt() function returns the index of a child
    at a given position.

    The relationTo() function provides information about how two
    different objects relate to each other, and navigate() allows
    traversing from one object to another object with a given
    relationship.

    \section1 Properties

    The central property of an accessible objects is what role() it
    has. Different objects can have the same role, e.g. both the "Add
    line" element in a scroll bar and the \c OK button in a dialog have
    the same role, "button". The role implies what kind of
    interaction the user can perform with the user interface element.

    An object's state() property is a combination of different state
    flags and can describe both how the object's state differs from a
    "normal" state, e.g. it might be unavailable, and also how it
    behaves, e.g. it might be selectable.

    The text() property provides textual information about the object.
    An object usually has a name, but can provide extended information
    such as a description, help text, or information about any
    keyboard accelerators it provides. Some objects allow changing the
    text() property through the setText() function, but this
    information is in most cases read-only.

    The rect() property provides information about the geometry of an
    accessible object. This information is usually only available for
    visual objects.

    \section1 Actions and Selection

    To enable the user to interact with an accessible object the
    object must expose information about the actions that it can
    perform. userActionCount() returns the number of actions supported by
    an accessible object, and actionText() returns textual information
    about those actions. doAction() invokes an action.

    Objects that support selections can define actions to change the selection.

    \section2 Objects and children

    A QAccessibleInterface provides information about the accessible
    object, and can also provide information for the children of that
    object if those children don't provide a QAccessibleInterface
    implementation themselves. This is practical if the object has
    many similar children (e.g. items in a list view), or if the
    children are an integral part of the object itself, for example, the
    different sections in a scroll bar.

    If an accessible object provides information about its children
    through one QAccessibleInterface, the children are referenced
    using indexes. The index is 1-based for the children, i.e. 0
    refers to the object itself, 1 to the first child, 2 to the second
    child, and so on.

    All functions in QAccessibleInterface that take a child index
    relate to the object itself if the index is 0, or to the child
    specified. If a child provides its own interface implementation
    (which can be retrieved through navigation) asking the parent for
    information about that child will usually not succeed.

    \sa QAccessible
*/

/*!
    \fn bool QAccessibleInterface::isValid() const

    Returns true if all the data necessary to use this interface
    implementation is valid (e.g. all pointers are non-null);
    otherwise returns false.

    \sa object()
*/

/*!
    \fn QObject *QAccessibleInterface::object() const

    Returns a pointer to the QObject this interface implementation provides
    information for.

    \sa isValid()
*/

/*!
    \fn int QAccessibleInterface::childCount() const

    Returns the number of children that belong to this object. A child
    can provide accessibility information on its own (e.g. a child
    widget), or be a sub-element of this accessible object.

    All objects provide this information.

    \sa indexOfChild()
*/

/*!
    \fn int QAccessibleInterface::indexOfChild(const QAccessibleInterface *child) const

    Returns the 1-based index of the object \a child in this object's
    children list, or -1 if \a child is not a child of this object. 0
    is not a possible return value.

    All objects provide this information about their children.

    \sa childCount()
*/

/*!
    \fn QAccessible::Relation QAccessibleInterface::relationTo(int child,
const QAccessibleInterface *other, int otherChild) const

    Returns the relationship between this object's \a child and the \a
    other object's \a otherChild. If \a child is 0 the object's own relation
    is returned.

    The returned value indicates the relation of the called object to
    the \a other object, e.g. if this object is a child of \a other
    the return value will be \c Child.

    The return value is a combination of the bit flags in the
    QAccessible::Relation enumeration.

    All objects provide this information.

    \sa indexOfChild(), navigate()
*/

/*!
    \fn int QAccessibleInterface::childAt(int x, int y) const

    Returns the 1-based index of the child that contains the screen
    coordinates (\a x, \a y). This function returns 0 if the point is
    positioned on the object itself. If the tested point is outside
    the boundaries of the object this function returns -1.

    This function is only relyable for visible objects (invisible
    object might not be laid out correctly).

    All visual objects provide this information.

    \sa rect()
*/

/*!
    \fn int QAccessibleInterface::navigate(RelationFlag relation, int entry, QAccessibleInterface
**target) const

    Navigates from this object to an object that has a relationship
    \a relation to this object, and returns the respective object in
    \a target. It is the caller's responsibility to delete *\a target
    after use.

    If an object is found, \a target is set to point to the object, and
    the index of the child of \a target is returned. The return value
    is 0 if \a target itself is the requested object. \a target is set
    to null if this object is the target object (i.e. the requested
    object is a handled by this object).

    If no object is found \a target is set to null, and the return
    value is -1.

    The \a entry parameter has two different meanings:
    \list
    \i \e{Hierarchical and Logical relationships} -- if multiple objects with
    the requested relationship exist \a entry specifies which one to
    return. \a entry is 1-based, e.g. use 1 to get the first (and
    possibly only) object with the requested relationship.

    The following code demonstrates how to use this function to
    navigate to the first child of an object:

    \snippet doc/src/snippets/code/src_gui_accessible_qaccessible.cpp 0

    \i \e{Geometric relationships} -- the index of the child from
    which to start navigating in the specified direction. \a entry
    can be 0 to navigate to a sibling of this object, or non-null to
    navigate within contained children that don't provide their own
    accessible information.
    \endlist

    Note that the \c Descendent value for \a relation is not supported.

    All objects support navigation.

    \sa relationTo(), childCount()
*/

/*!
    \fn QString QAccessibleInterface::text(Text t, int child) const

    Returns the value of the text property \a t of the object, or of
    the object's child if \a child is not 0.

    The \l Name is a string used by clients to identify, find, or
    announce an accessible object for the user. All objects must have
    a name that is unique within their container. The name can be
    used differently by clients, so the name should both give a
    short description of the object and be unique.

    An accessible object's \l Description provides textual information
    about an object's visual appearance. The description is primarily
    used to provide greater context for vision-impaired users, but is
    also used for context searching or other applications. Not all
    objects have a description. An "OK" button would not need a
    description, but a tool button that shows a picture of a smiley
    would.

    The \l Value of an accessible object represents visual information
    contained by the object, e.g. the text in a line edit. Usually,
    the value can be modified by the user. Not all objects have a
    value, e.g. static text labels don't, and some objects have a
    state that already is the value, e.g. toggle buttons.

    The \l Help text provides information about the function and
    usage of an accessible object. Not all objects provide this
    information.

    The \l Accelerator is a keyboard shortcut that activates the
    object's default action. A keyboard shortcut is the underlined
    character in the text of a menu, menu item or widget, and is
    either the character itself, or a combination of this character
    and a modifier key like Alt, Ctrl or Shift. Command controls like
    tool buttons also have shortcut keys and usually display them in
    their tooltip.

    All objects provide a string for \l Name.

    \sa role(), state()
*/

/*!
    \fn void QAccessibleInterface::setText(Text t, int child, const QString &text)

    Sets the text property \a t of the object, or of the object's
    child if \a child is not 0, to \a text.

    Note that the text properties of most objects are read-only.

    \sa text()
*/

/*!
    \fn QRect QAccessibleInterface::rect(int child) const

    Returns the geometry of the object, or of the object's child if \a child
    is not 0. The geometry is in screen coordinates.

    This function is only reliable for visible objects (invisible
    objects might not be laid out correctly).

    All visual objects provide this information.

    \sa childAt()
*/

/*!
    \fn QAccessible::Role QAccessibleInterface::role(int child) const

    Returns the role of the object, or of the object's child if \a child
    is not 0. The role of an object is usually static.

    All accessible objects have a role.

    \sa text(), state()
*/

/*!
    \fn QAccessible::State QAccessibleInterface::state(int child) const

    Returns the current state of the object, or of the object's child if
    \a child is not 0. The returned value is a combination of the flags in
    the QAccessible::StateFlag enumeration.

    All accessible objects have a state.

    \sa text(), role()
*/

/*!
    \fn int QAccessibleInterface::userActionCount(int child) const

    Returns the number of custom actions of the object, or of the
    object's child if \a child is not 0.

    The \c Action type enumerates predefined actions: these
    are not included in the returned value.

    \sa actionText(), doAction()
*/

/*!
    \fn QString QAccessibleInterface::actionText(int action, Text t, int child) const

    Returns the text property \a t of the action \a action supported by
    the object, or of the object's child if \a child is not 0.

    \sa text(), userActionCount()
*/

/*!
    \fn bool QAccessibleInterface::doAction(int action, int child, const QVariantList &params)

    Asks the object, or the object's \a child if \a child is not 0, to
    execute \a action using the parameters, \a params. Returns true if
    the action could be executed; otherwise returns false.

    \a action can be a predefined or a custom action.

    \sa userActionCount(), actionText()
*/

/*!
    \fn QColor QAccessibleInterface::backgroundColor()
    \internal
*/

/*!
    \fn QAccessibleEditableTextInterface *QAccessibleInterface::editableTextInterface()
    \internal
*/

/*!
    \fn QColor QAccessibleInterface::foregroundColor()
    \internal
*/

/*!
    \fn QAccessibleTextInterface *QAccessibleInterface::textInterface()
    \internal
*/

/*!
    \fn QAccessibleValueInterface *QAccessibleInterface::valueInterface()
    \internal
*/

/*!
    \fn QAccessibleTableInterface *QAccessibleInterface::tableInterface()
    \internal
*/

/*!
    \fn QAccessibleTable2Interface *QAccessibleInterface::table2Interface()
    \internal
*/

/*!
    \fn QAccessibleActionInterface *QAccessibleInterface::actionInterface()
    \internal
*/

/*!
    \fn QAccessibleImageInterface *QAccessibleInterface::imageInterface()
    \internal
*/

/*!
    \class QAccessibleEvent
    \brief The QAccessibleEvent class is used to query addition
    accessibility information about complex widgets.

    The event can be of type QEvent::AccessibilityDescription or
    QEvent::AccessibilityHelp.

    Some QAccessibleInterface implementations send QAccessibleEvents
    to the widget they wrap to obtain the description or help text of
    a widget or of its children. The widget can answer by calling
    setValue() with the requested information.

    The default QWidget::event() implementation simply sets the text
    to be the widget's \l{QWidget::toolTip}{tooltip} (for \l
    AccessibilityDescription event) or its
    \l{QWidget::whatsThis}{"What's This?" text} (for \l
    AccessibilityHelp event).

    \ingroup accessibility
    \ingroup events
*/

/*!
    \fn QAccessibleEvent::QAccessibleEvent(Type type, int child)

    Constructs an accessibility event of the given \a type, which
    must be QEvent::AccessibilityDescription or
    QEvent::AccessibilityHelp.

    \a child is the (1-based) index of the child to which the request
    applies. If \a child is 0, the request is for the widget itself.

    \sa child()
*/

/*!
    \fn int QAccessibleEvent::child() const

    Returns the (1-based) index of the child to which the request
    applies. If the child is 0, the request is for the widget itself.
*/

/*!
    \fn QString QAccessibleEvent::value() const

    Returns the text set using setValue().

    \sa setValue()
*/

/*!
    \fn void QAccessibleEvent::setValue(const QString &text)

    Set the description or help text for the given child() to \a
    text, thereby answering the request.

    \sa value()
*/

/*!
    \since 4.2

    Invokes a \a method on \a child with the given parameters \a params
    and returns the result of the operation as QVariant.

    Note that the type of the returned QVariant depends on the action.

    Returns an invalid QVariant if the object doesn't support the action.
*/
QVariant QAccessibleInterface::invokeMethod(Method method, int child, const QVariantList &params)
{
   if (!(state(0) & HasInvokeExtension)) {
      return QVariant();
   }

   return static_cast<QAccessibleInterfaceEx *>(this)->invokeMethodEx(method, child, params);
}

QVariant QAccessibleInterfaceEx::virtual_hook(const QVariant &)
{
   return QVariant();
}

/*! \internal */
QAccessible2Interface *QAccessibleInterface::cast_helper(QAccessible2::InterfaceType t)
{
   if (state(0) & HasInvokeExtension) {
      return static_cast<QAccessibleInterfaceEx *>(this)->interface_cast(t);
   }
   return 0;
}

QT_END_NAMESPACE

#endif
