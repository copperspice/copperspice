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

#include <qtextformat.h>
#include <qtextformat_p.h>

#include <qvariant.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qmap.h>
#include <qhashfunc.h>

QTextLength::operator QVariant() const
{
   return QVariant(QVariant::TextLength, this);
}


QDataStream &operator<<(QDataStream &stream, const QTextLength &length)
{
   return stream << qint32(length.lengthType) << double(length.fixedValueOrPercentage);
}

QDataStream &operator>>(QDataStream &stream, QTextLength &length)
{
   qint32 type;
   double fixedValueOrPercentage;
   stream >> type >> fixedValueOrPercentage;
   length.fixedValueOrPercentage = fixedValueOrPercentage;
   length.lengthType = QTextLength::Type(type);
   return stream;
}


class QTextFormatPrivate : public QSharedData
{
 public:
   QTextFormatPrivate() : hashDirty(true), fontDirty(true), hashValue(0) {}

   struct Property {
      inline Property(qint32 k, const QVariant &v) : key(k), value(v) {}
      inline Property() {}

      qint32 key;
      QVariant value;

      inline bool operator==(const Property &other) const {
         return key == other.key && value == other.value;
      }
      inline bool operator!=(const Property &other) const {
         return key != other.key || value != other.value;
      }
   };

   inline uint hash() const {
      if (!hashDirty) {
         return hashValue;
      }
      return recalcHash();
   }

   inline bool operator==(const QTextFormatPrivate &rhs) const {
      if (hash() != rhs.hash()) {
         return false;
      }

      return props == rhs.props;
   }

   inline void insertProperty(qint32 key, const QVariant &value) {
      hashDirty = true;
      if (key >= QTextFormat::FirstFontProperty && key <= QTextFormat::LastFontProperty) {
         fontDirty = true;
      }
      for (int i = 0; i < props.count(); ++i)
         if (props.at(i).key == key) {
            props[i].value = value;
            return;
         }
      props.append(Property(key, value));
   }

   inline void clearProperty(qint32 key) {
      for (int i = 0; i < props.count(); ++i)
         if (props.at(i).key == key) {
            hashDirty = true;
            if (key >= QTextFormat::FirstFontProperty && key <= QTextFormat::LastFontProperty) {
               fontDirty = true;
            }
            props.remove(i);
            return;
         }
   }

   inline int propertyIndex(qint32 key) const {
      for (int i = 0; i < props.count(); ++i)
         if (props.at(i).key == key) {
            return i;
         }
      return -1;
   }

   inline QVariant property(qint32 key) const {
      const int idx = propertyIndex(key);
      if (idx < 0) {
         return QVariant();
      }
      return props.at(idx).value;
   }

   inline bool hasProperty(qint32 key) const {
      return propertyIndex(key) != -1;
   }

   void resolveFont(const QFont &defaultFont);

   inline const QFont &font() const {
      if (fontDirty) {
         recalcFont();
      }
      return fnt;
   }

   QVector<Property> props;
 private:

   uint recalcHash() const;
   void recalcFont() const;

   mutable bool hashDirty;
   mutable bool fontDirty;
   mutable uint hashValue;
   mutable QFont fnt;

   friend QDataStream &operator<<(QDataStream &, const QTextFormat &);
   friend QDataStream &operator>>(QDataStream &, QTextFormat &);
};

static inline uint hash(const QColor &color)
{
   return (color.isValid()) ? color.rgba() : 0x234109;
}

static inline uint hash(const QPen &pen)
{
   return hash(pen.color()) + qHash(pen.widthF());
}

static inline uint hash(const QBrush &brush)
{
   return hash(brush.color()) + (brush.style() << 3);
}

