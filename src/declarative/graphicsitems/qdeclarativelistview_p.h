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

#ifndef QDECLARATIVELISTVIEW_P_H
#define QDECLARATIVELISTVIEW_P_H

#include <qdeclarativeflickable_p.h>
#include <qdeclarativeguard_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeViewSection : public QObject
{
   DECL_CS_OBJECT(QDeclarativeViewSection)

   DECL_CS_PROPERTY_READ(property, property)
   DECL_CS_PROPERTY_WRITE(property, setProperty)
   DECL_CS_PROPERTY_NOTIFY(property, propertyChanged)
   DECL_CS_PROPERTY_READ(criteria, criteria)
   DECL_CS_PROPERTY_WRITE(criteria, setCriteria)
   DECL_CS_PROPERTY_NOTIFY(criteria, criteriaChanged)
   DECL_CS_PROPERTY_READ(*delegate, delegate)
   DECL_CS_PROPERTY_WRITE(*delegate, setDelegate)
   DECL_CS_PROPERTY_NOTIFY(*delegate, delegateChanged)
   DECL_CS_ENUM(SectionCriteria)

 public:
   QDeclarativeViewSection(QObject *parent = nullptr) : QObject(parent), m_criteria(FullString), m_delegate(0) {}

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
   DECL_CS_SIGNAL_1(Public, void propertyChanged())
   DECL_CS_SIGNAL_2(propertyChanged)
   DECL_CS_SIGNAL_1(Public, void criteriaChanged())
   DECL_CS_SIGNAL_2(criteriaChanged)
   DECL_CS_SIGNAL_1(Public, void delegateChanged())
   DECL_CS_SIGNAL_2(delegateChanged)

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

   DECL_CS_PROPERTY_READ(model, model)
   DECL_CS_PROPERTY_WRITE(model, setModel)
   DECL_CS_PROPERTY_NOTIFY(model, modelChanged)
   DECL_CS_PROPERTY_READ(*delegate, delegate)
   DECL_CS_PROPERTY_WRITE(*delegate, setDelegate)
   DECL_CS_PROPERTY_NOTIFY(*delegate, delegateChanged)
   DECL_CS_PROPERTY_READ(currentIndex, currentIndex)
   DECL_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   DECL_CS_PROPERTY_NOTIFY(currentIndex, currentIndexChanged)
   DECL_CS_PROPERTY_READ(*currentItem, currentItem)
   DECL_CS_PROPERTY_NOTIFY(*currentItem, currentIndexChanged)
   DECL_CS_PROPERTY_READ(count, count)
   DECL_CS_PROPERTY_NOTIFY(count, countChanged)

   DECL_CS_PROPERTY_READ(*highlight, highlight)
   DECL_CS_PROPERTY_WRITE(*highlight, setHighlight)
   DECL_CS_PROPERTY_NOTIFY(*highlight, highlightChanged)
   DECL_CS_PROPERTY_READ(*highlightItem, highlightItem)
   DECL_CS_PROPERTY_NOTIFY(*highlightItem, highlightItemChanged)
   DECL_CS_PROPERTY_READ(highlightFollowsCurrentItem, highlightFollowsCurrentItem)
   DECL_CS_PROPERTY_WRITE(highlightFollowsCurrentItem, setHighlightFollowsCurrentItem)
   DECL_CS_PROPERTY_NOTIFY(highlightFollowsCurrentItem, highlightFollowsCurrentItemChanged)
   DECL_CS_PROPERTY_READ(highlightMoveSpeed, highlightMoveSpeed)
   DECL_CS_PROPERTY_WRITE(highlightMoveSpeed, setHighlightMoveSpeed)
   DECL_CS_PROPERTY_NOTIFY(highlightMoveSpeed, highlightMoveSpeedChanged)
   DECL_CS_PROPERTY_READ(highlightMoveDuration, highlightMoveDuration)
   DECL_CS_PROPERTY_WRITE(highlightMoveDuration, setHighlightMoveDuration)
   DECL_CS_PROPERTY_NOTIFY(highlightMoveDuration, highlightMoveDurationChanged)
   DECL_CS_PROPERTY_READ(highlightResizeSpeed, highlightResizeSpeed)
   DECL_CS_PROPERTY_WRITE(highlightResizeSpeed, setHighlightResizeSpeed)
   DECL_CS_PROPERTY_NOTIFY(highlightResizeSpeed, highlightResizeSpeedChanged)
   DECL_CS_PROPERTY_READ(highlightResizeDuration, highlightResizeDuration)
   DECL_CS_PROPERTY_WRITE(highlightResizeDuration, setHighlightResizeDuration)
   DECL_CS_PROPERTY_NOTIFY(highlightResizeDuration, highlightResizeDurationChanged)

   DECL_CS_PROPERTY_READ(preferredHighlightBegin, preferredHighlightBegin)
   DECL_CS_PROPERTY_WRITE(preferredHighlightBegin, setPreferredHighlightBegin)
   DECL_CS_PROPERTY_NOTIFY(preferredHighlightBegin, preferredHighlightBeginChanged)
   DECL_CS_PROPERTY_RESET(preferredHighlightBegin, resetPreferredHighlightBegin)
   DECL_CS_PROPERTY_READ(preferredHighlightEnd, preferredHighlightEnd)
   DECL_CS_PROPERTY_WRITE(preferredHighlightEnd, setPreferredHighlightEnd)
   DECL_CS_PROPERTY_NOTIFY(preferredHighlightEnd, preferredHighlightEndChanged)
   DECL_CS_PROPERTY_RESET(preferredHighlightEnd, resetPreferredHighlightEnd)
   DECL_CS_PROPERTY_READ(highlightRangeMode, highlightRangeMode)
   DECL_CS_PROPERTY_WRITE(highlightRangeMode, setHighlightRangeMode)
   DECL_CS_PROPERTY_NOTIFY(highlightRangeMode, highlightRangeModeChanged)

   DECL_CS_PROPERTY_READ(spacing, spacing)
   DECL_CS_PROPERTY_WRITE(spacing, setSpacing)
   DECL_CS_PROPERTY_NOTIFY(spacing, spacingChanged)
   DECL_CS_PROPERTY_READ(orientation, orientation)
   DECL_CS_PROPERTY_WRITE(orientation, setOrientation)
   DECL_CS_PROPERTY_NOTIFY(orientation, orientationChanged)
   DECL_CS_PROPERTY_READ(layoutDirection, layoutDirection)
   DECL_CS_PROPERTY_WRITE(layoutDirection, setLayoutDirection)
   DECL_CS_PROPERTY_NOTIFY(layoutDirection, layoutDirectionChanged)
   DECL_CS_PROPERTY_REVISION(layoutDirection, 1)
   DECL_CS_PROPERTY_READ(keyNavigationWraps, isWrapEnabled)
   DECL_CS_PROPERTY_WRITE(keyNavigationWraps, setWrapEnabled)
   DECL_CS_PROPERTY_NOTIFY(keyNavigationWraps, keyNavigationWrapsChanged)
   DECL_CS_PROPERTY_READ(cacheBuffer, cacheBuffer)
   DECL_CS_PROPERTY_WRITE(cacheBuffer, setCacheBuffer)
   DECL_CS_PROPERTY_NOTIFY(cacheBuffer, cacheBufferChanged)
   DECL_CS_PROPERTY_READ(*section, sectionCriteria)
   DECL_CS_PROPERTY_CONSTANT(*section)
   DECL_CS_PROPERTY_READ(currentSection, currentSection)
   DECL_CS_PROPERTY_NOTIFY(currentSection, currentSectionChanged)

   DECL_CS_PROPERTY_READ(snapMode, snapMode)
   DECL_CS_PROPERTY_WRITE(snapMode, setSnapMode)
   DECL_CS_PROPERTY_NOTIFY(snapMode, snapModeChanged)

   DECL_CS_PROPERTY_READ(*header, header)
   DECL_CS_PROPERTY_WRITE(*header, setHeader)
   DECL_CS_PROPERTY_NOTIFY(*header, headerChanged)
   DECL_CS_PROPERTY_READ(*footer, footer)
   DECL_CS_PROPERTY_WRITE(*footer, setFooter)
   DECL_CS_PROPERTY_NOTIFY(*footer, footerChanged)

   DECL_CS_ENUM(HighlightRangeMode)
   DECL_CS_ENUM(Orientation)
   DECL_CS_ENUM(SnapMode)
   DECL_CS_ENUM(PositionMode)

   DECL_CS_CLASSINFO("DefaultProperty", "data")

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

   DECL_CS_SLOT_1(Public, void incrementCurrentIndex())
   DECL_CS_SLOT_2(incrementCurrentIndex)
   DECL_CS_SLOT_1(Public, void decrementCurrentIndex())
   DECL_CS_SLOT_2(decrementCurrentIndex)

   DECL_CS_SIGNAL_1(Public, void countChanged())
   DECL_CS_SIGNAL_2(countChanged)
   DECL_CS_SIGNAL_1(Public, void spacingChanged())
   DECL_CS_SIGNAL_2(spacingChanged)
   DECL_CS_SIGNAL_1(Public, void orientationChanged())
   DECL_CS_SIGNAL_2(orientationChanged)

   DECL_CS_SIGNAL_1(Public, void layoutDirectionChanged())
   DECL_CS_SIGNAL_2(layoutDirectionChanged)
   CS_REVISION(layoutDirectionChanged, 1)

   DECL_CS_SIGNAL_1(Public, void currentIndexChanged())
   DECL_CS_SIGNAL_2(currentIndexChanged)
   DECL_CS_SIGNAL_1(Public, void currentSectionChanged())
   DECL_CS_SIGNAL_2(currentSectionChanged)
   DECL_CS_SIGNAL_1(Public, void highlightMoveSpeedChanged())
   DECL_CS_SIGNAL_2(highlightMoveSpeedChanged)
   DECL_CS_SIGNAL_1(Public, void highlightMoveDurationChanged())
   DECL_CS_SIGNAL_2(highlightMoveDurationChanged)
   DECL_CS_SIGNAL_1(Public, void highlightResizeSpeedChanged())
   DECL_CS_SIGNAL_2(highlightResizeSpeedChanged)
   DECL_CS_SIGNAL_1(Public, void highlightResizeDurationChanged())
   DECL_CS_SIGNAL_2(highlightResizeDurationChanged)
   DECL_CS_SIGNAL_1(Public, void highlightChanged())
   DECL_CS_SIGNAL_2(highlightChanged)
   DECL_CS_SIGNAL_1(Public, void highlightItemChanged())
   DECL_CS_SIGNAL_2(highlightItemChanged)
   DECL_CS_SIGNAL_1(Public, void modelChanged())
   DECL_CS_SIGNAL_2(modelChanged)
   DECL_CS_SIGNAL_1(Public, void delegateChanged())
   DECL_CS_SIGNAL_2(delegateChanged)
   DECL_CS_SIGNAL_1(Public, void highlightFollowsCurrentItemChanged())
   DECL_CS_SIGNAL_2(highlightFollowsCurrentItemChanged)
   DECL_CS_SIGNAL_1(Public, void preferredHighlightBeginChanged())
   DECL_CS_SIGNAL_2(preferredHighlightBeginChanged)
   DECL_CS_SIGNAL_1(Public, void preferredHighlightEndChanged())
   DECL_CS_SIGNAL_2(preferredHighlightEndChanged)
   DECL_CS_SIGNAL_1(Public, void highlightRangeModeChanged())
   DECL_CS_SIGNAL_2(highlightRangeModeChanged)
   DECL_CS_SIGNAL_1(Public, void keyNavigationWrapsChanged())
   DECL_CS_SIGNAL_2(keyNavigationWrapsChanged)
   DECL_CS_SIGNAL_1(Public, void cacheBufferChanged())
   DECL_CS_SIGNAL_2(cacheBufferChanged)
   DECL_CS_SIGNAL_1(Public, void snapModeChanged())
   DECL_CS_SIGNAL_2(snapModeChanged)
   DECL_CS_SIGNAL_1(Public, void headerChanged())
   DECL_CS_SIGNAL_2(headerChanged)
   DECL_CS_SIGNAL_1(Public, void footerChanged())
   DECL_CS_SIGNAL_2(footerChanged)

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
   DECL_CS_SLOT_1(Private, void updateSections())
   DECL_CS_SLOT_2(updateSections)
   DECL_CS_SLOT_1(Private, void refill())
   DECL_CS_SLOT_2(refill)
   DECL_CS_SLOT_1(Private, void trackedPositionChanged())
   DECL_CS_SLOT_2(trackedPositionChanged)
   DECL_CS_SLOT_1(Private, void itemsInserted(int index, int count))
   DECL_CS_SLOT_2(itemsInserted)
   DECL_CS_SLOT_1(Private, void itemsRemoved(int index, int count))
   DECL_CS_SLOT_2(itemsRemoved)
   DECL_CS_SLOT_1(Private, void itemsMoved(int from, int to, int count))
   DECL_CS_SLOT_2(itemsMoved)
   DECL_CS_SLOT_1(Private, void itemsChanged(int index, int count))
   DECL_CS_SLOT_2(itemsChanged)
   DECL_CS_SLOT_1(Private, void modelReset())
   DECL_CS_SLOT_2(modelReset)
   DECL_CS_SLOT_1(Private, void destroyRemoved())
   DECL_CS_SLOT_2(destroyRemoved)
   DECL_CS_SLOT_1(Private, void createdItem(int index, QDeclarativeItem *item))
   DECL_CS_SLOT_2(createdItem)
   DECL_CS_SLOT_1(Private, void destroyingItem(QDeclarativeItem *item))
   DECL_CS_SLOT_2(destroyingItem)
   DECL_CS_SLOT_1(Private, void animStopped())
   DECL_CS_SLOT_2(animStopped)
};

