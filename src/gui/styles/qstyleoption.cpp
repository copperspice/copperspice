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

#include <qstyleoption.h>
#include <qapplication.h>

#ifdef Q_OS_MAC
# include <qt_mac_p.h>
# include <qmacstyle_mac.h>
#endif

#include <qdebug.h>
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

QStyleOption::QStyleOption(int version, int type)
   : version(version), type(type), state(QStyle::State_None),
     direction(QApplication::layoutDirection()), fontMetrics(QFont())
{
}

QStyleOption::~QStyleOption()
{
}

void QStyleOption::init(const QWidget *widget)
{
   QWidget *window = widget->window();
   state = QStyle::State_None;
   if (widget->isEnabled()) {
      state |= QStyle::State_Enabled;
   }
   if (widget->hasFocus()) {
      state |= QStyle::State_HasFocus;
   }
   if (window->testAttribute(Qt::WA_KeyboardFocusChange)) {
      state |= QStyle::State_KeyboardFocusChange;
   }
   if (widget->underMouse()) {
      state |= QStyle::State_MouseOver;
   }
   if (window->isActiveWindow()) {
      state |= QStyle::State_Active;
   }
   if (widget->isWindow()) {
      state |= QStyle::State_Window;
   }

#ifdef Q_OS_MAC
   extern bool qt_mac_can_clickThrough(const QWidget * w); //qwidget_mac.cpp
   if (!(state & QStyle::State_Active) && !qt_mac_can_clickThrough(widget)) {
      state &= ~QStyle::State_Enabled;
   }

   switch (QMacStyle::widgetSizePolicy(widget)) {
      case QMacStyle::SizeSmall:
         state |= QStyle::State_Small;
         break;
      case QMacStyle::SizeMini:
         state |= QStyle::State_Mini;
         break;
      default:
         ;
   }
#endif
#ifdef QT_KEYPAD_NAVIGATION
   if (widget->hasEditFocus()) {
      state |= QStyle::State_HasEditFocus;
   }
#endif

   direction = widget->layoutDirection();
   rect = widget->rect();
   palette = widget->palette();
   fontMetrics = widget->fontMetrics();
}

/*!
   Constructs a copy of \a other.
*/
QStyleOption::QStyleOption(const QStyleOption &other)
   : version(Version), type(Type), state(other.state),
     direction(other.direction), rect(other.rect), fontMetrics(other.fontMetrics),
     palette(other.palette)
{
}

/*!
    Assign \a other to this QStyleOption.
*/
QStyleOption &QStyleOption::operator=(const QStyleOption &other)
{
   state = other.state;
   direction = other.direction;
   rect = other.rect;
   fontMetrics = other.fontMetrics;
   palette = other.palette;
   return *this;
}

QStyleOptionFocusRect::QStyleOptionFocusRect()
   : QStyleOption(Version, SO_FocusRect)
{
   state |= QStyle::State_KeyboardFocusChange; // assume we had one, will be corrected in initFrom()
}

QStyleOptionFocusRect::QStyleOptionFocusRect(int version)
   : QStyleOption(version, SO_FocusRect)
{
   state |= QStyle::State_KeyboardFocusChange;  // assume we had one, will be corrected in initFrom()
}

QStyleOptionFrame::QStyleOptionFrame()
   : QStyleOption(Version, SO_Frame), lineWidth(0), midLineWidth(0)
{
}

QStyleOptionFrame::QStyleOptionFrame(int version)
   : QStyleOption(version, SO_Frame), lineWidth(0), midLineWidth(0)
{
}

QStyleOptionFrameV2::QStyleOptionFrameV2()
   : QStyleOptionFrame(Version), features(None)
{
}

QStyleOptionFrameV2::QStyleOptionFrameV2(int version)
   : QStyleOptionFrame(version), features(None)
{
}

QStyleOptionFrameV2::QStyleOptionFrameV2(const QStyleOptionFrame &other)
{
   QStyleOptionFrame::operator=(other);

   const QStyleOptionFrameV2 *f2 = qstyleoption_cast<const QStyleOptionFrameV2 *>(&other);
   features = f2 ? f2->features : FrameFeatures(QStyleOptionFrameV2::None);
   version = Version;
}

QStyleOptionFrameV2 &QStyleOptionFrameV2::operator=(const QStyleOptionFrame &other)
{
   QStyleOptionFrame::operator=(other);

   const QStyleOptionFrameV2 *f2 = qstyleoption_cast<const QStyleOptionFrameV2 *>(&other);
   features = f2 ? f2->features : FrameFeatures(QStyleOptionFrameV2::None);
   version = Version;
   return *this;
}

QStyleOptionFrameV3::QStyleOptionFrameV3()
   : QStyleOptionFrameV2(Version), frameShape(QFrame::NoFrame), unused(0)
{
}

QStyleOptionFrameV3::QStyleOptionFrameV3(int version)
   : QStyleOptionFrameV2(version), frameShape(QFrame::NoFrame), unused(0)
{
}

QStyleOptionFrameV3::QStyleOptionFrameV3(const QStyleOptionFrame &other)
{
   operator=(other);
}

QStyleOptionFrameV3 &QStyleOptionFrameV3::operator=(const QStyleOptionFrame &other)
{
   QStyleOptionFrameV2::operator=(other);

   const QStyleOptionFrameV3 *f3 = qstyleoption_cast<const QStyleOptionFrameV3 *>(&other);
   frameShape = f3 ? f3->frameShape : QFrame::NoFrame;
   version = Version;
   return *this;
}

QStyleOptionViewItemV2::QStyleOptionViewItemV2()
   : QStyleOptionViewItem(Version), features(None)
{
}

QStyleOptionViewItemV2::QStyleOptionViewItemV2(const QStyleOptionViewItem &other)
   : QStyleOptionViewItem(Version)
{
   (void)QStyleOptionViewItemV2::operator=(other);
}

/*!
    \internal
*/
QStyleOptionViewItemV2::QStyleOptionViewItemV2(int version)
   : QStyleOptionViewItem(version)
{

}

/*!
    Assigns the \a other style option to this style option. The \a
    other style option can be either of the QStyleOptionViewItemV2 or
    QStyleOptionViewItem types.

    If the \a{other} style option's version is 1, this style option's
    \l ViewItemFeature value is set to \l QStyleOptionViewItemV2::None.
    If its version is 2, its \l ViewItemFeature value is simply copied
    to this style option.
*/
QStyleOptionViewItemV2 &QStyleOptionViewItemV2::operator=(const QStyleOptionViewItem &other)
{
   QStyleOptionViewItem::operator=(other);
   const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&other);
   features = v2 ? v2->features : ViewItemFeatures(QStyleOptionViewItemV2::None);
   return *this;
}

/*!
    Constructs a QStyleOptionViewItemV3 object.
*/
QStyleOptionViewItemV3::QStyleOptionViewItemV3()
   : QStyleOptionViewItemV2(Version), widget(0)
{
}

/*!
    Constructs a copy of \a other.
*/
QStyleOptionViewItemV3::QStyleOptionViewItemV3(const QStyleOptionViewItem &other)
   : QStyleOptionViewItemV2(Version), widget(0)
{
   (void)QStyleOptionViewItemV3::operator=(other);
}

/*!
    \fn QStyleOptionViewItemV3::QStyleOptionViewItemV3(const QStyleOptionViewItemV3 &other)

    Constructs a copy of \a other.
*/

/*!
    Assigns the \a other style option to this style option. The \a
    other style option can be an instance of the QStyleOptionViewItemV2,
    QStyleOptionViewItemV3 or QStyleOptionViewItem types.
*/
QStyleOptionViewItemV3 &QStyleOptionViewItemV3::operator = (const QStyleOptionViewItem &other)
{
   QStyleOptionViewItemV2::operator=(other);
   const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&other);
   locale = v3 ? v3->locale : QLocale();
   widget = v3 ? v3->widget : 0;
   return *this;
}

/*!
    \internal
*/
QStyleOptionViewItemV3::QStyleOptionViewItemV3(int version)
   : QStyleOptionViewItemV2(version), widget(0)
{
}

#ifndef QT_NO_ITEMVIEWS

/*!
    \class QStyleOptionViewItemV4
    \brief The QStyleOptionViewItemV4 class is used to describe the
    parameters necessary for drawing a frame in Qt 4.4 or above.
    \since 4.4

    QStyleOptionViewItemV4 inherits QStyleOptionViewItemV3.

    An instance of the QStyleOptionViewItemV4 class has
    \l{QStyleOption::type} {type} SO_ViewItem and
    \l{QStyleOption::version} {version} 4. The type is used internally
    by QStyleOption, its subclasses, and qstyleoption_cast() to
    determine the type of style option. In general you do not need to
    worry about this unless you want to create your own QStyleOption
    subclass and your own styles. The version is used by QStyleOption
    subclasses to implement extensions without breaking
    compatibility. If you use qstyleoption_cast(), you normally do not
    need to check it.

    See QStyleOptionViewItemV3's detailed description for a discussion
    of how to handle "V3" classes.

    \sa QStyleOptionViewItem, QStyleOption
*/

QStyleOptionViewItemV4::QStyleOptionViewItemV4()
   : QStyleOptionViewItemV3(Version), checkState(Qt::Unchecked), viewItemPosition(QStyleOptionViewItemV4::Invalid)
{
}

QStyleOptionViewItemV4::QStyleOptionViewItemV4(const QStyleOptionViewItem &other)
   : QStyleOptionViewItemV3(Version)
{
   (void)QStyleOptionViewItemV4::operator=(other);
}

QStyleOptionViewItemV4 &QStyleOptionViewItemV4::operator = (const QStyleOptionViewItem &other)
{
   QStyleOptionViewItemV3::operator=(other);

   if (const QStyleOptionViewItemV4 *v4 = qstyleoption_cast<const QStyleOptionViewItemV4 *>(&other)) {
      index = v4->index;
      checkState = v4->checkState;
      text = v4->text;
      viewItemPosition = v4->viewItemPosition;
      backgroundBrush = v4->backgroundBrush;
      icon = v4->icon;

   } else {
      viewItemPosition = QStyleOptionViewItemV4::Invalid;
      checkState = Qt::Unchecked;
   }
   return *this;
}

/*!
    \internal
*/
QStyleOptionViewItemV4::QStyleOptionViewItemV4(int version)
   : QStyleOptionViewItemV3(version)
{
}
#endif // QT_NO_ITEMVIEWS

/*!
    \class QStyleOptionGroupBox
    \brief The QStyleOptionGroupBox class describes the parameters for
    drawing a group box.

    \since 4.1

    QStyleOptionButton contains all the information that QStyle
    functions need the various graphical elements of a group box.

    It holds the lineWidth and the midLineWidth for drawing the panel,
    the group box's \l {text}{title} and the title's \l
    {textAlignment}{alignment} and \l {textColor}{color}.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QStyleOptionComplex, QGroupBox
*/

/*!
    \enum QStyleOptionGroupBox::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_GroupBox} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionGroupBox::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionGroupBox::lineWidth
    \brief the line width for drawing the panel

    The value of this variable is, currently, always 1.

    \sa QFrame::lineWidth
*/

