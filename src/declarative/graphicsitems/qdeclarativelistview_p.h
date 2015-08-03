/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QDECLARATIVELISTVIEW_P_H
#define QDECLARATIVELISTVIEW_P_H

#include <qdeclarativeflickable_p.h>
#include <qdeclarativeguard_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeViewSection : public QObject
{
   DECL_CS_OBJECT(QDeclarativeViewSection)

   CS_PROPERTY_READ(property, property)
   CS_PROPERTY_WRITE(property, setProperty)
   CS_PROPERTY_NOTIFY(property, propertyChanged)
   CS_PROPERTY_READ(criteria, criteria)
   CS_PROPERTY_WRITE(criteria, setCriteria)
   CS_PROPERTY_NOTIFY(criteria, criteriaChanged)
   CS_PROPERTY_READ(*delegate, delegate)
   CS_PROPERTY_WRITE(*delegate, setDelegate)
   CS_PROPERTY_NOTIFY(*delegate, delegateChanged)
   CS_ENUM(SectionCriteria)

 public:
   QDeclarativeViewSection(QObject *parent = 0) : QObject(parent), m_criteria(FullString), m_delegate(0) {}

   QString property() const {
      return m_property;
   }
   void setProperty(const QString &);

   enum SectionCriteria { FullString, FirstCharacter };
   SectionCriteria criteria() const {
      return m_criteria;
   }
   void setCriteria(SectionCriteria);

   QDeclarativeComponent *delegate() const {
      return m_delegate;
   }
   void setDelegate(QDeclarativeComponent *delegate);

   QString sectionString(const QString &value);

 public:
   CS_SIGNAL_1(Public, void propertyChanged())
   CS_SIGNAL_2(propertyChanged)
   CS_SIGNAL_1(Public, void criteriaChanged())
   CS_SIGNAL_2(criteriaChanged)
   CS_SIGNAL_1(Public, void delegateChanged())
   CS_SIGNAL_2(delegateChanged)

 private:
   QString m_property;
   SectionCriteria m_criteria;
   QDeclarativeComponent *m_delegate;
};


class QDeclarativeVisualModel;
class QDeclarativeListViewAttached;
class QDeclarativeListViewPrivate;