class QDeclarativeListViewAttached : public QObject
{
   DECL_CS_OBJECT(QDeclarativeListViewAttached)
 public:
   QDeclarativeListViewAttached(QObject *parent)
      : QObject(parent), m_view(0), m_isCurrent(false), m_delayRemove(false) {}
   ~QDeclarativeListViewAttached() {}

   DECL_CS_PROPERTY_READ(*view, view)
   DECL_CS_PROPERTY_NOTIFY(*view, viewChanged)
   QDeclarativeListView *view() {
      return m_view;
   }
   void setView(QDeclarativeListView *view) {
      if (view != m_view) {
         m_view = view;
         emit viewChanged();
      }
   }

   DECL_CS_PROPERTY_READ(isCurrentItem, isCurrentItem)
   DECL_CS_PROPERTY_NOTIFY(isCurrentItem, currentItemChanged)
   bool isCurrentItem() const {
      return m_isCurrent;
   }
   void setIsCurrentItem(bool c) {
      if (m_isCurrent != c) {
         m_isCurrent = c;
         emit currentItemChanged();
      }
   }

   DECL_CS_PROPERTY_READ(previousSection, prevSection)
   DECL_CS_PROPERTY_NOTIFY(previousSection, prevSectionChanged)
   QString prevSection() const {
      return m_prevSection;
   }
   void setPrevSection(const QString &sect) {
      if (m_prevSection != sect) {
         m_prevSection = sect;
         emit prevSectionChanged();
      }
   }