/*!
    \variable QStyleOptionGroupBox::midLineWidth
    \brief the mid-line width for drawing the panel

    The mid-line width is usually used when drawing sunken or raised
    group box frames. The value of this variable is, currently, always 0.

    \sa QFrame::midLineWidth
*/

/*!
    \variable QStyleOptionGroupBox::text
    \brief the text of the group box

    The default value is an empty string.

    \sa QGroupBox::title
*/

/*!
    \variable QStyleOptionGroupBox::textAlignment
    \brief the alignment of the group box title

    The default value is Qt::AlignLeft.

    \sa QGroupBox::alignment
*/

/*!
    \variable QStyleOptionGroupBox::features
    \brief the features of the group box frame

    The frame is flat by default.

    \sa QStyleOptionFrameV2::FrameFeature
*/

/*!
    \variable QStyleOptionGroupBox::textColor
    \brief the color of the group box title

    The default value is an invalid color with the RGB value (0, 0,
    0). An invalid color is a color that is not properly set up for
    the underlying window system.
*/

/*!
    Constructs a QStyleOptionGroupBox, initializing the members
    variables to their default values.
*/
QStyleOptionGroupBox::QStyleOptionGroupBox()
   : QStyleOptionComplex(Version, Type), features(QStyleOptionFrameV2::None),
     textAlignment(Qt::AlignLeft), lineWidth(0), midLineWidth(0)
{
}

/*!
    \fn QStyleOptionGroupBox::QStyleOptionGroupBox(const QStyleOptionGroupBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \internal
*/
QStyleOptionGroupBox::QStyleOptionGroupBox(int version)
   : QStyleOptionComplex(version, Type), features(QStyleOptionFrameV2::None),
     textAlignment(Qt::AlignLeft), lineWidth(0), midLineWidth(0)
{
}

/*!
    \class QStyleOptionHeader
    \brief The QStyleOptionHeader class is used to describe the
    parameters for drawing a header.

    QStyleOptionHeader contains all the information that QStyle
    functions need to draw the item views' header pane, header sort
    arrow, and header label.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption
*/

/*!
    Constructs a QStyleOptionHeader, initializing the members
    variables to their default values.
*/

QStyleOptionHeader::QStyleOptionHeader()
   : QStyleOption(QStyleOptionHeader::Version, SO_Header),
     section(0), textAlignment(Qt::AlignLeft), iconAlignment(Qt::AlignLeft),
     position(QStyleOptionHeader::Beginning),
     selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None),
     orientation(Qt::Horizontal)
{
}

/*!
    \internal
*/
QStyleOptionHeader::QStyleOptionHeader(int version)
   : QStyleOption(version, SO_Header),
     section(0), textAlignment(Qt::AlignLeft), iconAlignment(Qt::AlignLeft),
     position(QStyleOptionHeader::Beginning),
     selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None),
     orientation(Qt::Horizontal)
{
}

/*!
    \variable QStyleOptionHeader::orientation
    \brief the header's orientation (horizontal or vertical)

    The default orientation is Qt::Horizontal
*/

/*!
    \fn QStyleOptionHeader::QStyleOptionHeader(const QStyleOptionHeader &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionHeader::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_Header} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionHeader::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionHeader::section
    \brief which section of the header is being painted

    The default value is 0.
*/

/*!
    \variable QStyleOptionHeader::text
    \brief the text of the header

    The default value is an empty string.
*/

/*!
    \variable QStyleOptionHeader::textAlignment
    \brief the alignment flags for the text of the header

    The default value is Qt::AlignLeft.
*/

/*!
    \variable QStyleOptionHeader::icon
    \brief the icon of the header

    The default value is an empty icon, i.e. an icon with neither a
    pixmap nor a filename.
*/

/*!
    \variable QStyleOptionHeader::iconAlignment
    \brief the alignment flags for the icon of the header

    The default value is Qt::AlignLeft.
*/

/*!
    \variable QStyleOptionHeader::position
    \brief the section's position in relation to the other sections

    The default value is QStyleOptionHeader::Beginning.
*/

/*!
    \variable QStyleOptionHeader::selectedPosition
    \brief the section's position in relation to the selected section

    The default value is QStyleOptionHeader::NotAdjacent
*/

/*!
    \variable QStyleOptionHeader::sortIndicator
    \brief the direction the sort indicator should be drawn

    The default value is QStyleOptionHeader::None.
*/

/*!
    \enum QStyleOptionHeader::SectionPosition

    This enum lets you know where the section's position is in relation to the other sections.

    \value Beginning At the beginining of the header
    \value Middle In the middle of the header
    \value End At the end of the header
    \value OnlyOneSection Only one header section

    \sa position
*/

/*!
    \enum QStyleOptionHeader::SelectedPosition

    This enum lets you know where the section's position is in relation to the selected section.

    \value NotAdjacent Not adjacent to the selected section
    \value NextIsSelected The next section is selected
    \value PreviousIsSelected The previous section is selected
    \value NextAndPreviousAreSelected Both the next and previous section are selected

    \sa selectedPosition
*/

/*!
    \enum QStyleOptionHeader::SortIndicator

    Indicates which direction the sort indicator should be drawn

    \value None No sort indicator is needed
    \value SortUp Draw an up indicator
    \value SortDown Draw a down indicator

    \sa sortIndicator
*/

/*!
    \class QStyleOptionButton
    \brief The QStyleOptionButton class is used to describe the
    parameters for drawing buttons.

    QStyleOptionButton contains all the information that QStyle
    functions need to draw graphical elements like QPushButton,
    QCheckBox, and QRadioButton.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QStyleOptionToolButton
*/

/*!
    \enum QStyleOptionButton::ButtonFeature

    This enum describes the different types of features a push button can have.

    \value None Indicates a normal push button.
    \value Flat Indicates a flat push button.
    \value HasMenu Indicates that the button has a drop down menu.
    \value DefaultButton Indicates that the button is a default button.
    \value AutoDefaultButton Indicates that the button is an auto default button.
    \value CommandLinkButton Indicates that the button is a Windows Vista type command link.

    \sa features
*/

/*!
    Constructs a QStyleOptionButton, initializing the members
    variables to their default values.
*/

QStyleOptionButton::QStyleOptionButton()
   : QStyleOption(QStyleOptionButton::Version, SO_Button), features(None)
{
}

/*!
    \internal
*/
QStyleOptionButton::QStyleOptionButton(int version)
   : QStyleOption(version, SO_Button), features(None)
{
}

/*!
    \fn QStyleOptionButton::QStyleOptionButton(const QStyleOptionButton &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionButton::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_Button} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionButton::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionButton::features
    \brief a bitwise OR of the features that describe this button

    \sa ButtonFeature
*/

/*!
    \variable QStyleOptionButton::text
    \brief the text of the button

    The default value is an empty string.
*/

/*!
    \variable QStyleOptionButton::icon
    \brief the icon of the button

    The default value is an empty icon, i.e. an icon with neither a
    pixmap nor a filename.

    \sa iconSize
*/

/*!
    \variable QStyleOptionButton::iconSize
    \brief the size of the icon for the button

    The default value is QSize(-1, -1), i.e. an invalid size.
*/


#ifndef QT_NO_TOOLBAR
/*!
    \class QStyleOptionToolBar
    \brief The QStyleOptionToolBar class is used to describe the
    parameters for drawing a toolbar.

    \since 4.1

    QStyleOptionToolBar contains all the information that QStyle
    functions need to draw QToolBar.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    The QStyleOptionToolBar class holds the lineWidth and the
    midLineWidth for drawing the widget. It also stores information
    about which \l {toolBarArea}{area} the toolbar should be located
    in, whether it is movable or not, which position the toolbar line
    should have (positionOfLine), and the toolbar's position within
    the line (positionWithinLine).

    In addition, the class provides a couple of enums: The
    ToolBarFeature enum is used to describe whether a toolbar is
    movable or not, and the ToolBarPosition enum is used to describe
    the position of a toolbar line, as well as the toolbar's position
    within the line.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption
*/

/*!
    Constructs a QStyleOptionToolBar, initializing the members
    variables to their default values.
*/

QStyleOptionToolBar::QStyleOptionToolBar()
   : QStyleOption(Version, SO_ToolBar), positionOfLine(OnlyOne), positionWithinLine(OnlyOne),
     toolBarArea(Qt::TopToolBarArea), features(None), lineWidth(0), midLineWidth(0)
{
}

/*!
    \fn QStyleOptionToolBar::QStyleOptionToolBar(const QStyleOptionToolBar &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \internal
*/
QStyleOptionToolBar::QStyleOptionToolBar(int version)
   : QStyleOption(version, SO_ToolBar), positionOfLine(OnlyOne), positionWithinLine(OnlyOne),
     toolBarArea(Qt::TopToolBarArea), features(None), lineWidth(0), midLineWidth(0)
{

}

/*!
    \enum QStyleOptionToolBar::ToolBarPosition

    \image qstyleoptiontoolbar-position.png

    This enum is used to describe the position of a toolbar line, as
    well as the toolbar's position within the line.

    The order of the positions within a line starts at the top of a
    vertical line, and from the left within a horizontal line. The
    order of the positions for the lines is always from the the
    parent widget's boundary edges.

    \value Beginning The toolbar is located at the beginning of the line,
           or the toolbar line is the first of several lines. There can
           only be one toolbar (and only one line) with this position.
    \value Middle The toolbar is located in the middle of the line,
           or the toolbar line is in the middle of several lines. There can
           several toolbars (and lines) with this position.
    \value End The toolbar is located at the end of the line,
           or the toolbar line is the last of several lines. There can
           only be one toolbar (and only one line) with this position.
    \value OnlyOne There is only one toolbar or line. This is the default value
           of the positionOfLine and positionWithinLine variables.

    \sa positionWithinLine, positionOfLine
*/

/*!
    \enum QStyleOptionToolBar::ToolBarFeature

    This enum is used to describe whether a toolbar is movable or not.

    \value None The toolbar cannot be moved. The default value.
    \value Movable The toolbar is movable, and a handle will appear when
           holding the cursor over the toolbar's boundary.

    \sa features, QToolBar::isMovable()
*/

/*!
    \variable QStyleOptionToolBar::positionOfLine

    This variable holds the position of the toolbar line.

    The default value is QStyleOptionToolBar::OnlyOne.
*/

/*!
    \variable QStyleOptionToolBar::positionWithinLine

    This variable holds the position of the toolbar within a line.

    The default value is QStyleOptionToolBar::OnlyOne.
*/

/*!
    \variable QStyleOptionToolBar::toolBarArea

    This variable holds the location for drawing the toolbar.

    The default value is Qt::TopToolBarArea.

    \sa Qt::ToolBarArea
*/

/*!
    \variable QStyleOptionToolBar::features

    This variable holds whether the toolbar is movable or not.

    The default value is \l None.
*/

/*!
    \variable QStyleOptionToolBar::lineWidth

    This variable holds the line width for drawing the toolbar.

    The default value is 0.
*/

/*!
    \variable QStyleOptionToolBar::midLineWidth

    This variable holds the mid-line width for drawing the toolbar.

    The default value is 0.
*/