static inline uint variantHash(const QVariant &variant)
{
   // simple and fast hash functions to differentiate between type and value
   switch (variant.userType()) { // sorted by occurrence frequency
      case QVariant::String:
         return qHash(variant.toString());

      case QVariant::Double:
         return qHash(variant.toDouble());

      case QVariant::Int:
         return 0x811890 + variant.toInt();

      case QVariant::Brush:
         return 0x01010101 + hash(qvariant_cast<QBrush>(variant));

      case QVariant::Bool:
         return 0x371818 + variant.toBool();

      case QVariant::Pen:
         return 0x02020202 + hash(qvariant_cast<QPen>(variant));

      case QVariant::List:
         return 0x8377 + qvariant_cast<QVariantList>(variant).count();

      case QVariant::Color:
         return hash(qvariant_cast<QColor>(variant));

      case QVariant::TextLength:
         return 0x377 + hash(qvariant_cast<QTextLength>(variant).rawValue());

      case QMetaType::Float:
         return qHash(variant.toFloat());

      case QVariant::Invalid:
         return 0;

      default:
         break;
   }
   return qHash(variant.typeName());
}

static inline int getHash(const QTextFormatPrivate *d, int format)
{
   return (d ? d->hash() : 0) + format;
}

uint QTextFormatPrivate::recalcHash() const
{
   hashValue = 0;
   for (QVector<Property>::const_iterator it = props.constBegin(); it != props.constEnd(); ++it) {
      hashValue += (static_cast<quint32>(it->key) << 16) + variantHash(it->value);
   }

   hashDirty = false;

   return hashValue;
}

void QTextFormatPrivate::resolveFont(const QFont &defaultFont)
{
   recalcFont();
   const uint oldMask = fnt.resolve();
   fnt = fnt.resolve(defaultFont);

   if (hasProperty(QTextFormat::FontSizeAdjustment)) {
      const qreal scaleFactors[7] = {qreal(0.7), qreal(0.8), qreal(1.0), qreal(1.2), qreal(1.5), qreal(2), qreal(2.4)};

      const int htmlFontSize = qBound(0, property(QTextFormat::FontSizeAdjustment).toInt() + 3 - 1, 6);


      if (defaultFont.pointSize() <= 0) {
         qreal pixelSize = scaleFactors[htmlFontSize] * defaultFont.pixelSize();
         fnt.setPixelSize(qRound(pixelSize));
      } else {
         qreal pointSize = scaleFactors[htmlFontSize] * defaultFont.pointSizeF();
         fnt.setPointSizeF(pointSize);
      }
   }

   fnt.resolve(oldMask);
}

void QTextFormatPrivate::recalcFont() const
{
   // update cached font as well
   QFont f;
   bool hasSpacingInformation = false;
   QFont::SpacingType spacingType = QFont::PercentageSpacing;
   qreal letterSpacing = 0.0;

   for (int i = 0; i < props.count(); ++i) {
      switch (props.at(i).key) {
         case QTextFormat::FontFamily:
            f.setFamily(props.at(i).value.toString());
            break;
         case QTextFormat::FontPointSize:
            f.setPointSizeF(props.at(i).value.toReal());
            break;
         case  QTextFormat::FontPixelSize:
            f.setPixelSize(props.at(i).value.toInt());
            break;

         case QTextFormat::FontWeight: {
            const QVariant weightValue = props.at(i).value;
            int weight = weightValue.toInt();

            if (weight >= 0 && weightValue.isValid()) {
               f.setWeight(weight);
            }

            break;
         }

         case QTextFormat::FontItalic:
            f.setItalic(props.at(i).value.toBool());
            break;
         case QTextFormat::FontUnderline:
            if (! hasProperty(QTextFormat::TextUnderlineStyle)) {
               // don't use the old one if the new one is there.
               f.setUnderline(props.at(i).value.toBool());
            }
            break;
         case QTextFormat::TextUnderlineStyle:
            f.setUnderline(static_cast<QTextCharFormat::UnderlineStyle>(props.at(i).value.toInt()) ==
               QTextCharFormat::SingleUnderline);
            break;
         case QTextFormat::FontOverline:
            f.setOverline(props.at(i).value.toBool());
            break;
         case QTextFormat::FontStrikeOut:
            f.setStrikeOut(props.at(i).value.toBool());
            break;

         case QTextFormat::FontLetterSpacingType:
            spacingType = static_cast<QFont::SpacingType>(props.at(i).value.toInt());
            hasSpacingInformation = true;
            break;

         case QTextFormat::FontLetterSpacing:
            letterSpacing = props.at(i).value.toReal();
            hasSpacingInformation = true;
            break;

         case QTextFormat::FontWordSpacing:
            f.setWordSpacing(props.at(i).value.toReal());
            break;

         case QTextFormat::FontCapitalization:
            f.setCapitalization(static_cast<QFont::Capitalization> (props.at(i).value.toInt()));
            break;

         case QTextFormat::FontFixedPitch: {
            const bool value = props.at(i).value.toBool();

            if (f.fixedPitch() != value) {
               f.setFixedPitch(value);
            }

            break;
         }

         case QTextFormat::FontStretch:
            f.setStretch(props.at(i).value.toInt());
            break;

         case QTextFormat::FontStyleHint:
            f.setStyleHint(static_cast<QFont::StyleHint>(props.at(i).value.toInt()), f.styleStrategy());
            break;
         case QTextFormat::FontHintingPreference:
            f.setHintingPreference(static_cast<QFont::HintingPreference>(props.at(i).value.toInt()));
            break;
         case QTextFormat::FontStyleStrategy:
            f.setStyleStrategy(static_cast<QFont::StyleStrategy>(props.at(i).value.toInt()));
            break;
         case QTextFormat::FontKerning:
            f.setKerning(props.at(i).value.toBool());
            break;
         default:
            break;
      }
   }
   if (hasSpacingInformation) {
      f.setLetterSpacing(spacingType, letterSpacing);
   }
   fnt = f;
   fontDirty = false;
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QTextFormat &fmt)
{
   stream << fmt.format_type << fmt.properties();
   return stream;
}

Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QTextFormat &fmt)
{
   QMap<qint32, QVariant> properties;
   stream >> fmt.format_type >> properties;

   // QTextFormat's default constructor doesn't allocate the private structure, so
   // we have to do this, in case fmt is a default constructed value.
   if (!fmt.d) {
      fmt.d = new QTextFormatPrivate();
   }

   for (QMap<qint32, QVariant>::const_iterator it = properties.constBegin();
      it != properties.constEnd(); ++it) {
      fmt.d->insertProperty(it.key(), it.value());
   }

   return stream;
}