   DECL_CS_PROPERTY_READ(nextSection, nextSection)
   DECL_CS_PROPERTY_NOTIFY(nextSection, nextSectionChanged)
   QString nextSection() const {
      return m_nextSection;
   }
   void setNextSection(const QString &sect) {
      if (m_nextSection != sect) {
         m_nextSection = sect;
         emit nextSectionChanged();
      }
   }

   DECL_CS_PROPERTY_READ(section, section)
   DECL_CS_PROPERTY_NOTIFY(section, sectionChanged)
   QString section() const {
      return m_section;
   }
   void setSection(const QString &sect) {
      if (m_section != sect) {
         m_section = sect;
         emit sectionChanged();
      }
   }

   DECL_CS_PROPERTY_READ(delayRemove, delayRemove)
   DECL_CS_PROPERTY_WRITE(delayRemove, setDelayRemove)
   DECL_CS_PROPERTY_NOTIFY(delayRemove, delayRemoveChanged)
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
   DECL_CS_SIGNAL_1(Public, void currentItemChanged())
   DECL_CS_SIGNAL_2(currentItemChanged)
   DECL_CS_SIGNAL_1(Public, void sectionChanged())
   DECL_CS_SIGNAL_2(sectionChanged)
   DECL_CS_SIGNAL_1(Public, void prevSectionChanged())
   DECL_CS_SIGNAL_2(prevSectionChanged)
   DECL_CS_SIGNAL_1(Public, void nextSectionChanged())
   DECL_CS_SIGNAL_2(nextSectionChanged)
   DECL_CS_SIGNAL_1(Public, void delayRemoveChanged())
   DECL_CS_SIGNAL_2(delayRemoveChanged)
   DECL_CS_SIGNAL_1(Public, void add())
   DECL_CS_SIGNAL_2(add)
   DECL_CS_SIGNAL_1(Public, void remove())
   DECL_CS_SIGNAL_2(remove)
   DECL_CS_SIGNAL_1(Public, void viewChanged())
   DECL_CS_SIGNAL_2(viewChanged)

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