/*!
    \enum QStyleOptionToolBar::StyleOptionType

    This enum is used to hold information about the type of the style
    option, and is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_ToolBar} for
    this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionToolBar::StyleOptionVersion

    This enum is used to hold information about the version of the
    style option, and is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

#endif

#ifndef QT_NO_TABBAR
/*!
    \class QStyleOptionTab
    \brief The QStyleOptionTab class is used to describe the
    parameters for drawing a tab bar.

    The QStyleOptionTab class is used for drawing several built-in Qt
    widgets including \l QTabBar and the panel for \l QTabWidget. Note
    that to describe the parameters necessary for drawing a frame in
    Qt 4.1 or above, you must use the QStyleOptionFrameV2 subclass.

    An instance of the QStyleOptiontabV2 class has
    \l{QStyleOption::type} {type} \l SO_Tab and
    \l{QStyleOption::version} {version} 1. The type is used internally
    by QStyleOption, its subclasses, and qstyleoption_cast() to
    determine the type of style option. In general you do not need to
    worry about this unless you want to create your own QStyleOption
    subclass and your own styles. The version is used by QStyleOption
    subclasses to implement extensions without breaking
    compatibility. If you use qstyleoption_cast(), you normally do not
    need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionTab and QStyleOptionTabV2.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionTabV2, QStyleOption
*/

/*!
    Constructs a QStyleOptionTab object, initializing the members
    variables to their default values.
*/

QStyleOptionTab::QStyleOptionTab()
   : QStyleOption(QStyleOptionTab::Version, SO_Tab),
     shape(QTabBar::RoundedNorth),
     row(0),
     position(Beginning),
     selectedPosition(NotAdjacent), cornerWidgets(QStyleOptionTab::NoCornerWidgets)
{
}

/*!
    \internal
*/
QStyleOptionTab::QStyleOptionTab(int version)
   : QStyleOption(version, SO_Tab),
     shape(QTabBar::RoundedNorth),
     row(0),
     position(Beginning),
     selectedPosition(NotAdjacent), cornerWidgets(QStyleOptionTab::NoCornerWidgets)
{
}

/*!
    \fn QStyleOptionTab::QStyleOptionTab(const QStyleOptionTab &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionTab::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_Tab} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionTab::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \enum QStyleOptionTab::TabPosition

    This enum describes the position of the tab.

    \value Beginning The tab is the first tab in the tab bar.
    \value Middle The tab is neither the first nor the last tab in the tab bar.
    \value End The tab is the last tab in the tab bar.
    \value OnlyOneTab The tab is both the first and the last tab in the tab bar.

    \sa position
*/

/*!
    \enum QStyleOptionTab::CornerWidget

    These flags indicate the corner widgets in a tab.

    \value NoCornerWidgets  There are no corner widgets
    \value LeftCornerWidget  Left corner widget
    \value RightCornerWidget Right corner widget

    \sa cornerWidgets
*/

/*! \enum QStyleOptionTab::SelectedPosition

    This enum describes the position of the selected tab. Some styles
    need to draw a tab differently depending on whether or not it is
    adjacent to the selected tab.

    \value NotAdjacent The tab is not adjacent to a selected tab (or is the selected tab).
    \value NextIsSelected The next tab (typically the tab on the right) is selected.
    \value PreviousIsSelected The previous tab (typically the tab on the left) is selected.

    \sa selectedPosition
*/

/*!
    \variable QStyleOptionTab::selectedPosition
    \brief the position of the selected tab in relation to this tab

    The default value is NotAdjacent, i.e. the tab is not adjacent to
    a selected tab nor is it the selected tab.
*/

/*!
    \variable QStyleOptionTab::cornerWidgets
    \brief an OR combination of CornerWidget values indicating the
    corner widgets of the tab bar

    The default value is NoCornerWidgets.

    \sa CornerWidget
*/


/*!
    \variable QStyleOptionTab::shape
    \brief the tab shape used to draw the tab; by default
    QTabBar::RoundedNorth

    \sa QTabBar::Shape
*/

/*!
    \variable QStyleOptionTab::text
    \brief the text of the tab

    The default value is an empty string.
*/

/*!
    \variable QStyleOptionTab::icon
    \brief the icon for the tab

    The default value is an empty icon, i.e. an icon with neither a
    pixmap nor a filename.
*/

/*!
    \variable QStyleOptionTab::row
    \brief which row the tab is currently in

    The default value is 0, indicating the front row.  Currently this
    property can only be 0.
*/

/*!
    \variable QStyleOptionTab::position
    \brief the position of the tab in the tab bar

    The default value is \l Beginning, i.e. the tab is the first tab
    in the tab bar.
*/

/*!
    \class QStyleOptionTabV2
    \brief The QStyleOptionTabV2 class is used to describe the
    parameters necessary for drawing a tabs in Qt 4.1 or above.

    \since 4.1

    An instance of the QStyleOptionTabV2 class has
    \l{QStyleOption::type} {type} \l SO_Tab and
    \l{QStyleOption::version} {version} 2. The type is used internally
    by QStyleOption, its subclasses, and qstyleoption_cast() to
    determine the type of style option. In general you do not need to
    worry about this unless you want to create your own QStyleOption
    subclass and your own styles. The version is used by QStyleOption
    subclasses to implement extensions without breaking
    compatibility. If you use qstyleoption_cast(), you normally do not
    need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionTab and QStyleOptionTabV2. One way to achieve this is
    to use the QStyleOptionTabV2 copy constructor. For example:

    \snippet doc/src/snippets/qstyleoption/main.cpp 3

    In the example above: If \c tabOption's version is 1, the extra
    member (\l iconSize) will be set to an invalid size for \c tabV2.
    If \c tabOption's version is 2, the constructor will simply copy
    the \c tab's iconSize.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionTab, QStyleOption
*/

/*!
    \enum QStyleOptionTabV2::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 2

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionTabV2::iconSize
    \brief the size for the icons

    The default value is QSize(-1, -1), i.e. an invalid size; use
    QStyle::pixelMetric() to find the default icon size for tab bars.

    \sa QTabBar::iconSize()
*/

/*!
    Constructs a QStyleOptionTabV2.
*/
QStyleOptionTabV2::QStyleOptionTabV2()
   : QStyleOptionTab(Version)
{
}

/*!
    \internal
*/
QStyleOptionTabV2::QStyleOptionTabV2(int version)
   : QStyleOptionTab(version)
{
}

/*!
    \fn QStyleOptionTabV2::QStyleOptionTabV2(const QStyleOptionTabV2 &other)

    Constructs a copy of the \a other style option.
*/

/*!
    Constructs a QStyleOptionTabV2 copy of the \a other style option
    which can be either of the QStyleOptionTabV2 or QStyleOptionTab
    types.

    If the other style option's version is 1, the new style option's
    \c iconSize is set to an invalid value. If its version is 2, its
    \c iconSize value is simply copied to the new style option.
*/
QStyleOptionTabV2::QStyleOptionTabV2(const QStyleOptionTab &other)
   : QStyleOptionTab(Version)
{
   if (const QStyleOptionTabV2 *tab = qstyleoption_cast<const QStyleOptionTabV2 *>(&other)) {
      *this = *tab;
   } else {
      *((QStyleOptionTab *)this) = other;
      version = Version;
   }
}

/*!
    Assigns the \a other style option to this QStyleOptionTabV2. The
    \a other style option can be either of the QStyleOptionTabV2 or
    QStyleOptionTab types.

    If the other style option's version is 1, this style option's \c
    iconSize is set to an invalid size. If its version is 2, its \c
    iconSize value is simply copied to this style option.
*/
QStyleOptionTabV2 &QStyleOptionTabV2::operator=(const QStyleOptionTab &other)
{
   QStyleOptionTab::operator=(other);

   if (const QStyleOptionTabV2 *tab = qstyleoption_cast<const QStyleOptionTabV2 *>(&other)) {
      iconSize = tab->iconSize;
   } else {
      iconSize = QSize();
   }
   return *this;
}

/*!
    Constructs a QStyleOptionTabV3.
*/

QStyleOptionTabV3::QStyleOptionTabV3()
   : QStyleOptionTabV2(Version)
   , documentMode(false)
{
}

/*!
    \internal
*/
QStyleOptionTabV3::QStyleOptionTabV3(int version)
   : QStyleOptionTabV2(version)
{
}

/*!
    \fn QStyleOptionTabV3::QStyleOptionTabV3(const QStyleOptionTabV3 &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \fn QStyleOptionTabV3::QStyleOptionTabV3(const QStyleOptionTabV2 &other)

    Constructs a copy of the \a other style option.
*/

/*!
    Constructs a QStyleOptionTabV3 copy of the \a other style option
    which can be either of the QStyleOptionTabV3, QStyleOptionTabV2
    or QStyleOptionTab types.

    If the other style option's version is 1 or 2, the new style option's
    \c leftButtonSize and \c rightButtonSize is set to an invalid value.  If
    its version is 3, its \c leftButtonSize and \c rightButtonSize values
    are simply copied to the new style option.
*/
QStyleOptionTabV3::QStyleOptionTabV3(const QStyleOptionTab &other)
   : QStyleOptionTabV2(Version)
{
   if (const QStyleOptionTabV3 *tab = qstyleoption_cast<const QStyleOptionTabV3 *>(&other)) {
      *this = *tab;
   } else {
      *((QStyleOptionTabV2 *)this) = other;
      version = Version;
   }
}

/*!
    Assigns the \a other style option to this QStyleOptionTabV3. The
    \a other style option can be either of the QStyleOptionTabV3,
    QStyleOptionTabV2 or QStyleOptionTab types.

    If the other style option's version is 1 or 2, the new style option's
    \c leftButtonSize and \c rightButtonSize is set to an invalid value.  If
    its version is 3, its \c leftButtonSize and \c rightButtonSize values
    are simply copied to the new style option.
*/
QStyleOptionTabV3 &QStyleOptionTabV3::operator=(const QStyleOptionTab &other)
{
   QStyleOptionTabV2::operator=(other);

   if (const QStyleOptionTabV3 *tab = qstyleoption_cast<const QStyleOptionTabV3 *>(&other)) {
      leftButtonSize = tab->leftButtonSize;
      rightButtonSize = tab->rightButtonSize;
   } else {
      leftButtonSize = QSize();
      rightButtonSize = QSize();
      documentMode = false;
   }
   return *this;
}

#endif // QT_NO_TABBAR

/*!
    \class QStyleOptionProgressBar
    \brief The QStyleOptionProgressBar class is used to describe the
    parameters necessary for drawing a progress bar.

    Since Qt 4.1, Qt uses the QStyleOptionProgressBarV2 subclass for
    drawing QProgressBar.

    An instance of the QStyleOptionProgressBar class has type
    SO_ProgressBar and version 1.

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.  The
    version is used by QStyleOption subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast(),
    you normally do not need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionProgressBar and QStyleOptionProgressBarV2.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionProgressBarV2, QStyleOption
*/

/*!
    Constructs a QStyleOptionProgressBar, initializing the members
    variables to their default values.
*/