QTextFormat::QTextFormat()
   : format_type(InvalidFormat)
{
}


QTextFormat::QTextFormat(int type)
   : format_type(type)
{
}


/*!
    \fn QTextFormat::QTextFormat(const QTextFormat &other)

    Creates a new text format with the same attributes as the \a other
    text format.
*/
QTextFormat::QTextFormat(const QTextFormat &rhs)
   : d(rhs.d), format_type(rhs.format_type)
{
}

/*!
    \fn QTextFormat &QTextFormat::operator=(const QTextFormat &other)

    Assigns the \a other text format to this text format, and returns a
    reference to this text format.
*/
QTextFormat &QTextFormat::operator=(const QTextFormat &rhs)
{
   d = rhs.d;
   format_type = rhs.format_type;
   return *this;
}


QTextFormat::~QTextFormat()
{
}


/*!
   Returns the text format as a QVariant
*/
QTextFormat::operator QVariant() const
{
   return QVariant(QVariant::TextFormat, this);
}

/*!
    Merges the \a other format with this format; where there are
    conflicts the \a other format takes precedence.
*/
void QTextFormat::merge(const QTextFormat &other)
{
   if (format_type != other.format_type) {
      return;
   }

   if (!d) {
      d = other.d;
      return;
   }

   if (!other.d) {
      return;
   }

   QTextFormatPrivate *d = this->d;

   const QVector<QTextFormatPrivate::Property> &otherProps = other.d->props;
   d->props.reserve(d->props.size() + otherProps.size());
   for (int i = 0; i < otherProps.count(); ++i) {
      const QTextFormatPrivate::Property &p = otherProps.at(i);
      d->insertProperty(p.key, p.value);
   }
}

/*!
    Returns the type of this format.

    \sa FormatType
*/
int QTextFormat::type() const
{
   return format_type;
}

/*!
    Returns this format as a block format.
*/
QTextBlockFormat QTextFormat::toBlockFormat() const
{
   return QTextBlockFormat(*this);
}

/*!
    Returns this format as a character format.
*/
QTextCharFormat QTextFormat::toCharFormat() const
{
   return QTextCharFormat(*this);
}

/*!
    Returns this format as a list format.
*/
QTextListFormat QTextFormat::toListFormat() const
{
   return QTextListFormat(*this);
}