class QDeclarativeListView : public QDeclarativeFlickable
{
   DECL_CS_OBJECT(QDeclarativeListView)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeListView)

   CS_PROPERTY_READ(model, model)
   CS_PROPERTY_WRITE(model, setModel)
   CS_PROPERTY_NOTIFY(model, modelChanged)
   CS_PROPERTY_READ(*delegate, delegate)
   CS_PROPERTY_WRITE(*delegate, setDelegate)
   CS_PROPERTY_NOTIFY(*delegate, delegateChanged)
   CS_PROPERTY_READ(currentIndex, currentIndex)
   CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   CS_PROPERTY_NOTIFY(currentIndex, currentIndexChanged)
   CS_PROPERTY_READ(*currentItem, currentItem)
   CS_PROPERTY_NOTIFY(*currentItem, currentIndexChanged)
   CS_PROPERTY_READ(count, count)
   CS_PROPERTY_NOTIFY(count, countChanged)

   CS_PROPERTY_READ(*highlight, highlight)
   CS_PROPERTY_WRITE(*highlight, setHighlight)
   CS_PROPERTY_NOTIFY(*highlight, highlightChanged)
   CS_PROPERTY_READ(*highlightItem, highlightItem)
   CS_PROPERTY_NOTIFY(*highlightItem, highlightItemChanged)
   CS_PROPERTY_READ(highlightFollowsCurrentItem, highlightFollowsCurrentItem)
   CS_PROPERTY_WRITE(highlightFollowsCurrentItem, setHighlightFollowsCurrentItem)
   CS_PROPERTY_NOTIFY(highlightFollowsCurrentItem, highlightFollowsCurrentItemChanged)
   CS_PROPERTY_READ(highlightMoveSpeed, highlightMoveSpeed)
   CS_PROPERTY_WRITE(highlightMoveSpeed, setHighlightMoveSpeed)
   CS_PROPERTY_NOTIFY(highlightMoveSpeed, highlightMoveSpeedChanged)
   CS_PROPERTY_READ(highlightMoveDuration, highlightMoveDuration)
   CS_PROPERTY_WRITE(highlightMoveDuration, setHighlightMoveDuration)
   CS_PROPERTY_NOTIFY(highlightMoveDuration, highlightMoveDurationChanged)
   CS_PROPERTY_READ(highlightResizeSpeed, highlightResizeSpeed)
   CS_PROPERTY_WRITE(highlightResizeSpeed, setHighlightResizeSpeed)
   CS_PROPERTY_NOTIFY(highlightResizeSpeed, highlightResizeSpeedChanged)
   CS_PROPERTY_READ(highlightResizeDuration, highlightResizeDuration)
   CS_PROPERTY_WRITE(highlightResizeDuration, setHighlightResizeDuration)
   CS_PROPERTY_NOTIFY(highlightResizeDuration, highlightResizeDurationChanged)

   CS_PROPERTY_READ(preferredHighlightBegin, preferredHighlightBegin)
   CS_PROPERTY_WRITE(preferredHighlightBegin, setPreferredHighlightBegin)
   CS_PROPERTY_NOTIFY(preferredHighlightBegin, preferredHighlightBeginChanged)
   CS_PROPERTY_RESET(preferredHighlightBegin, resetPreferredHighlightBegin)
   CS_PROPERTY_READ(preferredHighlightEnd, preferredHighlightEnd)
   CS_PROPERTY_WRITE(preferredHighlightEnd, setPreferredHighlightEnd)
   CS_PROPERTY_NOTIFY(preferredHighlightEnd, preferredHighlightEndChanged)
   CS_PROPERTY_RESET(preferredHighlightEnd, resetPreferredHighlightEnd)
   CS_PROPERTY_READ(highlightRangeMode, highlightRangeMode)
   CS_PROPERTY_WRITE(highlightRangeMode, setHighlightRangeMode)
   CS_PROPERTY_NOTIFY(highlightRangeMode, highlightRangeModeChanged)

   CS_PROPERTY_READ(spacing, spacing)
   CS_PROPERTY_WRITE(spacing, setSpacing)
   CS_PROPERTY_NOTIFY(spacing, spacingChanged)
   CS_PROPERTY_READ(orientation, orientation)
   CS_PROPERTY_WRITE(orientation, setOrientation)
   CS_PROPERTY_NOTIFY(orientation, orientationChanged)
   CS_PROPERTY_READ(layoutDirection, layoutDirection)
   CS_PROPERTY_WRITE(layoutDirection, setLayoutDirection)
   CS_PROPERTY_NOTIFY(layoutDirection, layoutDirectionChanged)
   CS_PROPERTY_REVISION(layoutDirection, 1)
   CS_PROPERTY_READ(keyNavigationWraps, isWrapEnabled)
   CS_PROPERTY_WRITE(keyNavigationWraps, setWrapEnabled)
   CS_PROPERTY_NOTIFY(keyNavigationWraps, keyNavigationWrapsChanged)
   CS_PROPERTY_READ(cacheBuffer, cacheBuffer)
   CS_PROPERTY_WRITE(cacheBuffer, setCacheBuffer)
   CS_PROPERTY_NOTIFY(cacheBuffer, cacheBufferChanged)
   CS_PROPERTY_READ(*section, sectionCriteria)
   CS_PROPERTY_CONSTANT(*section)
   CS_PROPERTY_READ(currentSection, currentSection)
   CS_PROPERTY_NOTIFY(currentSection, currentSectionChanged)

   CS_PROPERTY_READ(snapMode, snapMode)
   CS_PROPERTY_WRITE(snapMode, setSnapMode)
   CS_PROPERTY_NOTIFY(snapMode, snapModeChanged)

   CS_PROPERTY_READ(*header, header)
   CS_PROPERTY_WRITE(*header, setHeader)
   CS_PROPERTY_NOTIFY(*header, headerChanged)
   CS_PROPERTY_READ(*footer, footer)
   CS_PROPERTY_WRITE(*footer, setFooter)
   CS_PROPERTY_NOTIFY(*footer, footerChanged)

   CS_ENUM(HighlightRangeMode)
   CS_ENUM(Orientation)
   CS_ENUM(SnapMode)
   CS_ENUM(PositionMode)
   CS_CLASSINFO("DefaultProperty", "data")

 public:
   QDeclarativeListView(QDeclarativeItem *parent = 0);
   ~QDeclarativeListView();

   QVariant model() const;
   void setModel(const QVariant &);

   QDeclarativeComponent *delegate() const;
   void setDelegate(QDeclarativeComponent *);

   int currentIndex() const;
   void setCurrentIndex(int idx);

   QDeclarativeItem *currentItem();
   QDeclarativeItem *highlightItem();
   int count() const;

   QDeclarativeComponent *highlight() const;
   void setHighlight(QDeclarativeComponent *highlight);

   bool highlightFollowsCurrentItem() const;
   void setHighlightFollowsCurrentItem(bool);

   enum HighlightRangeMode { NoHighlightRange, ApplyRange, StrictlyEnforceRange };
   HighlightRangeMode highlightRangeMode() const;
   void setHighlightRangeMode(HighlightRangeMode mode);

   qreal preferredHighlightBegin() const;
   void setPreferredHighlightBegin(qreal);
   void resetPreferredHighlightBegin();

   qreal preferredHighlightEnd() const;
   void setPreferredHighlightEnd(qreal);
   void resetPreferredHighlightEnd();

   qreal spacing() const;
   void setSpacing(qreal spacing);

   enum Orientation { Horizontal = Qt::Horizontal, Vertical = Qt::Vertical };
   Orientation orientation() const;
   void setOrientation(Orientation);

   Qt::LayoutDirection layoutDirection() const;
   void setLayoutDirection(Qt::LayoutDirection);
   Qt::LayoutDirection effectiveLayoutDirection() const;

   bool isWrapEnabled() const;
   void setWrapEnabled(bool);

   int cacheBuffer() const;
   void setCacheBuffer(int);

   QDeclarativeViewSection *sectionCriteria();
   QString currentSection() const;

   qreal highlightMoveSpeed() const;
   void setHighlightMoveSpeed(qreal);

   int highlightMoveDuration() const;
   void setHighlightMoveDuration(int);

   qreal highlightResizeSpeed() const;
   void setHighlightResizeSpeed(qreal);

   int highlightResizeDuration() const;
   void setHighlightResizeDuration(int);

   enum SnapMode { NoSnap, SnapToItem, SnapOneItem };
   SnapMode snapMode() const;
   void setSnapMode(SnapMode mode);

   QDeclarativeComponent *footer() const;
   void setFooter(QDeclarativeComponent *);

   QDeclarativeComponent *header() const;
   void setHeader(QDeclarativeComponent *);

   virtual void setContentX(qreal pos);
   virtual void setContentY(qreal pos);

   static QDeclarativeListViewAttached *qmlAttachedProperties(QObject *);

   enum PositionMode { Beginning, Center, End, Visible, Contain };

   CS_INVOKABLE_METHOD_1(Public, void positionViewAtIndex(int index, int mode) )
   CS_INVOKABLE_METHOD_2(positionViewAtIndex)

   CS_INVOKABLE_METHOD_1(Public, int indexAt(qreal x, qreal y) const )
   CS_INVOKABLE_METHOD_2(indexAt)

   CS_INVOKABLE_METHOD_1(Public, void positionViewAtBeginning())
   CS_INVOKABLE_METHOD_2(positionViewAtBeginning)
   CS_REVISION(positionViewAtBeginning, 1)

   CS_INVOKABLE_METHOD_1(Public, void positionViewAtEnd())
   CS_INVOKABLE_METHOD_2(positionViewAtEnd)
   CS_REVISION(positionViewAtEnd, 1)

   CS_SLOT_1(Public, void incrementCurrentIndex())
   CS_SLOT_2(incrementCurrentIndex)
   CS_SLOT_1(Public, void decrementCurrentIndex())
   CS_SLOT_2(decrementCurrentIndex)

   CS_SIGNAL_1(Public, void countChanged())
   CS_SIGNAL_2(countChanged)
   CS_SIGNAL_1(Public, void spacingChanged())
   CS_SIGNAL_2(spacingChanged)
   CS_SIGNAL_1(Public, void orientationChanged())
   CS_SIGNAL_2(orientationChanged)

   CS_SIGNAL_1(Public, void layoutDirectionChanged())
   CS_SIGNAL_2(layoutDirectionChanged)
   CS_REVISION(layoutDirectionChanged, 1)

   CS_SIGNAL_1(Public, void currentIndexChanged())
   CS_SIGNAL_2(currentIndexChanged)
   CS_SIGNAL_1(Public, void currentSectionChanged())
   CS_SIGNAL_2(currentSectionChanged)
   CS_SIGNAL_1(Public, void highlightMoveSpeedChanged())
   CS_SIGNAL_2(highlightMoveSpeedChanged)
   CS_SIGNAL_1(Public, void highlightMoveDurationChanged())
   CS_SIGNAL_2(highlightMoveDurationChanged)
   CS_SIGNAL_1(Public, void highlightResizeSpeedChanged())
   CS_SIGNAL_2(highlightResizeSpeedChanged)
   CS_SIGNAL_1(Public, void highlightResizeDurationChanged())
   CS_SIGNAL_2(highlightResizeDurationChanged)
   CS_SIGNAL_1(Public, void highlightChanged())
   CS_SIGNAL_2(highlightChanged)
   CS_SIGNAL_1(Public, void highlightItemChanged())
   CS_SIGNAL_2(highlightItemChanged)
   CS_SIGNAL_1(Public, void modelChanged())
   CS_SIGNAL_2(modelChanged)
   CS_SIGNAL_1(Public, void delegateChanged())
   CS_SIGNAL_2(delegateChanged)
   CS_SIGNAL_1(Public, void highlightFollowsCurrentItemChanged())
   CS_SIGNAL_2(highlightFollowsCurrentItemChanged)
   CS_SIGNAL_1(Public, void preferredHighlightBeginChanged())
   CS_SIGNAL_2(preferredHighlightBeginChanged)
   CS_SIGNAL_1(Public, void preferredHighlightEndChanged())
   CS_SIGNAL_2(preferredHighlightEndChanged)
   CS_SIGNAL_1(Public, void highlightRangeModeChanged())
   CS_SIGNAL_2(highlightRangeModeChanged)
   CS_SIGNAL_1(Public, void keyNavigationWrapsChanged())
   CS_SIGNAL_2(keyNavigationWrapsChanged)
   CS_SIGNAL_1(Public, void cacheBufferChanged())
   CS_SIGNAL_2(cacheBufferChanged)
   CS_SIGNAL_1(Public, void snapModeChanged())
   CS_SIGNAL_2(snapModeChanged)
   CS_SIGNAL_1(Public, void headerChanged())
   CS_SIGNAL_2(headerChanged)
   CS_SIGNAL_1(Public, void footerChanged())
   CS_SIGNAL_2(footerChanged)

 protected:
   virtual bool event(QEvent *event);
   virtual void viewportMoved();
   virtual qreal minYExtent() const;
   virtual qreal maxYExtent() const;
   virtual qreal minXExtent() const;
   virtual qreal maxXExtent() const;
   virtual void keyPressEvent(QKeyEvent *);
   virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
   virtual void componentComplete();

 private :
   CS_SLOT_1(Private, void updateSections())
   CS_SLOT_2(updateSections)
   CS_SLOT_1(Private, void refill())
   CS_SLOT_2(refill)
   CS_SLOT_1(Private, void trackedPositionChanged())
   CS_SLOT_2(trackedPositionChanged)
   CS_SLOT_1(Private, void itemsInserted(int index, int count))
   CS_SLOT_2(itemsInserted)
   CS_SLOT_1(Private, void itemsRemoved(int index, int count))
   CS_SLOT_2(itemsRemoved)
   CS_SLOT_1(Private, void itemsMoved(int from, int to, int count))
   CS_SLOT_2(itemsMoved)
   CS_SLOT_1(Private, void itemsChanged(int index, int count))
   CS_SLOT_2(itemsChanged)
   CS_SLOT_1(Private, void modelReset())
   CS_SLOT_2(modelReset)
   CS_SLOT_1(Private, void destroyRemoved())
   CS_SLOT_2(destroyRemoved)
   CS_SLOT_1(Private, void createdItem(int index, QDeclarativeItem *item))
   CS_SLOT_2(createdItem)
   CS_SLOT_1(Private, void destroyingItem(QDeclarativeItem *item))
   CS_SLOT_2(destroyingItem)
   CS_SLOT_1(Private, void animStopped())
   CS_SLOT_2(animStopped)
};