QStyleOptionProgressBar::QStyleOptionProgressBar()
   : QStyleOption(QStyleOptionProgressBar::Version, SO_ProgressBar),
     minimum(0), maximum(0), progress(0), textAlignment(Qt::AlignLeft), textVisible(false)
{
}

/*!
    \internal
*/
QStyleOptionProgressBar::QStyleOptionProgressBar(int version)
   : QStyleOption(version, SO_ProgressBar),
     minimum(0), maximum(0), progress(0), textAlignment(Qt::AlignLeft), textVisible(false)
{
}

/*!
    \fn QStyleOptionProgressBar::QStyleOptionProgressBar(const QStyleOptionProgressBar &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionProgressBar::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_ProgressBar} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionProgressBar::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionProgressBar::minimum
    \brief the minimum value for the progress bar

    This is the minimum value in the progress bar. The default value
    is 0.

    \sa QProgressBar::minimum
*/

/*!
    \variable QStyleOptionProgressBar::maximum
    \brief the maximum value for the progress bar

    This is the maximum value in the progress bar. The default value
    is 0.

    \sa QProgressBar::maximum
*/

/*!
    \variable QStyleOptionProgressBar::text
    \brief the text for the progress bar

    The progress bar text is usually just the progress expressed as a
    string.  An empty string indicates that the progress bar has not
    started yet. The default value is an empty string.

    \sa QProgressBar::text
*/

/*!
    \variable QStyleOptionProgressBar::textVisible
    \brief a flag indicating whether or not text is visible

    If this flag is true then the text is visible. Otherwise, the text
    is not visible. The default value is false.

    \sa QProgressBar::textVisible
*/


/*!
    \variable QStyleOptionProgressBar::textAlignment
    \brief the text alignment for the text in the QProgressBar

    This can be used as a guide on where the text should be in the
    progress bar. The default value is Qt::AlignLeft.
*/

/*!
    \variable QStyleOptionProgressBar::progress
    \brief the current progress for the progress bar

    The current progress. A value of QStyleOptionProgressBar::minimum
    - 1 indicates that the progress hasn't started yet. The default
    value is 0.

    \sa QProgressBar::value
*/

/*!
    \class QStyleOptionProgressBarV2
    \brief The QStyleOptionProgressBarV2 class is used to describe the
    parameters necessary for drawing a progress bar in Qt 4.1 or above.

    \since 4.1

    An instance of this class has \l{QStyleOption::type} {type}
    SO_ProgressBar and \l{QStyleOption::version} {version} 2.

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles. The
    version is used by QStyleOption subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast(),
    you normally do not need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionProgressBar and QStyleOptionProgressBarV2. One way
    to achieve this is to use the QStyleOptionProgressBarV2 copy
    constructor. For example:

    \snippet doc/src/snippets/qstyleoption/main.cpp 2

    In the example above: If the \c progressBarOption's version is 1,
    the extra members (\l orientation, \l invertedAppearance, and \l
    bottomToTop) are set to default values for \c progressBarV2. If
    the \c progressBarOption's version is 2, the constructor will
    simply copy the extra members to progressBarV2.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionProgressBar, QStyleOption
*/

/*!
    Constructs a QStyleOptionProgressBarV2, initializing he members
    variables to their default values.
*/

QStyleOptionProgressBarV2::QStyleOptionProgressBarV2()
   : QStyleOptionProgressBar(2),
     orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
}

/*!
    \internal
*/
QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(int version)
   : QStyleOptionProgressBar(version),
     orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
}

/*!
    Constructs a copy of the \a other style option which can be either
    of the QStyleOptionProgressBar and QStyleOptionProgressBarV2
    types.

    If the \a{other} style option's version is 1, the extra members (\l
    orientation, \l invertedAppearance, and \l bottomToTop) are set
    to default values for the new style option. If \a{other}'s version
    is 2, the extra members are simply copied.

    \sa version
*/
QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(const QStyleOptionProgressBar &other)
   : QStyleOptionProgressBar(2), orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
   const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(&other);
   if (pb2) {
      *this = *pb2;
   } else {
      *((QStyleOptionProgressBar *)this) = other;
   }
}

/*!
    Constructs a copy of the \a other style option.
*/
QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(const QStyleOptionProgressBarV2 &other)
   : QStyleOptionProgressBar(2), orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
   *this = other;
}

/*!
    Assigns the \a other style option to this style option. The \a
    other style option can be either of the QStyleOptionProgressBarV2
    or QStyleOptionProgressBar types.

    If the \a{other} style option's version is 1, the extra members
    (\l orientation, \l invertedAppearance, and \l bottomToTop) are
    set to default values for this style option. If \a{other}'s
    version is 2, the extra members are simply copied to this style
    option.
*/
QStyleOptionProgressBarV2 &QStyleOptionProgressBarV2::operator=(const QStyleOptionProgressBar &other)
{
   QStyleOptionProgressBar::operator=(other);

   const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(&other);
   orientation = pb2 ? pb2->orientation : Qt::Horizontal;
   invertedAppearance = pb2 ? pb2->invertedAppearance : false;
   bottomToTop = pb2 ? pb2->bottomToTop : false;
   return *this;
}

/*!
    \variable QStyleOptionProgressBarV2::orientation
    \brief the progress bar's orientation (horizontal or vertical);
    the default orentation is Qt::Horizontal

    \sa QProgressBar::orientation
*/

/*!
    \variable QStyleOptionProgressBarV2::invertedAppearance
    \brief whether the progress bar's appearance is inverted

    The default value is false.

    \sa QProgressBar::invertedAppearance
*/

/*!
    \variable QStyleOptionProgressBarV2::bottomToTop
    \brief whether the text reads from bottom to top when the progress
    bar is vertical

    The default value is false.

    \sa QProgressBar::textDirection
*/

/*!
    \enum QStyleOptionProgressBarV2::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_ProgressBar} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionProgressBarV2::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 2

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/


/*!
    \class QStyleOptionMenuItem
    \brief The QStyleOptionMenuItem class is used to describe the
    parameter necessary for drawing a menu item.

    QStyleOptionMenuItem contains all the information that QStyle
    functions need to draw the menu items from \l QMenu. It is also
    used for drawing other menu-related widgets.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption
*/

/*!
    Constructs a QStyleOptionMenuItem, initializing the members
    variables to their default values.
*/

QStyleOptionMenuItem::QStyleOptionMenuItem()
   : QStyleOption(QStyleOptionMenuItem::Version, SO_MenuItem), menuItemType(Normal),
     checkType(NotCheckable), checked(false), menuHasCheckableItems(true), maxIconWidth(0), tabWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionMenuItem::QStyleOptionMenuItem(int version)
   : QStyleOption(version, SO_MenuItem), menuItemType(Normal),
     checkType(NotCheckable), checked(false), menuHasCheckableItems(true), maxIconWidth(0), tabWidth(0)
{
}

/*!
    \fn QStyleOptionMenuItem::QStyleOptionMenuItem(const QStyleOptionMenuItem &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionMenuItem::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_MenuItem} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionMenuItem::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \enum QStyleOptionMenuItem::MenuItemType

    This enum indicates the type of menu item that the structure describes.

    \value Normal A normal menu item.
    \value DefaultItem A menu item that is the default action as specified with \l QMenu::defaultAction().
    \value Separator A menu separator.
    \value SubMenu Indicates the menu item points to a sub-menu.
    \value Scroller A popup menu scroller (currently only used on Mac OS X).
    \value TearOff A tear-off handle for the menu.
    \value Margin The margin of the menu.
    \value EmptyArea The empty area of the menu.

    \sa menuItemType
*/

/*!
    \enum QStyleOptionMenuItem::CheckType

    This enum is used to indicate whether or not a check mark should be
    drawn for the item, or even if it should be drawn at all.

    \value NotCheckable The item is not checkable.
    \value Exclusive The item is an exclusive check item (like a radio button).
    \value NonExclusive The item is a non-exclusive check item (like a check box).

    \sa checkType, QAction::checkable, QAction::checked, QActionGroup::exclusive
*/

/*!
    \variable QStyleOptionMenuItem::menuItemType
    \brief the type of menu item

    The default value is \l Normal.

    \sa MenuItemType
*/

/*!
    \variable QStyleOptionMenuItem::checkType
    \brief the type of checkmark of the menu item

    The default value is \l NotCheckable.

    \sa CheckType
*/

/*!
    \variable QStyleOptionMenuItem::checked
    \brief whether the menu item is checked or not

    The default value is false.
*/

/*!
    \variable QStyleOptionMenuItem::menuHasCheckableItems
    \brief whether the menu as a whole has checkable items or not

    The default value is true.

    If this option is set to false, then the menu has no checkable
    items. This makes it possible for GUI styles to save some
    horizontal space that would normally be used for the check column.
*/

/*!
    \variable QStyleOptionMenuItem::menuRect
    \brief the rectangle for the entire menu

    The default value is a null rectangle, i.e. a rectangle with both
    the width and the height set to 0.
*/

/*!
    \variable QStyleOptionMenuItem::text
    \brief the text for the menu item

    Note that the text format is something like this "Menu
    text\bold{\\t}Shortcut".

    If the menu item doesn't have a shortcut, it will just contain the
    menu item's text. The default value is an empty string.
*/

/*!
    \variable QStyleOptionMenuItem::icon
    \brief the icon for the menu item

    The default value is an empty icon, i.e. an icon with neither a
    pixmap nor a filename.
*/

/*!
    \variable QStyleOptionMenuItem::maxIconWidth
    \brief the maximum icon width for the icon in the menu item

    This can be used for drawing the icon into the correct place or
    properly aligning items. The variable must be set regardless of
    whether or not the menu item has an icon. The default value is 0.
*/

/*!
    \variable QStyleOptionMenuItem::tabWidth
    \brief the tab width for the menu item

    The tab width is the distance between the text of the menu item
    and the shortcut. The default value is 0.
*/


/*!
    \variable QStyleOptionMenuItem::font
    \brief the font used for the menu item text

    This is the font that should be used for drawing the menu text
    minus the shortcut. The shortcut is usually drawn using the
    painter's font. By default, the application's default font is
    used.
*/

/*!
    \class QStyleOptionComplex
    \brief The QStyleOptionComplex class is used to hold parameters that are
    common to all complex controls.

    This class is not used on its own. Instead it is used to derive
    other complex control options, for example QStyleOptionSlider and
    QStyleOptionSpinBox.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator).

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption
*/

/*!
    Constructs a QStyleOptionComplex of the specified \a type and \a
    version, initializing the member variables to their default
    values. This constructor is usually called by subclasses.
*/

QStyleOptionComplex::QStyleOptionComplex(int version, int type)
   : QStyleOption(version, type), subControls(QStyle::SC_All), activeSubControls(QStyle::SC_None)
{
}

/*!
    \fn QStyleOptionComplex::QStyleOptionComplex(const QStyleOptionComplex &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionComplex::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_Complex} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionComplex::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionComplex::subControls

    This variable holds a bitwise OR of the \l{QStyle::SubControl}
    {sub-controls} to be drawn for the complex control.

    The default value is QStyle::SC_All.

    \sa QStyle::SubControl
*/