/*!
    Returns this format as a table format.
*/
QTextTableFormat QTextFormat::toTableFormat() const
{
   return QTextTableFormat(*this);
}

/*!
    Returns this format as a frame format.
*/
QTextFrameFormat QTextFormat::toFrameFormat() const
{
   return QTextFrameFormat(*this);
}

/*!
    Returns this format as an image format.
*/
QTextImageFormat QTextFormat::toImageFormat() const
{
   return QTextImageFormat(*this);
}

/*!
    \since 4.4

    Returns this format as a table cell format.
*/
QTextTableCellFormat QTextFormat::toTableCellFormat() const
{
   return QTextTableCellFormat(*this);
}

/*!
    Returns the value of the property specified by \a propertyId. If the
    property isn't of QTextFormat::Bool type, false is returned instead.

    \sa setProperty() intProperty() doubleProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() Property
*/
bool QTextFormat::boolProperty(int propertyId) const
{
   if (!d) {
      return false;
   }
   const QVariant prop = d->property(propertyId);
   if (prop.userType() != QVariant::Bool) {
      return false;
   }
   return prop.toBool();
}

/*!
    Returns the value of the property specified by \a propertyId. If the
    property is not of QTextFormat::Integer type, 0 is returned instead.

    \sa setProperty() boolProperty() doubleProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() Property
*/
int QTextFormat::intProperty(int propertyId) const
{
   // required, since the default layout direction has to be LayoutDirectionAuto, which is not integer 0
   int def = (propertyId == QTextFormat::LayoutDirection) ? int(Qt::LayoutDirectionAuto) : 0;

   if (!d) {
      return def;
   }
   const QVariant prop = d->property(propertyId);
   if (prop.userType() != QVariant::Int) {
      return def;
   }
   return prop.toInt();
}