class QDeclarativeListViewAttached : public QObject
{
   DECL_CS_OBJECT(QDeclarativeListViewAttached)
 public:
   QDeclarativeListViewAttached(QObject *parent)
      : QObject(parent), m_view(0), m_isCurrent(false), m_delayRemove(false) {}
   ~QDeclarativeListViewAttached() {}

   CS_PROPERTY_READ(*view, view)
   CS_PROPERTY_NOTIFY(*view, viewChanged)
   QDeclarativeListView *view() {
      return m_view;
   }
   void setView(QDeclarativeListView *view) {
      if (view != m_view) {
         m_view = view;
         emit viewChanged();
      }
   }

   CS_PROPERTY_READ(isCurrentItem, isCurrentItem)
   CS_PROPERTY_NOTIFY(isCurrentItem, currentItemChanged)
   bool isCurrentItem() const {
      return m_isCurrent;
   }
   void setIsCurrentItem(bool c) {
      if (m_isCurrent != c) {
         m_isCurrent = c;
         emit currentItemChanged();
      }
   }

   CS_PROPERTY_READ(previousSection, prevSection)
   CS_PROPERTY_NOTIFY(previousSection, prevSectionChanged)
   QString prevSection() const {
      return m_prevSection;
   }
   void setPrevSection(const QString &sect) {
      if (m_prevSection != sect) {
         m_prevSection = sect;
         emit prevSectionChanged();
      }
   }