/*!
    \variable QStyleOptionComplex::activeSubControls

    This variable holds a bitwise OR of the \l{QStyle::SubControl}
    {sub-controls} that are active for the complex control.

    The default value is QStyle::SC_None.

    \sa QStyle::SubControl
*/

#ifndef QT_NO_SLIDER
/*!
    \class QStyleOptionSlider
    \brief The QStyleOptionSlider class is used to describe the
    parameters needed for drawing a slider.

    QStyleOptionSlider contains all the information that QStyle
    functions need to draw QSlider and QScrollBar.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionComplex, QSlider, QScrollBar
*/

/*!
    Constructs a QStyleOptionSlider, initializing the members
    variables to their default values.
*/

QStyleOptionSlider::QStyleOptionSlider()
   : QStyleOptionComplex(Version, SO_Slider), orientation(Qt::Horizontal), minimum(0), maximum(0),
     tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
     sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0), notchTarget(0.0),
     dialWrapping(false)
{
}

/*!
    \internal
*/
QStyleOptionSlider::QStyleOptionSlider(int version)
   : QStyleOptionComplex(version, SO_Slider), orientation(Qt::Horizontal), minimum(0), maximum(0),
     tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
     sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0), notchTarget(0.0),
     dialWrapping(false)
{
}

/*!
    \fn QStyleOptionSlider::QStyleOptionSlider(const QStyleOptionSlider &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionSlider::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_Slider} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionSlider::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionSlider::orientation
    \brief the slider's orientation (horizontal or vertical)

    The default orientation is Qt::Horizontal.

    \sa Qt::Orientation
*/

/*!
    \variable QStyleOptionSlider::minimum
    \brief the minimum value for the slider

    The default value is 0.
*/

/*!
    \variable QStyleOptionSlider::maximum
    \brief the maximum value for the slider

    The default value is 0.
*/

/*!
    \variable QStyleOptionSlider::tickPosition
    \brief the position of the slider's tick marks, if any

    The default value is QSlider::NoTicks.

    \sa QSlider::TickPosition
*/

/*!
    \variable QStyleOptionSlider::tickInterval
    \brief the interval that should be drawn between tick marks

    The default value is 0.
*/

/*!
    \variable QStyleOptionSlider::notchTarget
    \brief the number of pixel between notches

    The default value is 0.0.

    \sa QDial::notchTarget()
*/

/*!
    \variable QStyleOptionSlider::dialWrapping
    \brief whether the dial should wrap or not

    The default value is false, i.e. the dial is not wrapped.

    \sa QDial::wrapping()
*/

/*!
    \variable QStyleOptionSlider::upsideDown
    \brief the slider control orientation

    Normally a slider increases as it moves up or to the right;
    upsideDown indicates that it should do the opposite (increase as
    it moves down or to the left).  The default value is false,
    i.e. the slider increases as it moves up or to the right.

    \sa QStyle::sliderPositionFromValue(),
    QStyle::sliderValueFromPosition(),
    QAbstractSlider::invertedAppearance
*/

/*!
    \variable QStyleOptionSlider::sliderPosition
    \brief the position of the slider handle

    If the slider has active feedback (i.e.,
    QAbstractSlider::tracking is true), this value will be the same as
    \l sliderValue. Otherwise, it will have the current position of
    the handle. The default value is 0.

    \sa QAbstractSlider::tracking, sliderValue
*/

/*!
    \variable QStyleOptionSlider::sliderValue
    \brief the value of the slider

    If the slider has active feedback (i.e.,
    QAbstractSlider::tracking is true), this value will be the same
    as \l sliderPosition. Otherwise, it will have the value the
    slider had before the mouse was pressed.

    The default value is 0.

    \sa QAbstractSlider::tracking sliderPosition
*/

/*!
    \variable QStyleOptionSlider::singleStep
    \brief the size of the single step of the slider

    The default value is 0.

    \sa QAbstractSlider::singleStep
*/

/*!
    \variable QStyleOptionSlider::pageStep
    \brief the size of the page step of the slider

    The default value is 0.

    \sa QAbstractSlider::pageStep
*/
#endif // QT_NO_SLIDER

#ifndef QT_NO_SPINBOX
/*!
    \class QStyleOptionSpinBox
    \brief The QStyleOptionSpinBox class is used to describe the
    parameters necessary for drawing a spin box.

    QStyleOptionSpinBox contains all the information that QStyle
    functions need to draw QSpinBox and QDateTimeEdit.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QStyleOptionComplex
*/

/*!
    Constructs a QStyleOptionSpinBox, initializing the members
    variables to their default values.
*/