/*!
    Returns the value of the property specified by \a propertyId. If the
    property isn't of QVariant::Double or QMetaType::Float type, 0 is
    returned instead.

    \sa setProperty() boolProperty() intProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() Property
*/
qreal QTextFormat::doubleProperty(int propertyId) const
{
   if (!d) {
      return 0.;
   }
   const QVariant prop = d->property(propertyId);
   if (prop.userType() != QVariant::Double && prop.userType() != QMetaType::Float) {
      return 0.;
   }
   return qvariant_cast<qreal>(prop);
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of QVariant::String type, an empty string is
    returned instead.

    \sa setProperty() boolProperty() intProperty() doubleProperty() colorProperty() lengthProperty() lengthVectorProperty() Property
*/
QString QTextFormat::stringProperty(int propertyId) const
{
   if (!d) {
      return QString();
   }
   const QVariant prop = d->property(propertyId);
   if (prop.userType() != QVariant::String) {
      return QString();
   }
   return prop.toString();
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of QVariant::Color type, an invalid color is
    returned instead.

    \sa setProperty(), boolProperty(), intProperty(), doubleProperty(),
        stringProperty(), lengthProperty(), lengthVectorProperty(), Property
*/
QColor QTextFormat::colorProperty(int propertyId) const
{
   if (!d) {
      return QColor();
   }
   const QVariant prop = d->property(propertyId);
   if (prop.userType() != QVariant::Color) {
      return QColor();
   }
   return qvariant_cast<QColor>(prop);
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of QVariant::Pen type, Qt::NoPen is
    returned instead.

    \sa setProperty() boolProperty() intProperty() doubleProperty() stringProperty() lengthProperty() lengthVectorProperty() Property
*/
QPen QTextFormat::penProperty(int propertyId) const
{
   if (!d) {
      return QPen(Qt::NoPen);
   }
   const QVariant prop = d->property(propertyId);
   if (prop.userType() != QVariant::Pen) {
      return QPen(Qt::NoPen);
   }
   return qvariant_cast<QPen>(prop);
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of QVariant::Brush type, Qt::NoBrush is
    returned instead.

    \sa setProperty() boolProperty() intProperty() doubleProperty() stringProperty() lengthProperty() lengthVectorProperty() Property
*/
QBrush QTextFormat::brushProperty(int propertyId) const
{
   if (!d) {
      return QBrush(Qt::NoBrush);
   }
   const QVariant prop = d->property(propertyId);
   if (prop.userType() != QVariant::Brush) {
      return QBrush(Qt::NoBrush);
   }
   return qvariant_cast<QBrush>(prop);
}

/*!
    Returns the value of the property given by \a propertyId.

    \sa setProperty() boolProperty() intProperty() doubleProperty() stringProperty() colorProperty() lengthVectorProperty() Property
*/
QTextLength QTextFormat::lengthProperty(int propertyId) const
{
   if (!d) {
      return QTextLength();
   }
   return qvariant_cast<QTextLength>(d->property(propertyId));
}

/*!
    Returns the value of the property given by \a propertyId. If the
    property isn't of QTextFormat::LengthVector type, an empty length
    vector is returned instead.

    \sa setProperty() boolProperty() intProperty() doubleProperty() stringProperty() colorProperty() lengthProperty() Property
*/
QVector<QTextLength> QTextFormat::lengthVectorProperty(int propertyId) const
{
   QVector<QTextLength> vector;
   if (!d) {
      return vector;
   }
   const QVariant prop = d->property(propertyId);
   if (prop.userType() != QVariant::List) {
      return vector;
   }

   QList<QVariant> propertyList = prop.toList();
   for (int i = 0; i < propertyList.size(); ++i) {
      QVariant var = propertyList.at(i);
      if (var.userType() == QVariant::TextLength) {
         vector.append(qvariant_cast<QTextLength>(var));
      }
   }

   return vector;
}

/*!
    Returns the property specified by the given \a propertyId.

    \sa Property
*/
QVariant QTextFormat::property(int propertyId) const
{
   return d ? d->property(propertyId) : QVariant();
}

/*!
    Sets the property specified by the \a propertyId to the given \a value.

    \sa Property
*/
void QTextFormat::setProperty(int propertyId, const QVariant &value)
{
   if (!d) {
      d = new QTextFormatPrivate;
   }
   if (!value.isValid()) {
      clearProperty(propertyId);
   } else {
      d->insertProperty(propertyId, value);
   }
}

/*!
    Sets the value of the property given by \a propertyId to \a value.

    \sa lengthVectorProperty() Property
*/
void QTextFormat::setProperty(int propertyId, const QVector<QTextLength> &value)
{
   if (!d) {
      d = new QTextFormatPrivate;
   }
   QVariantList list;
   for (int i = 0; i < value.size(); ++i) {
      list << value.at(i);
   }
   d->insertProperty(propertyId, list);
}

/*!
    Clears the value of the property given by \a propertyId

    \sa Property
*/
void QTextFormat::clearProperty(int propertyId)
{
   if (!d) {
      return;
   }
   d->clearProperty(propertyId);
}


int QTextFormat::objectIndex() const
{
   if (!d) {
      return -1;
   }
   const QVariant prop = d->property(ObjectIndex);
   if (prop.userType() != QVariant::Int) { // ####
      return -1;
   }
   return prop.toInt();
}

/*!
    \fn void QTextFormat::setObjectIndex(int index)

    Sets the format object's object \a index.

    \sa objectIndex()
*/
void QTextFormat::setObjectIndex(int o)
{
   if (o == -1) {
      if (d) {
         d->clearProperty(ObjectIndex);
      }

   } else {
      if (!d) {
         d = new QTextFormatPrivate;
      }
      // ### type
      d->insertProperty(ObjectIndex, o);
   }
}

/*!
    Returns true if the text format has a property with the given \a
    propertyId; otherwise returns false.

    \sa properties() Property
*/
bool QTextFormat::hasProperty(int propertyId) const
{
   return d ? d->hasProperty(propertyId) : false;
}

/*
    Returns the property type for the given \a propertyId.

    \sa hasProperty() allPropertyIds() Property
*/

/*!
    Returns a map with all properties of this text format.
*/
QMap<int, QVariant> QTextFormat::properties() const
{
   QMap<int, QVariant> map;
   if (d) {
      for (int i = 0; i < d->props.count(); ++i) {
         map.insert(d->props.at(i).key, d->props.at(i).value);
      }
   }
   return map;
}

/*!
    \since 4.3
    Returns the number of properties stored in the format.
*/
int QTextFormat::propertyCount() const
{
   return d ? d->props.count() : 0;
}

/*!
    \fn bool QTextFormat::operator!=(const QTextFormat &other) const

    Returns true if this text format is different from the \a other text
    format.
*/


/*!
    \fn bool QTextFormat::operator==(const QTextFormat &other) const

    Returns true if this text format is the same as the \a other text
    format.
*/
bool QTextFormat::operator==(const QTextFormat &rhs) const
{
   if (format_type != rhs.format_type) {
      return false;
   }

   if (d == rhs.d) {
      return true;
   }

   if (d && d->props.isEmpty() && !rhs.d) {
      return true;
   }

   if (!d && rhs.d && rhs.d->props.isEmpty()) {
      return true;
   }

   if (!d || !rhs.d) {
      return false;
   }

   return *d == *rhs.d;
}

QTextCharFormat::QTextCharFormat() : QTextFormat(CharFormat) {}

/*!
    \internal
    \fn QTextCharFormat::QTextCharFormat(const QTextFormat &other)

    Creates a new character format with the same attributes as the \a given
    text format.
*/
QTextCharFormat::QTextCharFormat(const QTextFormat &fmt)
   : QTextFormat(fmt)
{
}


bool QTextCharFormat::fontUnderline() const
{
   if (hasProperty(TextUnderlineStyle)) {
      return underlineStyle() == SingleUnderline;
   }
   return boolProperty(FontUnderline);
}

/*!
    \fn UnderlineStyle QTextCharFormat::underlineStyle() const
    \since 4.2

    Returns the style of underlining the text.
*/

/*!
    \fn void QTextCharFormat::setUnderlineStyle(UnderlineStyle style)
    \since 4.2

    Sets the style of underlining the text to \a style.
*/
void QTextCharFormat::setUnderlineStyle(UnderlineStyle style)
{
   setProperty(TextUnderlineStyle, style);
   // for compatibility
   setProperty(FontUnderline, style == SingleUnderline);
}





QString QTextCharFormat::anchorName() const
{
   QVariant prop = property(AnchorName);
   if (prop.userType() == QVariant::StringList) {
      return prop.toStringList().value(0);
   } else if (prop.userType() != QVariant::String) {
      return QString();
   }
   return prop.toString();
}

/*!
    \fn QStringList QTextCharFormat::anchorNames() const
    \since 4.3

    Returns the anchor names associated with this text format, or an empty
    string list if none has been set. If the anchor names are set, text with this
    format can be the destination of a hypertext link.
*/
QStringList QTextCharFormat::anchorNames() const
{
   QVariant prop = property(AnchorName);

   if (prop.userType() == QVariant::StringList) {
      return prop.toStringList();
   } else if (prop.userType() != QVariant::String) {
      return QStringList();
   }

   return QStringList(prop.toString());
}


void QTextCharFormat::setFont(const QFont &font)
{
   setFont(font, FontPropertiesAll);
}
void QTextCharFormat::setFont(const QFont &font, FontPropertiesInheritanceBehavior behavior)
{
   const uint mask = behavior == FontPropertiesAll ? uint(QFont::AllPropertiesResolved)
      : font.resolve();

   if (mask & QFont::FamilyResolved) {
      setFontFamily(font.family());
   }

   if (mask & QFont::SizeResolved) {
      const qreal pointSize = font.pointSizeF();
      if (pointSize > 0) {
         setFontPointSize(pointSize);
      } else {
         const int pixelSize = font.pixelSize();
         if (pixelSize > 0) {
            setProperty(QTextFormat::FontPixelSize, pixelSize);
         }
      }
   }

   if (mask & QFont::WeightResolved) {
      setFontWeight(font.weight());
   }
   if (mask & QFont::StyleResolved) {
      setFontItalic(font.style() != QFont::StyleNormal);
   }
   if (mask & QFont::UnderlineResolved) {
      setUnderlineStyle(font.underline() ? SingleUnderline : NoUnderline);
   }
   if (mask & QFont::OverlineResolved) {
      setFontOverline(font.overline());
   }
   if (mask & QFont::StrikeOutResolved) {
      setFontStrikeOut(font.strikeOut());
   }
   if (mask & QFont::FixedPitchResolved) {
      setFontFixedPitch(font.fixedPitch());
   }
   if (mask & QFont::CapitalizationResolved) {
      setFontCapitalization(font.capitalization());
   }
   if (mask & QFont::WordSpacingResolved) {
      setFontWordSpacing(font.wordSpacing());
   }
   if (mask & QFont::LetterSpacingResolved) {
      setFontLetterSpacingType(font.letterSpacingType());
      setFontLetterSpacing(font.letterSpacing());
   }
   if (mask & QFont::StretchResolved) {
      setFontStretch(font.stretch());
   }
   if (mask & QFont::StyleHintResolved) {
      setFontStyleHint(font.styleHint());
   }
   if (mask & QFont::StyleStrategyResolved) {
      setFontStyleStrategy(font.styleStrategy());
   }
   if (mask & QFont::HintingPreferenceResolved) {
      setFontHintingPreference(font.hintingPreference());
   }
   if (mask & QFont::KerningResolved) {
      setFontKerning(font.kerning());
   }
}


QFont QTextCharFormat::font() const
{
   return d ? d->font() : QFont();
}


QTextBlockFormat::QTextBlockFormat() : QTextFormat(BlockFormat) {}

/*!
    \internal
    \fn QTextBlockFormat::QTextBlockFormat(const QTextFormat &other)

    Creates a new block format with the same attributes as the \a given
    text format.
*/
QTextBlockFormat::QTextBlockFormat(const QTextFormat &fmt)
   : QTextFormat(fmt)
{
}

/*!
    \since 4.4
    Sets the tab positions for the text block to those specified by
    \a tabs.

    \sa tabPositions()
*/
void QTextBlockFormat::setTabPositions(const QList<QTextOption::Tab> &tabs)
{
   QList<QVariant> list;
   QList<QTextOption::Tab>::const_iterator iter = tabs.constBegin();
   while (iter != tabs.constEnd()) {
      QVariant v;
      v.setValue<QTextOption::Tab>(*iter);
      list.append(v);
      ++iter;
   }
   setProperty(TabPositions, list);
}

QList<QTextOption::Tab> QTextBlockFormat::tabPositions() const
{
   QVariant variant = property(TabPositions);
   if (variant.isNull()) {
      return QList<QTextOption::Tab>();
   }

   QList<QTextOption::Tab> answer;
   QList<QVariant> variantsList = qvariant_cast<QList<QVariant>>(variant);
   QList<QVariant>::iterator iter = variantsList.begin();
   while (iter != variantsList.end()) {
      answer.append( qvariant_cast<QTextOption::Tab>(*iter));
      ++iter;
   }
   return answer;
}



QTextListFormat::QTextListFormat()
   : QTextFormat(ListFormat)
{
   setIndent(1);
}

/*!
    \internal
    \fn QTextListFormat::QTextListFormat(const QTextFormat &other)

    Creates a new list format with the same attributes as the \a given
    text format.
*/
QTextListFormat::QTextListFormat(const QTextFormat &fmt)
   : QTextFormat(fmt)
{
}


QTextFrameFormat::QTextFrameFormat() : QTextFormat(FrameFormat)
{
   setBorderStyle(BorderStyle_Outset);
   setBorderBrush(Qt::darkGray);
}

/*!
    \internal
    \fn QTextFrameFormat::QTextFrameFormat(const QTextFormat &other)

    Creates a new frame format with the same attributes as the \a given
    text format.
*/
QTextFrameFormat::QTextFrameFormat(const QTextFormat &fmt)
   : QTextFormat(fmt)
{
}

void QTextFrameFormat::setMargin(qreal amargin)
{
   setProperty(FrameMargin, amargin);
   setProperty(FrameTopMargin, amargin);
   setProperty(FrameBottomMargin, amargin);
   setProperty(FrameLeftMargin, amargin);
   setProperty(FrameRightMargin, amargin);
}

qreal QTextFrameFormat::topMargin() const
{
   if (!hasProperty(FrameTopMargin)) {
      return margin();
   }
   return doubleProperty(FrameTopMargin);
}


qreal QTextFrameFormat::bottomMargin() const
{
   if (!hasProperty(FrameBottomMargin)) {
      return margin();
   }
   return doubleProperty(FrameBottomMargin);
}

qreal QTextFrameFormat::leftMargin() const
{
   if (!hasProperty(FrameLeftMargin)) {
      return margin();
   }
   return doubleProperty(FrameLeftMargin);
}

qreal QTextFrameFormat::rightMargin() const
{
   if (!hasProperty(FrameRightMargin)) {
      return margin();
   }
   return doubleProperty(FrameRightMargin);
}

QTextTableFormat::QTextTableFormat()
   : QTextFrameFormat()
{
   setObjectType(TableObject);
   setCellSpacing(2);
   setBorder(1);
}

QTextTableFormat::QTextTableFormat(const QTextFormat &fmt)
   : QTextFrameFormat(fmt)
{
}

QTextImageFormat::QTextImageFormat() : QTextCharFormat()
{
   setObjectType(ImageObject);
}
QTextImageFormat::QTextImageFormat(const QTextFormat &fmt)
   : QTextCharFormat(fmt)
{
}

QTextTableCellFormat::QTextTableCellFormat()
   : QTextCharFormat()
{
   setObjectType(TableCellObject);
}


QTextTableCellFormat::QTextTableCellFormat(const QTextFormat &fmt)
   : QTextCharFormat(fmt)
{
}



QTextFormatCollection::QTextFormatCollection(const QTextFormatCollection &rhs)
{
   formats = rhs.formats;
   objFormats = rhs.objFormats;
}

QTextFormatCollection &QTextFormatCollection::operator=(const QTextFormatCollection &rhs)
{
   formats = rhs.formats;
   objFormats = rhs.objFormats;
   return *this;
}

QTextFormatCollection::~QTextFormatCollection()
{
}

int QTextFormatCollection::indexForFormat(const QTextFormat &format)
{
   uint hash = getHash(format.d, format.format_type);
   QMultiHash<uint, int>::const_iterator i = hashes.find(hash);

   while (i != hashes.constEnd() && i.key() == hash) {
      if (formats.value(i.value()) == format) {
         return i.value();
      }
      ++i;
   }

   int idx = formats.size();
   formats.append(format);

   QT_TRY {
      QTextFormat &f = formats.last();
      if (!f.d)
      {
         f.d = new QTextFormatPrivate;
      }
      f.d->resolveFont(defaultFnt);

      if (!hashes.contains(hash, idx))
      {
         hashes.insert(hash, idx);
      }

   } QT_CATCH(...) {
      formats.pop_back();
      QT_RETHROW;
   }
   return idx;
}

bool QTextFormatCollection::hasFormatCached(const QTextFormat &format) const
{
   uint hash = getHash(format.d, format.format_type);
   QMultiHash<uint, int>::const_iterator i = hashes.find(hash);
   while (i != hashes.constEnd() && i.key() == hash) {
      if (formats.value(i.value()) == format) {
         return true;
      }
      ++i;
   }
   return false;
}

int QTextFormatCollection::objectFormatIndex(int objectIndex) const
{
   if (objectIndex == -1) {
      return -1;
   }
   return objFormats.at(objectIndex);
}

void QTextFormatCollection::setObjectFormatIndex(int objectIndex, int formatIndex)
{
   objFormats[objectIndex] = formatIndex;
}

int QTextFormatCollection::createObjectIndex(const QTextFormat &f)
{
   const int objectIndex = objFormats.size();
   objFormats.append(indexForFormat(f));
   return objectIndex;
}

QTextFormat QTextFormatCollection::format(int idx) const
{
   if (idx < 0 || idx >= formats.count()) {
      return QTextFormat();
   }

   return formats.at(idx);
}

void QTextFormatCollection::setDefaultFont(const QFont &f)
{
   defaultFnt = f;

   for (int i = 0; i < formats.count(); ++i) {
      if (formats[i].d) {
         formats[i].d->resolveFont(defaultFnt);
      }
   }
}

QDebug operator<<(QDebug dbg, const QTextLength &l)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QTextLength(QTextLength::Type(" << l.type() << "))";
   return dbg;
}

QDebug operator<<(QDebug dbg, const QTextFormat &f)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QTextFormat(QTextFormat::FormatType(" << f.type() << "))";
   return dbg;
}