   CS_PROPERTY_READ(nextSection, nextSection)
   CS_PROPERTY_NOTIFY(nextSection, nextSectionChanged)
   QString nextSection() const {
      return m_nextSection;
   }
   void setNextSection(const QString &sect) {
      if (m_nextSection != sect) {
         m_nextSection = sect;
         emit nextSectionChanged();
      }
   }

   CS_PROPERTY_READ(section, section)
   CS_PROPERTY_NOTIFY(section, sectionChanged)
   QString section() const {
      return m_section;
   }
   void setSection(const QString &sect) {
      if (m_section != sect) {
         m_section = sect;
         emit sectionChanged();
      }
   }

   CS_PROPERTY_READ(delayRemove, delayRemove)
   CS_PROPERTY_WRITE(delayRemove, setDelayRemove)
   CS_PROPERTY_NOTIFY(delayRemove, delayRemoveChanged)
   bool delayRemove() const {
      return m_delayRemove;
   }
   void setDelayRemove(bool delay) {
      if (m_delayRemove != delay) {
         m_delayRemove = delay;
         emit delayRemoveChanged();
      }
   }

   void emitAdd() {
      emit add();
   }
   void emitRemove() {
      emit remove();
   }

 public:
   CS_SIGNAL_1(Public, void currentItemChanged())
   CS_SIGNAL_2(currentItemChanged)
   CS_SIGNAL_1(Public, void sectionChanged())
   CS_SIGNAL_2(sectionChanged)
   CS_SIGNAL_1(Public, void prevSectionChanged())
   CS_SIGNAL_2(prevSectionChanged)
   CS_SIGNAL_1(Public, void nextSectionChanged())
   CS_SIGNAL_2(nextSectionChanged)
   CS_SIGNAL_1(Public, void delayRemoveChanged())
   CS_SIGNAL_2(delayRemoveChanged)
   CS_SIGNAL_1(Public, void add())
   CS_SIGNAL_2(add)
   CS_SIGNAL_1(Public, void remove())
   CS_SIGNAL_2(remove)
   CS_SIGNAL_1(Public, void viewChanged())
   CS_SIGNAL_2(viewChanged)

   QDeclarativeGuard<QDeclarativeListView> m_view;
   mutable QString m_section;
   QString m_prevSection;
   QString m_nextSection;
   bool m_isCurrent : 1;
   bool m_delayRemove : 1;
};


QT_END_NAMESPACE

QML_DECLARE_TYPEINFO(QDeclarativeListView, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPE(QDeclarativeListView)
QML_DECLARE_TYPE(QDeclarativeViewSection)

#endif