QStyleOptionSpinBox::QStyleOptionSpinBox()
   : QStyleOptionComplex(Version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
     stepEnabled(QAbstractSpinBox::StepNone), frame(false)
{
}

/*!
    \internal
*/
QStyleOptionSpinBox::QStyleOptionSpinBox(int version)
   : QStyleOptionComplex(version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
     stepEnabled(QAbstractSpinBox::StepNone), frame(false)
{
}

#endif // QT_NO_SPINBOX

QStyleOptionDockWidget::QStyleOptionDockWidget()
   : QStyleOption(Version, SO_DockWidget), closable(false),
     movable(false), floatable(false)
{
}

/*!
    \internal
*/
QStyleOptionDockWidget::QStyleOptionDockWidget(int version)
   : QStyleOption(version, SO_DockWidget), closable(false),
     movable(false), floatable(false)
{
}

QStyleOptionDockWidgetV2::QStyleOptionDockWidgetV2()
   : QStyleOptionDockWidget(Version), verticalTitleBar(false)
{
}

QStyleOptionDockWidgetV2::QStyleOptionDockWidgetV2(
   const QStyleOptionDockWidget &other)
   : QStyleOptionDockWidget(Version)
{
   (void)QStyleOptionDockWidgetV2::operator=(other);
}

QStyleOptionDockWidgetV2 &QStyleOptionDockWidgetV2::operator = (
   const QStyleOptionDockWidget &other)
{
   QStyleOptionDockWidget::operator=(other);
   const QStyleOptionDockWidgetV2 *v2
      = qstyleoption_cast<const QStyleOptionDockWidgetV2 *>(&other);
   verticalTitleBar = v2 ? v2->verticalTitleBar : false;
   return *this;
}

QStyleOptionDockWidgetV2::QStyleOptionDockWidgetV2(int version)
   : QStyleOptionDockWidget(version), verticalTitleBar(false)
{
}

/*!
    \fn QStyleOptionDockWidget::QStyleOptionDockWidget(const QStyleOptionDockWidget &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionDockWidget::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_DockWidget} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionDockWidget::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionDockWidget::title
    \brief the title of the dock window

    The default value is an empty string.
*/

/*!
    \variable QStyleOptionDockWidget::closable
    \brief whether the dock window is closable

    The default value is true.
*/

/*!
    \variable QStyleOptionDockWidget::movable
    \brief whether the dock window is movable

    The default value is false.
*/

/*!
    \variable QStyleOptionDockWidget::floatable
    \brief whether the dock window is floatable

    The default value is true.
*/

/*!
    \class QStyleOptionToolButton
    \brief The QStyleOptionToolButton class is used to describe the
    parameters for drawing a tool button.

    QStyleOptionToolButton contains all the information that QStyle
    functions need to draw QToolButton.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QStyleOptionComplex, QStyleOptionButton
*/

/*!
    \enum QStyleOptionToolButton::ToolButtonFeature
    Describes the various features that a tool button can have.

    \value None A normal tool button.
    \value Arrow The tool button is an arrow.
    \value Menu The tool button has a menu.
    \value PopupDelay There is a delay to showing the menu.
    \value HasMenu The button has a popup menu.
    \value MenuButtonPopup The button should display an arrow to
           indicate that a menu is present.

    \sa features, QToolButton::toolButtonStyle(), QToolButton::popupMode()
*/

/*!
    Constructs a QStyleOptionToolButton, initializing the members
    variables to their default values.
*/

QStyleOptionToolButton::QStyleOptionToolButton()
   : QStyleOptionComplex(Version, SO_ToolButton), features(None), arrowType(Qt::DownArrow)
   , toolButtonStyle(Qt::ToolButtonIconOnly)
{
}

/*!
    \internal
*/
QStyleOptionToolButton::QStyleOptionToolButton(int version)
   : QStyleOptionComplex(version, SO_ToolButton), features(None), arrowType(Qt::DownArrow)
   , toolButtonStyle(Qt::ToolButtonIconOnly)

{
}

/*!
    \fn QStyleOptionToolButton::QStyleOptionToolButton(const QStyleOptionToolButton &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionToolButton::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_ToolButton} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionToolButton::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionToolButton::features
    \brief an OR combination of the tool button's features

    The default value is \l None.

    \sa ToolButtonFeature
*/

/*!
    \variable QStyleOptionToolButton::icon
    \brief the icon for the tool button

    The default value is an empty icon, i.e. an icon with neither a
    pixmap nor a filename.

    \sa iconSize
*/

/*!
    \variable QStyleOptionToolButton::iconSize
    \brief the size of the icon for the tool button

    The default value is QSize(-1, -1), i.e. an invalid size.
*/

/*!
    \variable QStyleOptionToolButton::text
    \brief the text of the tool button

    This value is only used if toolButtonStyle is
    Qt::ToolButtonTextUnderIcon, Qt::ToolButtonTextBesideIcon, or
    Qt::ToolButtonTextOnly. The default value is an empty string.
*/

/*!
    \variable QStyleOptionToolButton::arrowType
    \brief the direction of the arrow for the tool button

    This value is only used if \l features includes \l Arrow. The
    default value is Qt::DownArrow.
*/

/*!
    \variable QStyleOptionToolButton::toolButtonStyle
    \brief a Qt::ToolButtonStyle value describing the appearance of
    the tool button

    The default value is Qt::ToolButtonIconOnly.

    \sa QToolButton::toolButtonStyle()
*/

/*!
    \variable QStyleOptionToolButton::pos
    \brief the position of the tool button

    The default value is a null point, i.e. (0, 0)
*/

/*!
    \variable QStyleOptionToolButton::font
    \brief the font that is used for the text

    This value is only used if toolButtonStyle is
    Qt::ToolButtonTextUnderIcon, Qt::ToolButtonTextBesideIcon, or
    Qt::ToolButtonTextOnly. By default, the application's default font
    is used.
*/

/*!
    \class QStyleOptionComboBox
    \brief The QStyleOptionComboBox class is used to describe the
    parameter for drawing a combobox.

    QStyleOptionButton contains all the information that QStyle
    functions need to draw QComboBox.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QStyleOptionComplex, QComboBox
*/

/*!
    Creates a QStyleOptionComboBox, initializing the members variables
    to their default values.
*/

QStyleOptionComboBox::QStyleOptionComboBox()
   : QStyleOptionComplex(Version, SO_ComboBox), editable(false), frame(true)
{
}

/*!
    \internal
*/
QStyleOptionComboBox::QStyleOptionComboBox(int version)
   : QStyleOptionComplex(version, SO_ComboBox), editable(false), frame(true)
{
}

/*!
    \fn QStyleOptionComboBox::QStyleOptionComboBox(const QStyleOptionComboBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionComboBox::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_ComboBox} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionComboBox::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionComboBox::editable
    \brief whether or not the combobox is editable or not

    the default
    value is false

    \sa QComboBox::isEditable()
*/


/*!
    \variable QStyleOptionComboBox::frame
    \brief whether the combo box has a frame

    The default value is true.
*/

/*!
    \variable QStyleOptionComboBox::currentText
    \brief the text for the current item of the combo box

    The default value is an empty string.
*/

/*!
    \variable QStyleOptionComboBox::currentIcon
    \brief the icon for the current item of the combo box

    The default value is an empty icon, i.e. an icon with neither a
    pixmap nor a filename.
*/

/*!
    \variable QStyleOptionComboBox::iconSize
    \brief the icon size for the current item of the combo box

    The default value is QSize(-1, -1), i.e. an invalid size.
*/

/*!
    \variable QStyleOptionComboBox::popupRect
    \brief the popup rectangle for the combobox

    The default value is a null rectangle, i.e. a rectangle with both
    the width and the height set to 0.

    This variable is currently unused. You can safely ignore it.

    \sa QStyle::SC_ComboBoxListBoxPopup
*/

/*!
    \class QStyleOptionToolBox
    \brief The QStyleOptionToolBox class is used to describe the
    parameters needed for drawing a tool box.

    QStyleOptionToolBox contains all the information that QStyle
    functions need to draw QToolBox.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QToolBox
*/

/*!
    Creates a QStyleOptionToolBox, initializing the members variables
    to their default values.
*/

QStyleOptionToolBox::QStyleOptionToolBox()
   : QStyleOption(Version, SO_ToolBox)
{
}

/*!
    \internal
*/
QStyleOptionToolBox::QStyleOptionToolBox(int version)
   : QStyleOption(version, SO_ToolBox)
{
}

/*!
    \fn QStyleOptionToolBox::QStyleOptionToolBox(const QStyleOptionToolBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionToolBox::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_ToolBox} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionToolBox::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionToolBox::icon
    \brief the icon for the tool box tab

   The default value is an empty icon, i.e. an icon with neither a
   pixmap nor a filename.
*/

/*!
    \variable QStyleOptionToolBox::text
    \brief the text for the tool box tab

    The default value is an empty string.
*/

/*!
    \class QStyleOptionToolBoxV2
    \brief The QStyleOptionToolBoxV2 class is used to describe the parameters necessary for drawing a frame in Qt 4.3 or above.

    \since 4.3
    QStyleOptionToolBoxV2 inherits QStyleOptionToolBox which is used for
    drawing the tabs in a QToolBox.

    An instance of the QStyleOptionToolBoxV2 class has
    \l{QStyleOption::type} {type} SO_ToolBox and
    \l{QStyleOption::version} {version} 2.  The type is used
    internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles. The
    version is used by QStyleOption subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast(),
    you normally do not need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionToolBox and QStyleOptionToolBoxV2.

    \sa QStyleOptionToolBox, QStyleOption
*/

/*!
    Contsructs a QStyleOptionToolBoxV2 object.
*/
QStyleOptionToolBoxV2::QStyleOptionToolBoxV2()
   : QStyleOptionToolBox(Version), position(Beginning), selectedPosition(NotAdjacent)
{
}

/*!
    \fn QStyleOptionToolBoxV2::QStyleOptionToolBoxV2(const QStyleOptionToolBoxV2 &other)

    Constructs a QStyleOptionToolBoxV2 copy of the \a other style option.
*/

/*!
    \internal
*/
QStyleOptionToolBoxV2::QStyleOptionToolBoxV2(int version)
   : QStyleOptionToolBox(version), position(Beginning), selectedPosition(NotAdjacent)
{
}

/*!
    Constructs a QStyleOptionToolBoxV2 copy of the \a other style option
    which can be either of the QStyleOptionToolBoxV2 or
    QStyleOptionToolBox types.

    If the \a other style option's version is 1, the new style
    option's \l{QStyleOptionTab::position} {position} value is set to
    \l QStyleOptionToolBoxV2::Beginning and \l selectedPosition is set
    to \l QStyleOptionToolBoxV2::NotAdjacent. If its version is 2, the
    \l{QStyleOptionTab::position} {position} selectedPosition values
    are simply copied to the new style option.

    \sa version
*/
QStyleOptionToolBoxV2::QStyleOptionToolBoxV2(const QStyleOptionToolBox &other)
{
   QStyleOptionToolBox::operator=(other);

   const QStyleOptionToolBoxV2 *f2 = qstyleoption_cast<const QStyleOptionToolBoxV2 *>(&other);
   position = f2 ? f2->position : Beginning;
   selectedPosition = f2 ? f2->selectedPosition : NotAdjacent;
   version = Version;
}

/*!
    Assigns the \a other style option to this style option. The \a
    other style option can be either of the QStyleOptionToolBoxV2 or
    QStyleOptionToolBox types.

    If the \a{other} style option's version is 1, this style option's
    \l{QStyleOptionTab::position} {position} and \l selectedPosition
    values are set to \l QStyleOptionToolBoxV2::Beginning and \l
    QStyleOptionToolBoxV2::NotAdjacent respectively. If its
    \l{QStyleOption::version} {version} is 2, these values are simply
    copied to this style option.
*/
QStyleOptionToolBoxV2 &QStyleOptionToolBoxV2::operator=(const QStyleOptionToolBox &other)
{
   QStyleOptionToolBox::operator=(other);

   const QStyleOptionToolBoxV2 *f2 = qstyleoption_cast<const QStyleOptionToolBoxV2 *>(&other);
   position = f2 ? f2->position : Beginning;
   selectedPosition = f2 ? f2->selectedPosition : NotAdjacent;
   version = Version;
   return *this;
}


/*!
    \enum QStyleOptionToolBoxV2::SelectedPosition

    This enum describes the position of the selected tab. Some styles
    need to draw a tab differently depending on whether or not it is
    adjacent to the selected tab.

    \value NotAdjacent The tab is not adjacent to a selected tab (or is the selected tab).
    \value NextIsSelected The next tab (typically the tab on the right) is selected.
    \value PreviousIsSelected The previous tab (typically the tab on the left) is selected.

    \sa selectedPosition
*/

/*!
    \enum QStyleOptionToolBoxV2::StyleOptionVersion

    This enum holds the version of this style option

    \value Version 2
*/

/*!
    \enum QStyleOptionToolBoxV2::TabPosition

    This enum describes tab positions relative to other tabs.

    \value Beginning The tab is the first (i.e., top-most) tab in
           the toolbox.
    \value Middle The tab is placed in the middle of the toolbox.
    \value End The tab is placed at the bottom of the toolbox.
    \value OnlyOneTab There is only one tab in the toolbox.
*/

/*!
    \variable QStyleOptionToolBoxV2::selectedPosition
    \brief the position of the selected tab in relation to this tab

    The default value is NotAdjacent, i.e. the tab is not adjacent to
    a selected tab nor is it the selected tab.
*/

#ifndef QT_NO_RUBBERBAND
/*!
    \class QStyleOptionRubberBand
    \brief The QStyleOptionRubberBand class is used to describe the
    parameters needed for drawing a rubber band.

    QStyleOptionRubberBand contains all the information that
    QStyle functions need to draw QRubberBand.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QRubberBand
*/

/*!
    Creates a QStyleOptionRubberBand, initializing the members
    variables to their default values.
*/

QStyleOptionRubberBand::QStyleOptionRubberBand()
   : QStyleOption(Version, SO_RubberBand), shape(QRubberBand::Line), opaque(false)
{
}

/*!
    \internal
*/
QStyleOptionRubberBand::QStyleOptionRubberBand(int version)
   : QStyleOption(version, SO_RubberBand), shape(QRubberBand::Line), opaque(false)
{
}

/*!
    \fn QStyleOptionRubberBand::QStyleOptionRubberBand(const QStyleOptionRubberBand &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionRubberBand::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_RubberBand} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionRubberBand::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionRubberBand::shape
    \brief the shape of the rubber band

    The default shape is QRubberBand::Line.
*/

/*!
    \variable QStyleOptionRubberBand::opaque
    \brief whether the rubber band is required to be drawn in an opaque style

    The default value is true.
*/
#endif // QT_NO_RUBBERBAND

/*!
    \class QStyleOptionTitleBar
    \brief The QStyleOptionTitleBar class is used to describe the
    parameters for drawing a title bar.

    QStyleOptionTitleBar contains all the information that QStyle
    functions need to draw the title bar of a QMdiSubWindow.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QStyleOptionComplex, QMdiSubWindow
*/

/*!
    Constructs a QStyleOptionTitleBar, initializing the members
    variables to their default values.
*/

QStyleOptionTitleBar::QStyleOptionTitleBar()
   : QStyleOptionComplex(Version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}

/*!
    \fn QStyleOptionTitleBar::QStyleOptionTitleBar(const QStyleOptionTitleBar &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionTitleBar::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_TitleBar} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionTitleBar::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \internal
*/
QStyleOptionTitleBar::QStyleOptionTitleBar(int version)
   : QStyleOptionComplex(version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}


/*!
    \variable QStyleOptionTitleBar::text
    \brief the text of the title bar

    The default value is an empty string.
*/

/*!
    \variable QStyleOptionTitleBar::icon
    \brief the icon for the title bar

    The default value is an empty icon, i.e. an icon with neither a
    pixmap nor a filename.
*/

/*!
    \variable QStyleOptionTitleBar::titleBarState
    \brief the state of the title bar

    This is basically the window state of the underlying widget. The
    default value is 0.

    \sa QWidget::windowState()
*/

/*!
    \variable QStyleOptionTitleBar::titleBarFlags
    \brief the widget flags for the title bar

    The default value is Qt::Widget.

    \sa Qt::WindowFlags
*/

/*!
    \class QStyleOptionViewItem
    \brief The QStyleOptionViewItem class is used to describe the
    parameters used to draw an item in a view widget.

    QStyleOptionViewItem contains all the information that QStyle
    functions need to draw the items for Qt's model/view classes.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, {model-view-programming.html}{Model/View
    Programming}
*/

/*!
    \enum QStyleOptionViewItem::Position

    This enum describes the position of the item's decoration.

    \value Left On the left of the text.
    \value Right On the right of the text.
    \value Top Above the text.
    \value Bottom Below the text.

    \sa decorationPosition
*/

/*!
    \variable QStyleOptionViewItem::showDecorationSelected
    \brief whether the decoration should be highlighted on selected
    items

    If this option is true, the branch and any decorations on selected items
    should be highlighted, indicating that the item is selected; otherwise, no
    highlighting is required. The default value is false.

    \sa QStyle::SH_ItemView_ShowDecorationSelected, QAbstractItemView
*/

/*!
    \variable QStyleOptionViewItem::textElideMode
    \brief where ellipsis should be added for text that is too long to fit
    into an item

    The default value is Qt::ElideMiddle, i.e. the ellipsis appears in
    the middle of the text.

    \sa Qt::TextElideMode, QStyle::SH_ItemView_EllipsisLocation
*/

/*!
    Constructs a QStyleOptionViewItem, initializing the members
    variables to their default values.
*/

QStyleOptionViewItem::QStyleOptionViewItem()
   : QStyleOption(Version, SO_ViewItem),
     displayAlignment(Qt::AlignLeft), decorationAlignment(Qt::AlignLeft),
     textElideMode(Qt::ElideMiddle), decorationPosition(Left),
     showDecorationSelected(false)
{
}

/*!
    \internal
*/
QStyleOptionViewItem::QStyleOptionViewItem(int version)
   : QStyleOption(version, SO_ViewItem),
     displayAlignment(Qt::AlignLeft), decorationAlignment(Qt::AlignLeft),
     textElideMode(Qt::ElideMiddle), decorationPosition(Left),
     showDecorationSelected(false)
{
}

/*!
    \fn QStyleOptionViewItem::QStyleOptionViewItem(const QStyleOptionViewItem &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \enum QStyleOptionViewItem::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_ViewItem} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionViewItem::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionViewItem::displayAlignment
    \brief the alignment of the display value for the item

    The default value is Qt::AlignLeft.
*/

/*!
    \variable QStyleOptionViewItem::decorationAlignment
    \brief the alignment of the decoration for the item

    The default value is Qt::AlignLeft.
*/

/*!
    \variable QStyleOptionViewItem::decorationPosition
    \brief the position of the decoration for the item

    The default value is \l Left.

    \sa Position
*/

/*!
    \variable QStyleOptionViewItem::decorationSize
    \brief the size of the decoration for the item

    The default value is QSize(-1, -1), i.e. an invalid size.

    \sa decorationAlignment, decorationPosition
*/

/*!
    \variable QStyleOptionViewItem::font
    \brief the font used for the item

    By default, the application's default font is used.

    \sa QFont
*/

/*!
    \fn T qstyleoption_cast<T>(const QStyleOption *option)
    \relates QStyleOption

    Returns a T or 0 depending on the \l{QStyleOption::type}{type} and
    \l{QStyleOption::version}{version} of the given \a option.

    Example:

    \snippet doc/src/snippets/qstyleoption/main.cpp 4

    \sa QStyleOption::type, QStyleOption::version
*/

/*!
    \fn T qstyleoption_cast<T>(QStyleOption *option)
    \overload
    \relates QStyleOption

    Returns a T or 0 depending on the type of the given \a option.
*/

#ifndef QT_NO_TABWIDGET
/*!
    \class QStyleOptionTabWidgetFrame
    \brief The QStyleOptionTabWidgetFrame class is used to describe the
    parameters for drawing the frame around a tab widget.

    QStyleOptionTabWidgetFrame contains all the information that
    QStyle functions need to draw the frame around QTabWidget.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QTabWidget
*/

/*!
    Constructs a QStyleOptionTabWidgetFrame, initializing the members
    variables to their default values.
*/
QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame()
   : QStyleOption(Version, SO_TabWidgetFrame), lineWidth(0), midLineWidth(0),
     shape(QTabBar::RoundedNorth)
{
}

/*!
    \fn QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame(const QStyleOptionTabWidgetFrame &other)

    Constructs a copy of \a other.
*/

/*! \internal */
QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame(int version)
   : QStyleOption(version, SO_TabWidgetFrame), lineWidth(0), midLineWidth(0),
     shape(QTabBar::RoundedNorth)
{
}

/*!
    \enum QStyleOptionTabWidgetFrame::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_TabWidgetFrame} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionTabWidgetFrame::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionTabWidgetFrame::lineWidth
    \brief the line width for drawing the panel

    The default value is 0.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::midLineWidth
    \brief the mid-line width for drawing the panel

    The mid line width is usually used in drawing sunken or raised
    frames. The default value is 0.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::shape
    \brief the tab shape used to draw the tabs

    The default value is QTabBar::RoundedNorth.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::tabBarSize
    \brief the size of the tab bar

    The default value is QSize(-1, -1), i.e. an invalid size.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::rightCornerWidgetSize
    \brief the size of the right-corner widget

    The default value is QSize(-1, -1), i.e. an invalid size.
*/

/*! \variable QStyleOptionTabWidgetFrame::leftCornerWidgetSize
    \brief the size of the left-corner widget

    The default value is QSize(-1, -1), i.e. an invalid size.
*/


/*!

    \class QStyleOptionTabWidgetFrameV2
    \brief The QStyleOptionTabWidgetFrameV2 class is used to describe the
    parameters for drawing the frame around a tab widget.

    QStyleOptionTabWidgetFrameV2 contains all the information that
    QStyle functions need to draw the frame around QTabWidget.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QTabWidget
*/


/*!
    \variable QStyleOptionTabWidgetFrameV2::tabBarRect
    \brief the rectangle containing all the tabs

    The default value is a null rectangle, i.e. a rectangle with both
    the width and the height set to 0.
*/

/*!
    \variable QStyleOptionTabWidgetFrameV2::selectedTabRect
    \brief the rectangle containing the selected tab

    This rectangle is contained within the tabBarRect. The default
    value is a null rectangle, i.e. a rectangle with both the width
    and the height set to 0.
*/


/*!
    Constructs a QStyleOptionTabWidgetFrameV2, initializing the members
    variables to their default values.
*/

QStyleOptionTabWidgetFrameV2::QStyleOptionTabWidgetFrameV2()
   : QStyleOptionTabWidgetFrame(Version)
{
}


/*! \internal */
QStyleOptionTabWidgetFrameV2::QStyleOptionTabWidgetFrameV2(int version)
   : QStyleOptionTabWidgetFrame(version)
{
}


/*! \fn QStyleOptionTabWidgetFrameV2::QStyleOptionTabWidgetFrameV2(const QStyleOptionTabWidgetFrameV2 &other)
    Constructs a QStyleOptionTabWidgetFrameV2 copy of the \a other style option.

    If the \a other style option's version is 1, the new style option's \l
    selectedTabRect and tabBarRect will contain null rects

    \sa version
*/

/*!
    Constructs a QStyleOptionTabWidgetFrameV2 copy of the \a other style option.

    If the \a other style option's version is 1, the new style option's \l
    selectedTabRect and tabBarRect will contain null rects

    \sa version
*/
QStyleOptionTabWidgetFrameV2::QStyleOptionTabWidgetFrameV2(const QStyleOptionTabWidgetFrame &other)
{
   QStyleOptionTabWidgetFrameV2::operator=(other);

}


/*!
    Assigns the \a other style option to this style option. The \a
    other style option can be either of the QStyleOptionFrameV2 or
    QStyleOptionFrame types.

    If the \a{other} style option's version is 1, this style option's
    QStyleOptionFrameV2::FrameFeature value is set to
    QStyleOptionFrameV2::None. If its version is 2, its
    \l{QStyleOptionFrameV2::}{FrameFeature} value is simply copied to
    this style option.
*/
QStyleOptionTabWidgetFrameV2 &QStyleOptionTabWidgetFrameV2::operator=(const QStyleOptionTabWidgetFrame &other)
{
   QStyleOptionTabWidgetFrame::operator=(other);
   if (const QStyleOptionTabWidgetFrameV2 *f2 = qstyleoption_cast<const QStyleOptionTabWidgetFrameV2 *>(&other)) {
      selectedTabRect = f2->selectedTabRect;
      tabBarRect = f2->tabBarRect;
   }
   return *this;
}


/*!
    \enum QStyleOptionTabWidgetFrameV2::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 2

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/


#endif // QT_NO_TABWIDGET

#ifndef QT_NO_TABBAR

/*!
    \class QStyleOptionTabBarBase
    \brief The QStyleOptionTabBarBase class is used to describe
    the base of a tab bar, i.e. the part that the tab bar usually
    overlaps with.

    QStyleOptionTabBarBase  contains all the information that QStyle
    functions need to draw the tab bar base. Note that this is only
    drawn for a standalone QTabBar (one that isn't part of a
    QTabWidget).

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QTabBar::drawBase()
*/

/*!
    Construct a QStyleOptionTabBarBase, initializing the members
    vaiables to their default values.
*/
QStyleOptionTabBarBase::QStyleOptionTabBarBase()
   : QStyleOption(Version, SO_TabBarBase), shape(QTabBar::RoundedNorth)
{
}

/*! \internal */
QStyleOptionTabBarBase::QStyleOptionTabBarBase(int version)
   : QStyleOption(version, SO_TabBarBase), shape(QTabBar::RoundedNorth)
{
}

/*!
    \fn QStyleOptionTabBarBase::QStyleOptionTabBarBase(const QStyleOptionTabBarBase &other)

    Constructs a copy of \a other.
*/

/*!
    \enum QStyleOptionTabBarBase::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_TabBarBase} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionTabBarBase::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionTabBarBase::shape
    \brief the shape of the tab bar

    The default value is QTabBar::RoundedNorth.
*/

/*!
    \variable QStyleOptionTabBarBase::tabBarRect
    \brief the rectangle containing all the tabs

    The default value is a null rectangle, i.e. a rectangle with both
    the width and the height set to 0.
*/

/*!
    \variable QStyleOptionTabBarBase::selectedTabRect
    \brief the rectangle containing the selected tab

    This rectangle is contained within the tabBarRect. The default
    value is a null rectangle, i.e. a rectangle with both the width
    and the height set to 0.
*/


/*!
    \class QStyleOptionTabBarBaseV2
    \brief The QStyleOptionTabBarBaseV2 class is used to describe
    the base of a tab bar, i.e. the part that the tab bar usually
    overlaps with.
    \since 4.5

    QStyleOptionTabBarBase  contains all the information that QStyle
    functions need to draw the tab bar base.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QTabBar::drawBase()
*/

/*!
    \enum QStyleOptionTabBarBaseV2::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 2

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleOptionTabBarBaseV2::documentMode
    \brief whether the tabbar is in document mode.

    The default value is false;
*/

/*!
    Construct a QStyleOptionTabBarBaseV2, initializing the members
    vaiables to their default values.
*/
QStyleOptionTabBarBaseV2::QStyleOptionTabBarBaseV2()
   : QStyleOptionTabBarBase(Version)
   , documentMode(false)
{
}

/*!
    \fn QStyleOptionTabBarBaseV2::QStyleOptionTabBarBaseV2(const QStyleOptionTabBarBaseV2 &other)

    Constructs a copy of \a other.
*/

/*!
    Constructs a copy of \a other.
*/
QStyleOptionTabBarBaseV2::QStyleOptionTabBarBaseV2(const QStyleOptionTabBarBase &other)
   : QStyleOptionTabBarBase(Version)
{
   (void)QStyleOptionTabBarBaseV2::operator=(other);
}

/*!
    Constructs a QStyleOptionTabBarBaseV2 copy of the \a other style option
    which can be either of the QStyleOptionTabBarBaseV2, or QStyleOptionTabBarBase types.

    If the other style option's version is not 1, the new style option's
    \c documentMode is set to false.  If its version is 2, its \c documentMode value
    is simply copied to the new style option.
*/
QStyleOptionTabBarBaseV2 &QStyleOptionTabBarBaseV2::operator = (const QStyleOptionTabBarBase &other)
{
   QStyleOptionTabBarBase::operator=(other);
   const QStyleOptionTabBarBaseV2 *v2 = qstyleoption_cast<const QStyleOptionTabBarBaseV2 *>(&other);
   documentMode = v2 ? v2->documentMode : false;
   return *this;
}

/*! \internal */
QStyleOptionTabBarBaseV2::QStyleOptionTabBarBaseV2(int version)
   : QStyleOptionTabBarBase(version), documentMode(false)
{
}

#endif // QT_NO_TABBAR

#ifndef QT_NO_SIZEGRIP
/*!
    \class QStyleOptionSizeGrip
    \brief The QStyleOptionSizeGrip class is used to describe the
    parameter for drawing a size grip.
    \since 4.2

    QStyleOptionButton contains all the information that QStyle
    functions need to draw QSizeGrip.

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters used by the style functions.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QStyleOptionComplex, QSizeGrip
*/

/*!
    Constructs a QStyleOptionSizeGrip.
*/
QStyleOptionSizeGrip::QStyleOptionSizeGrip()
   : QStyleOptionComplex(Version, Type), corner(Qt::BottomRightCorner)
{
}

/*!
    \fn QStyleOptionSizeGrip::QStyleOptionSizeGrip(const QStyleOptionSizeGrip &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \internal
*/
QStyleOptionSizeGrip::QStyleOptionSizeGrip(int version)
   : QStyleOptionComplex(version, Type), corner(Qt::BottomRightCorner)
{
}

/*!
    \variable QStyleOptionSizeGrip::corner

    The corner in which the size grip is located.
*/

/*!
    \enum QStyleOptionSizeGrip::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l{SO_TabBarBase} for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionSizeGrip::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/
#endif // QT_NO_SIZEGRIP

/*!
    \class QStyleOptionGraphicsItem
    \brief The QStyleOptionGraphicsItem class is used to describe
    the parameters needed to draw a QGraphicsItem.
    \since 4.2
    \ingroup graphicsview-api

    For performance reasons, the access to the member variables is
    direct (i.e., using the \c . or \c -> operator). This low-level feel
    makes the structures straightforward to use and emphasizes that
    these are simply parameters.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption, QGraphicsItem::paint()
*/

/*!
    \enum QStyleOptionGraphicsItem::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleOption subclass.

    \value Type The type of style option provided (\l SO_GraphicsItem for this class).

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleOptionGraphicsItem::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleOption subclass.

    \value Version 1

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    Constructs a QStyleOptionGraphicsItem.
*/
QStyleOptionGraphicsItem::QStyleOptionGraphicsItem()
   : QStyleOption(Version, Type), levelOfDetail(1)
{
}

/*!
    \internal
*/
QStyleOptionGraphicsItem::QStyleOptionGraphicsItem(int version)
   : QStyleOption(version, Type), levelOfDetail(1)
{
}

/*!
    \since 4.6

    Returns the level of detail from the \a worldTransform.

    Its value represents the maximum value of the height and
    width of a unity rectangle, mapped using the \a worldTransform
    of the painter used to draw the item. By default, if no
    transformations are applied, its value is 1. If zoomed out 1:2, the level
    of detail will be 0.5, and if zoomed in 2:1, its value is 2.
*/
qreal QStyleOptionGraphicsItem::levelOfDetailFromTransform(const QTransform &worldTransform)
{
   if (worldTransform.type() <= QTransform::TxTranslate) {
      return 1;   // Translation only? The LOD is 1.
   }

   // Two unit vectors.
   QLineF v1(0, 0, 1, 0);
   QLineF v2(0, 0, 0, 1);
   // LOD is the transformed area of a 1x1 rectangle.
   return qSqrt(worldTransform.map(v1).length() * worldTransform.map(v2).length());
}

/*!
    \fn QStyleOptionGraphicsItem::QStyleOptionGraphicsItem(const QStyleOptionGraphicsItem &other)

    Constructs a copy of \a other.
*/

/*!
    \variable QStyleOptionGraphicsItem::exposedRect
    \brief the exposed rectangle, in item coordinates

    Make use of this rectangle to speed up item drawing when only parts of the
    item are exposed. If the whole item is exposed, this rectangle will be the
    same as QGraphicsItem::boundingRect().

    This member is only initialized for items that have the
    QGraphicsItem::ItemUsesExtendedStyleOption flag set.
*/

/*!
     \variable QStyleOptionGraphicsItem::matrix
     \brief the complete transformation matrix for the item
     \obsolete

     The QMatrix provided through this member does include information about
     any perspective transformations applied to the view or item. To get the
     correct transformation matrix, use QPainter::transform() on the painter
     passed into the QGraphicsItem::paint() implementation.

     This matrix is the combination of the item's scene matrix and the matrix
     of the painter used for drawing the item. It is provided for convenience,
     allowing anvanced level-of-detail metrics that can be used to speed up
     item drawing.

     To find the dimensions of an item in screen coordinates (i.e., pixels),
     you can use the mapping functions of QMatrix, such as QMatrix::map().

     This member is only initialized for items that have the
     QGraphicsItem::ItemUsesExtendedStyleOption flag set.

     \sa QStyleOptionGraphicsItem::levelOfDetailFromTransform()
*/

/*!
    \variable QStyleOptionGraphicsItem::levelOfDetail
    \obsolete

    Use QStyleOptionGraphicsItem::levelOfDetailFromTransform()
    together with QPainter::worldTransform() instead.
*/

/*!
    \class QStyleHintReturn
    \brief The QStyleHintReturn class provides style hints that return more
    than basic data types.

    \ingroup appearance

    QStyleHintReturn and its subclasses are used to pass information
    from a style back to the querying widget. This is most useful
    when the return value from QStyle::styleHint() does not provide enough
    detail; for example, when a mask is to be returned.

    \omit
    ### --Sam
    \endomit
*/

/*!
    \enum QStyleHintReturn::HintReturnType

    \value SH_Default QStyleHintReturn
    \value SH_Mask \l QStyle::SH_RubberBand_Mask QStyle::SH_FocusFrame_Mask
    \value SH_Variant \l QStyle::SH_TextControl_FocusIndicatorTextCharFormat
*/

/*!
    \enum QStyleHintReturn::StyleOptionType

    This enum is used to hold information about the type of the style option, and
    is defined for each QStyleHintReturn subclass.

    \value Type The type of style option provided (\l SH_Default for
           this class).

    The type is used internally by QStyleHintReturn, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleHintReturn subclass and your own styles.

    \sa StyleOptionVersion
*/

/*!
    \enum QStyleHintReturn::StyleOptionVersion

    This enum is used to hold information about the version of the style option, and
    is defined for each QStyleHintReturn subclass.

    \value Version 1

    The version is used by QStyleHintReturn subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally do not need to check it.

    \sa StyleOptionType
*/

/*!
    \variable QStyleHintReturn::type
    \brief the type of the style hint container

    \sa HintReturnType
*/

/*!
    \variable QStyleHintReturn::version
    \brief the version of the style hint return container

    This value can be used by subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast<T>(), you
    normally do not need to check it.
*/

/*!
    Constructs a QStyleHintReturn with version \a version and type \a
    type.

    The version has no special meaning for QStyleHintReturn; it can be
    used by subclasses to distinguish between different version of
    the same hint type.

    \sa QStyleOption::version, QStyleOption::type
*/

QStyleHintReturn::QStyleHintReturn(int version, int type)
   : version(version), type(type)
{
}

/*!
    \internal
*/

QStyleHintReturn::~QStyleHintReturn()
{

}

/*!
    \class QStyleHintReturnMask
    \brief The QStyleHintReturnMask class provides style hints that return a QRegion.

    \ingroup appearance

    \omit
    ### --Sam
    \endomit
*/

/*!
    \variable QStyleHintReturnMask::region
    \brief the region for style hints that return a QRegion
*/

/*!
    Constructs a QStyleHintReturnMask. The member variables are
    initialized to default values.
*/
QStyleHintReturnMask::QStyleHintReturnMask() : QStyleHintReturn(Version, Type)
{
}

QStyleHintReturnVariant::QStyleHintReturnVariant() : QStyleHintReturn(Version, Type)
{
}

QDebug operator<<(QDebug debug, const QStyleOption::OptionType &optionType)
{
#if !defined(QT_NO_DEBUG)
   switch (optionType) {
      case QStyleOption::SO_Default:
         debug << "SO_Default";
         break;
      case QStyleOption::SO_FocusRect:
         debug << "SO_FocusRect";
         break;
      case QStyleOption::SO_Button:
         debug << "SO_Button";
         break;
      case QStyleOption::SO_Tab:
         debug << "SO_Tab";
         break;
      case QStyleOption::SO_MenuItem:
         debug << "SO_MenuItem";
         break;
      case QStyleOption::SO_Frame:
         debug << "SO_Frame";
         break;
      case QStyleOption::SO_ProgressBar:
         debug << "SO_ProgressBar";
         break;
      case QStyleOption::SO_ToolBox:
         debug << "SO_ToolBox";
         break;
      case QStyleOption::SO_Header:
         debug << "SO_Header";
         break;

      case QStyleOption::SO_DockWidget:
         debug << "SO_DockWidget";
         break;

      case QStyleOption::SO_ViewItem:
         debug << "SO_ViewItem";
         break;
      case QStyleOption::SO_TabWidgetFrame:
         debug << "SO_TabWidgetFrame";
         break;
      case QStyleOption::SO_TabBarBase:
         debug << "SO_TabBarBase";
         break;
      case QStyleOption::SO_RubberBand:
         debug << "SO_RubberBand";
         break;
      case QStyleOption::SO_Complex:
         debug << "SO_Complex";
         break;
      case QStyleOption::SO_Slider:
         debug << "SO_Slider";
         break;
      case QStyleOption::SO_SpinBox:
         debug << "SO_SpinBox";
         break;
      case QStyleOption::SO_ToolButton:
         debug << "SO_ToolButton";
         break;
      case QStyleOption::SO_ComboBox:
         debug << "SO_ComboBox";
         break;

      case QStyleOption::SO_TitleBar:
         debug << "SO_TitleBar";
         break;
      case QStyleOption::SO_CustomBase:
         debug << "SO_CustomBase";
         break;
      case QStyleOption::SO_GroupBox:
         debug << "SO_GroupBox";
         break;
      case QStyleOption::SO_ToolBar:
         debug << "SO_ToolBar";
         break;
      case QStyleOption::SO_ComplexCustomBase:
         debug << "SO_ComplexCustomBase";
         break;
      case QStyleOption::SO_SizeGrip:
         debug << "SO_SizeGrip";
         break;
      case QStyleOption::SO_GraphicsItem:
         debug << "SO_GraphicsItem";
         break;
   }

   return debug;
}

QDebug operator<<(QDebug debug, const QStyleOption &option)
{
#if !defined(QT_NO_DEBUG)
   debug << "QStyleOption(";
   debug << QStyleOption::OptionType(option.type);
   debug << ',' << (option.direction == Qt::RightToLeft ? "RightToLeft" : "LeftToRight");
   debug << ',' << option.state;
   debug << ',' << option.rect;
   debug << ')';
#else
   Q_UNUSED(option);
#endif
   return debug;
}
#endif

QT_END_NAMESPACE
