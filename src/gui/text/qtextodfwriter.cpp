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

#include <qglobal.h>

#ifndef QT_NO_TEXTODFWRITER

#include <qtextodfwriter_p.h>

#include <QDebug>
#include <QImageWriter>
#include <QTextListFormat>
#include <QTextList>
#include <QBuffer>
#include <QUrl>

#include <qtextdocument_p.h>
#include <qtexttable.h>
#include <qtextcursor.h>
#include <qtextimagehandler_p.h>
#include <qzipwriter_p.h>

// Convert pixels to postscript point units
static QString pixelToPoint(qreal pixels)
{
   // we hardcode 96 DPI, we do the same in the ODF importer to have a perfect roundtrip.
   return QString::number(pixels * 72 / 96) + "pt";
}

// strategies
class QOutputStrategy
{
 public:
   QOutputStrategy()
      : contentStream(nullptr), counter(1)
   { }

   virtual ~QOutputStrategy() {}
   virtual void addFile(const QString &fileName, const QString &mimeType, const QByteArray &bytes) = 0;

   QString createUniqueImageName() {
      return QString::fromLatin1("Pictures/Picture%1").formatArg(counter++);
   }

   QIODevice *contentStream;
   int counter;
};

class QXmlStreamStrategy : public QOutputStrategy
{
 public:
   QXmlStreamStrategy(QIODevice *device) {
      contentStream = device;
   }

   virtual ~QXmlStreamStrategy() {
      if (contentStream) {
         contentStream->close();
      }
   }

   void addFile(const QString &, const QString &, const QByteArray &) override {
      // ignore this
   }
};

class QZipStreamStrategy : public QOutputStrategy
{
 public:
   QZipStreamStrategy(QIODevice *device)
      : zip(device), manifestWriter(&manifest) {
      QByteArray mime("application/vnd.oasis.opendocument.text");
      zip.setCompressionPolicy(QZipWriter::NeverCompress);
      zip.addFile(QString::fromLatin1("mimetype"), mime); // for mime-magick
      zip.setCompressionPolicy(QZipWriter::AutoCompress);
      contentStream = &content;
      content.open(QIODevice::WriteOnly);
      manifest.open(QIODevice::WriteOnly);

      manifestNS = QString::fromLatin1("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0");
      // prettyfy
      manifestWriter.setAutoFormatting(true);
      manifestWriter.setAutoFormattingIndent(1);

      manifestWriter.writeNamespace(manifestNS, QString::fromLatin1("manifest"));
      manifestWriter.writeStartDocument();
      manifestWriter.writeStartElement(manifestNS, QString::fromLatin1("manifest"));
      manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("version"), QString::fromLatin1("1.2"));
      addFile(QString::fromLatin1("/"), QString::fromLatin1("application/vnd.oasis.opendocument.text"));
      addFile(QString::fromLatin1("content.xml"), QString::fromLatin1("text/xml"));
   }

   ~QZipStreamStrategy() {
      manifestWriter.writeEndDocument();
      manifest.close();
      zip.addFile(QString::fromLatin1("META-INF/manifest.xml"), &manifest);
      content.close();
      zip.addFile(QString::fromLatin1("content.xml"), &content);
      zip.close();
   }

   void addFile(const QString &fileName, const QString &mimeType, const QByteArray &bytes) override {
      zip.addFile(fileName, bytes);
      addFile(fileName, mimeType);
   }

 private:
   void addFile(const QString &fileName, const QString &mimeType) {
      manifestWriter.writeEmptyElement(manifestNS, QString::fromLatin1("file-entry"));
      manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("media-type"), mimeType);
      manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("full-path"), fileName);
   }

   QBuffer content;
   QBuffer manifest;
   QZipWriter zip;
   QXmlStreamWriter manifestWriter;
   QString manifestNS;
};

static QString bulletChar(QTextListFormat::Style style)
{
   switch (style) {
      case QTextListFormat::ListDisc:
         return QChar(0x25cf); // bullet character

      case QTextListFormat::ListCircle:
         return QChar(0x25cb); // white circle

      case QTextListFormat::ListSquare:
         return QChar(0x25a1); // white square

      case QTextListFormat::ListDecimal:
         return QString::fromLatin1("1");

      case QTextListFormat::ListLowerAlpha:
         return QString::fromLatin1("a");

      case QTextListFormat::ListUpperAlpha:
         return QString::fromLatin1("A");

      case QTextListFormat::ListLowerRoman:
         return QString::fromLatin1("i");

      case QTextListFormat::ListUpperRoman:
         return QString::fromLatin1("I");

      case QTextListFormat::ListStyleUndefined:
      default:
         return QString();
   }
}

void QTextOdfWriter::writeFrame(QXmlStreamWriter &writer, const QTextFrame *frame)
{
   Q_ASSERT(frame);
   const QTextTable *table = qobject_cast<const QTextTable *> (frame);

   if (table) { // Start a table.
      writer.writeStartElement(tableNS, QString::fromLatin1("table"));
      writer.writeEmptyElement(tableNS, QString::fromLatin1("table-column"));
      writer.writeAttribute(tableNS, QString::fromLatin1("number-columns-repeated"), QString::number(table->columns()));

   } else if (frame->document() && frame->document()->rootFrame() != frame) { // start a section
      writer.writeStartElement(textNS, QString::fromLatin1("section"));
   }

   QTextFrame::iterator iterator = frame->begin();
   QTextFrame *child = nullptr;

   int tableRow = -1;
   while (! iterator.atEnd()) {

      if (iterator.currentFrame() && child != iterator.currentFrame()) {
         writeFrame(writer, iterator.currentFrame());

      } else { // no frame, its a block
         QTextBlock block = iterator.currentBlock();

         if (table) {
            QTextTableCell cell = table->cellAt(block.position());

            if (tableRow < cell.row()) {
               if (tableRow >= 0) {
                  writer.writeEndElement();   // close table row
               }
               tableRow = cell.row();
               writer.writeStartElement(tableNS, QString::fromLatin1("table-row"));
            }

            writer.writeStartElement(tableNS, QString::fromLatin1("table-cell"));
            if (cell.columnSpan() > 1) {
               writer.writeAttribute(tableNS, QString::fromLatin1("number-columns-spanned"), QString::number(cell.columnSpan()));
            }

            if (cell.rowSpan() > 1) {
               writer.writeAttribute(tableNS, QString::fromLatin1("number-rows-spanned"), QString::number(cell.rowSpan()));
            }

            if (cell.format().isTableCellFormat()) {
               writer.writeAttribute(tableNS, "style-name",
                  QString("T%1").formatArg(cell.tableCellFormatIndex()));
            }
         }
         writeBlock(writer, block);
         if (table) {
            writer.writeEndElement();   // table-cell
         }
      }

      child = iterator.currentFrame();
      ++iterator;
   }

   if (tableRow >= 0) {
      writer.writeEndElement();   // close table-row
   }

   if (table || (frame->document() && frame->document()->rootFrame() != frame)) {
      writer.writeEndElement();   // close table or section element
   }
}

void QTextOdfWriter::writeBlock(QXmlStreamWriter &writer, const QTextBlock &block)
{
   if (block.textList()) { // its a list-item
      const int listLevel = block.textList()->format().indent();

      if (m_listStack.isEmpty() || m_listStack.top() != block.textList()) {
         // not the same list we were in

         while (m_listStack.count() >= listLevel && ! m_listStack.isEmpty() &&
            m_listStack.top() != block.textList() ) {
            // we need to close tags
            m_listStack.pop();
            writer.writeEndElement(); // list

            if (m_listStack.count()) {
               writer.writeEndElement();   // list-item
            }

         }

         while (m_listStack.count() < listLevel) {
            if (m_listStack.count()) {
               writer.writeStartElement(textNS, QString::fromLatin1("list-item"));
            }

            writer.writeStartElement(textNS, QString::fromLatin1("list"));
            if (m_listStack.count() == listLevel - 1) {
               m_listStack.push(block.textList());
               writer.writeAttribute(textNS, "style-name", QString("L%1")
                  .formatArg(block.textList()->formatIndex()));
            } else {
               m_listStack.push(nullptr);
            }
         }
      }
      writer.writeStartElement(textNS, QString::fromLatin1("list-item"));

   } else {
      while (! m_listStack.isEmpty()) {
         m_listStack.pop();
         writer.writeEndElement(); // list
         if (m_listStack.count()) {
            writer.writeEndElement();   // list-item
         }
      }
   }

   if (block.length() == 1) { // only a linefeed
      writer.writeEmptyElement(textNS, QString::fromLatin1("p"));
      writer.writeAttribute(textNS, "style-name", QString("p%1")
         .formatArg(block.blockFormatIndex()));

      if (block.textList()) {
         writer.writeEndElement();   // numbered-paragraph
      }
      return;
   }

   writer.writeStartElement(textNS, QString::fromLatin1("p"));
   writer.writeAttribute(textNS, "style-name", QString("p%1")
      .formatArg(block.blockFormatIndex()));

   for (QTextBlock::iterator frag = block.begin(); !frag.atEnd(); ++frag) {

      bool isHyperlink = frag.fragment().charFormat().hasProperty(QTextFormat::AnchorHref);
      if (isHyperlink) {
         QString value = frag.fragment().charFormat().property(QTextFormat::AnchorHref).toString();
         writer.writeStartElement(textNS, "a");
         writer.writeAttribute(xlinkNS, "href", value);
      }

      writer.writeCharacters(QString()); // Trick to make sure that the span gets no linefeed in front of it.
      writer.writeStartElement(textNS, QString::fromLatin1("span"));

      QString fragmentText = frag.fragment().text();

      if (fragmentText.length() == 1 && fragmentText[0] == 0xFFFC) {
         // its an inline character.
         writeInlineCharacter(writer, frag.fragment());
         writer.writeEndElement(); // span
         continue;
      }

      writer.writeAttribute(textNS, "style-name", QString("c%1")
         .formatArg(frag.fragment().charFormatIndex()));

      bool escapeNextSpace = true;
      int precedingSpaces = 0;
      int exportedIndex = 0;

      for (int i = 0; i <= fragmentText.count(); ++i) {

         QChar character = fragmentText[i];
         bool isSpace = character.unicode() == ' ';

         // find more than one space. -> <text:s text:c="2" />
         if (!isSpace && escapeNextSpace && precedingSpaces > 1) {
            const bool startParag = exportedIndex == 0 && i == precedingSpaces;
            if (!startParag) {
               writer.writeCharacters(fragmentText.mid(exportedIndex, i - precedingSpaces + 1 - exportedIndex));
            }

            writer.writeEmptyElement(textNS, QString::fromLatin1("s"));
            const int count = precedingSpaces - (startParag ? 0 : 1);

            if (count > 1) {
               writer.writeAttribute(textNS, QString::fromLatin1("c"), QString::number(count));
            }

            precedingSpaces = 0;
            exportedIndex = i;
         }

         if (i < fragmentText.count()) {
            if (character.unicode() == 0x2028) { // soft-return
               //if (exportedIndex < i)
               writer.writeCharacters(fragmentText.mid(exportedIndex, i - exportedIndex));
               writer.writeEmptyElement(textNS, QString::fromLatin1("line-break"));
               exportedIndex = i + 1;
               continue;
            } else if (character.unicode() == '\t') { // Tab
               //if (exportedIndex < i)
               writer.writeCharacters(fragmentText.mid(exportedIndex, i - exportedIndex));
               writer.writeEmptyElement(textNS, QString::fromLatin1("tab"));
               exportedIndex = i + 1;
               precedingSpaces = 0;
            } else if (isSpace) {
               ++precedingSpaces;
               escapeNextSpace = true;
            } else if (!isSpace) {
               precedingSpaces = 0;
            }
         }
      }

      writer.writeCharacters(fragmentText.mid(exportedIndex));
      writer.writeEndElement(); // span

      writer.writeCharacters(QString()); // Trick to make sure that the span gets no linefeed behind it.

      if (isHyperlink) {
         writer.writeEndElement();   // a
      }
   }

   writer.writeCharacters(QString()); // Trick to make sure that the span gets no linefeed behind it.
   writer.writeEndElement(); // p

   if (block.textList()) {
      writer.writeEndElement();   // list-item
   }
}

void QTextOdfWriter::writeInlineCharacter(QXmlStreamWriter &writer, const QTextFragment &fragment) const
{
   writer.writeStartElement(drawNS, QString::fromLatin1("frame"));

   if (m_strategy == nullptr) {
      // don't do anything

   } else if (fragment.charFormat().isImageFormat()) {
      QTextImageFormat imageFormat = fragment.charFormat().toImageFormat();
      writer.writeAttribute(drawNS, QString::fromLatin1("name"), imageFormat.name());

      QImage image;
      QString name = imageFormat.name();

      if (name.startsWith(":/")) {
         // auto-detect resources
         name.prepend(QString("qrc"));
      }

      QUrl url = QUrl::fromEncoded(name.toUtf8());

      const QVariant data = m_document->resource(QTextDocument::ImageResource, url);
      if (data.type() == QVariant::Image) {
         image = data.value<QImage>();

      } else if (data.type() == QVariant::ByteArray) {
         image.loadFromData(data.toByteArray());
      }

      if (image.isNull()) {
         QString context;

         if (image.isNull()) {
            // try direct loading
            name = imageFormat.name(); // remove qrc:/ prefix again
            image.load(name);
         }
      }

      if (! image.isNull()) {
         QBuffer imageBytes;
         QImageWriter imageWriter(&imageBytes, "png");
         imageWriter.write(image);
         QString filename = m_strategy->createUniqueImageName();
         m_strategy->addFile(filename, QString::fromLatin1("image/png"), imageBytes.data());

         // get the width/height from the format.
         qreal width = (imageFormat.hasProperty(QTextFormat::ImageWidth)) ? imageFormat.width() : image.width();
         writer.writeAttribute(svgNS, QString::fromLatin1("width"), pixelToPoint(width));
         qreal height = (imageFormat.hasProperty(QTextFormat::ImageHeight)) ? imageFormat.height() : image.height();
         writer.writeAttribute(svgNS, QString::fromLatin1("height"), pixelToPoint(height));

         writer.writeStartElement(drawNS, QString::fromLatin1("image"));
         writer.writeAttribute(xlinkNS, QString::fromLatin1("href"), filename);
         writer.writeEndElement(); // image
      }
   }

   writer.writeEndElement(); // frame
}

void QTextOdfWriter::writeFormats(QXmlStreamWriter &writer, const QSet<int> &formats) const
{
   writer.writeStartElement(officeNS, QString::fromLatin1("automatic-styles"));
   QVector<QTextFormat> allStyles = m_document->allFormats();
   QSetIterator<int> formatId(formats);

   while (formatId.hasNext()) {
      int formatIndex = formatId.next();
      QTextFormat textFormat = allStyles.at(formatIndex);

      switch (textFormat.type()) {
         case QTextFormat::CharFormat:
            if (textFormat.isTableCellFormat()) {
               writeTableCellFormat(writer, textFormat.toTableCellFormat(), formatIndex);
            } else {
               writeCharacterFormat(writer, textFormat.toCharFormat(), formatIndex);
            }
            break;

         case QTextFormat::BlockFormat:
            writeBlockFormat(writer, textFormat.toBlockFormat(), formatIndex);
            break;

         case QTextFormat::ListFormat:
            writeListFormat(writer, textFormat.toListFormat(), formatIndex);
            break;

         case QTextFormat::FrameFormat:
            writeFrameFormat(writer, textFormat.toFrameFormat(), formatIndex);
            break;

         case QTextFormat::TableFormat:

            break;
      }
   }

   writer.writeEndElement(); // automatic-styles
}

void QTextOdfWriter::writeBlockFormat(QXmlStreamWriter &writer, QTextBlockFormat format, int formatIndex) const
{
   writer.writeStartElement(styleNS, QString::fromLatin1("style"));
   writer.writeAttribute(styleNS, "name",   QString("p%1").formatArg(formatIndex));
   writer.writeAttribute(styleNS, "family", QString("paragraph"));
   writer.writeStartElement(styleNS, QString::fromLatin1("paragraph-properties"));

   if (format.hasProperty(QTextFormat::BlockAlignment)) {
      const Qt::Alignment alignment = format.alignment() & Qt::AlignHorizontal_Mask;
      QString value;
      if (alignment == Qt::AlignLeading) {
         value = QString::fromLatin1("start");
      } else if (alignment == Qt::AlignTrailing) {
         value = QString::fromLatin1("end");
      } else if (alignment == (Qt::AlignLeft | Qt::AlignAbsolute)) {
         value = QString::fromLatin1("left");
      } else if (alignment == (Qt::AlignRight | Qt::AlignAbsolute)) {
         value = QString::fromLatin1("right");
      } else if (alignment == Qt::AlignHCenter) {
         value = QString::fromLatin1("center");
      } else if (alignment == Qt::AlignJustify) {
         value = QString::fromLatin1("justify");
      } else {
         qWarning() << "QTextOdfWriter::writeBlockFormat() Unsupported paragraph alignment: " << format.alignment();
      }

      if (! value.isEmpty()) {
         writer.writeAttribute(foNS, QString::fromLatin1("text-align"), value);
      }
   }

   if (format.hasProperty(QTextFormat::BlockTopMargin)) {
      writer.writeAttribute(foNS, QString::fromLatin1("margin-top"), pixelToPoint(qMax(qreal(0.), format.topMargin())) );
   }

   if (format.hasProperty(QTextFormat::BlockBottomMargin)) {
      writer.writeAttribute(foNS, QString::fromLatin1("margin-bottom"), pixelToPoint(qMax(qreal(0.),
               format.bottomMargin())) );
   }

   if (format.hasProperty(QTextFormat::BlockLeftMargin) || format.hasProperty(QTextFormat::BlockIndent))
      writer.writeAttribute(foNS, QString::fromLatin1("margin-left"), pixelToPoint(qMax(qreal(0.),
               format.leftMargin() + format.indent())));
   if (format.hasProperty(QTextFormat::BlockRightMargin)) {
      writer.writeAttribute(foNS, QString::fromLatin1("margin-right"), pixelToPoint(qMax(qreal(0.), format.rightMargin())) );
   }
   if (format.hasProperty(QTextFormat::TextIndent)) {
      writer.writeAttribute(foNS, QString::fromLatin1("text-indent"), pixelToPoint(format.textIndent()));
   }
   if (format.hasProperty(QTextFormat::PageBreakPolicy)) {
      if (format.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore) {
         writer.writeAttribute(foNS, QString::fromLatin1("break-before"), QString::fromLatin1("page"));
      }
      if (format.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter) {
         writer.writeAttribute(foNS, QString::fromLatin1("break-after"), QString::fromLatin1("page"));
      }
   }
   if (format.hasProperty(QTextFormat::BackgroundBrush)) {
      QBrush brush = format.background();
      writer.writeAttribute(foNS, QString::fromLatin1("background-color"), brush.color().name());
   }
   if (format.hasProperty(QTextFormat::BlockNonBreakableLines)) {
      writer.writeAttribute(foNS, QString("keep-together"), format.nonBreakableLines() ? QString("true") : QString("false"));
   }

   if (format.hasProperty(QTextFormat::TabPositions)) {
      QList<QTextOption::Tab> tabs = format.tabPositions();
      writer.writeStartElement(styleNS, QString::fromLatin1("tab-stops"));
      QList<QTextOption::Tab>::iterator iterator = tabs.begin();

      while (iterator != tabs.end()) {
         writer.writeEmptyElement(styleNS, QString::fromLatin1("tab-stop"));
         writer.writeAttribute(styleNS, QString::fromLatin1("position"), pixelToPoint(iterator->position) );
         QString type;
         switch (iterator->type) {
            case QTextOption::DelimiterTab:
               type = "char";
               break;

            case QTextOption::LeftTab:
               type = "left";
               break;

            case QTextOption::RightTab:
               type = "right";
               break;

            case QTextOption::CenterTab:
               type = "center";
               break;
         }

         writer.writeAttribute(styleNS, QString::fromLatin1("type"), type);
         if (iterator->delimiter != 0) {
            writer.writeAttribute(styleNS, QString::fromLatin1("char"), iterator->delimiter);
         }

         ++iterator;
      }

      writer.writeEndElement(); // tab-stops
   }

   writer.writeEndElement(); // paragraph-properties
   writer.writeEndElement(); // style
}

void QTextOdfWriter::writeCharacterFormat(QXmlStreamWriter &writer, QTextCharFormat format, int formatIndex) const
{
   writer.writeStartElement(styleNS, QString::fromLatin1("style"));
   writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("c%1").formatArg(formatIndex));
   writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("text"));
   writer.writeEmptyElement(styleNS, QString::fromLatin1("text-properties"));

   if (format.fontItalic()) {
      writer.writeAttribute(foNS, QString::fromLatin1("font-style"), QString::fromLatin1("italic"));
   }

   if (format.hasProperty(QTextFormat::FontWeight) && format.fontWeight() != QFont::Normal) {
      QString value;
      if (format.fontWeight() == QFont::Bold) {
         value = QString::fromLatin1("bold");
      } else {
         value = QString::number(format.fontWeight() * 10);
      }
      writer.writeAttribute(foNS, QString::fromLatin1("font-weight"), value);
   }

   if (format.hasProperty(QTextFormat::FontFamily)) {
      writer.writeAttribute(foNS, QString::fromLatin1("font-family"), format.fontFamily());
   } else {
      writer.writeAttribute(foNS, QString::fromLatin1("font-family"), QString::fromLatin1("Sans"));   // Qt default
   }

   if (format.hasProperty(QTextFormat::FontPointSize)) {
      writer.writeAttribute(foNS, QString::fromLatin1("font-size"), QString::fromLatin1("%1pt").formatArg(format.fontPointSize()));
   }

   if (format.hasProperty(QTextFormat::FontCapitalization)) {
      switch (format.fontCapitalization()) {
         case QFont::MixedCase:
            writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("none"));
            break;
         case QFont::AllUppercase:
            writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("uppercase"));
            break;
         case QFont::AllLowercase:
            writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("lowercase"));
            break;
         case QFont::Capitalize:
            writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("capitalize"));
            break;
         case QFont::SmallCaps:
            writer.writeAttribute(foNS, QString::fromLatin1("font-variant"), QString::fromLatin1("small-caps"));
            break;
      }
   }

   if (format.hasProperty(QTextFormat::FontLetterSpacing)) {
      writer.writeAttribute(foNS, QString::fromLatin1("letter-spacing"), pixelToPoint(format.fontLetterSpacing()));
   }

   if (format.hasProperty(QTextFormat::FontWordSpacing) && format.fontWordSpacing() != 0) {
      writer.writeAttribute(foNS, QString::fromLatin1("word-spacing"), pixelToPoint(format.fontWordSpacing()));
   }

   if (format.hasProperty(QTextFormat::FontUnderline))
      writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-type"),
         format.fontUnderline() ? QString::fromLatin1("single") : QString::fromLatin1("none"));
   if (format.hasProperty(QTextFormat::FontOverline)) {
      //   bool   fontOverline () const  TODO
   }
   if (format.hasProperty(QTextFormat::FontStrikeOut))
      writer.writeAttribute(styleNS, QString::fromLatin1( "text-line-through-type"),
         format.fontStrikeOut() ? QString::fromLatin1("single") : QString::fromLatin1("none"));
   if (format.hasProperty(QTextFormat::TextUnderlineColor)) {
      writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-color"), format.underlineColor().name());
   }

   if (format.hasProperty(QTextFormat::FontFixedPitch)) {
      //   bool   fontFixedPitch () const  TODO
   }

   if (format.hasProperty(QTextFormat::TextUnderlineStyle)) {
      QString value;
      switch (format.underlineStyle()) {
         case QTextCharFormat::NoUnderline:
            value = QString::fromLatin1("none");
            break;

         case QTextCharFormat::SingleUnderline:
            value = QString::fromLatin1("solid");
            break;

         case QTextCharFormat::DashUnderline:
            value = QString::fromLatin1("dash");
            break;

         case QTextCharFormat::DotLine:
            value = QString::fromLatin1("dotted");
            break;

         case QTextCharFormat::DashDotLine:
            value = QString::fromLatin1("dash-dot");
            break;

         case QTextCharFormat::DashDotDotLine:
            value = QString::fromLatin1("dot-dot-dash");
            break;

         case QTextCharFormat::WaveUnderline:
            value = QString::fromLatin1("wave");
            break;

         case QTextCharFormat::SpellCheckUnderline:
            value = QString::fromLatin1("none");
            break;
      }
      writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-style"), value);
   }
   if (format.hasProperty(QTextFormat::TextVerticalAlignment)) {
      QString value;

      switch (format.verticalAlignment()) {
         case QTextCharFormat::AlignMiddle:
         case QTextCharFormat::AlignNormal:
            value = QString::fromLatin1("0%");
            break;

         case QTextCharFormat::AlignSuperScript:
            value = QString::fromLatin1("super");
            break;

         case QTextCharFormat::AlignSubScript:
            value = QString::fromLatin1("sub");
            break;

         case QTextCharFormat::AlignTop:
            value = QString::fromLatin1("100%");
            break;

         case QTextCharFormat::AlignBottom:
            value = QString::fromLatin1("-100%");
            break;

         case QTextCharFormat::AlignBaseline:
            break;

      }
      writer.writeAttribute(styleNS, QString::fromLatin1("text-position"), value);
   }

   if (format.hasProperty(QTextFormat::TextOutline)) {
      writer.writeAttribute(styleNS, QString::fromLatin1("text-outline"), QString::fromLatin1("true"));
   }

   if (format.hasProperty(QTextFormat::TextToolTip)) {
      //   QString   toolTip () const  TODO
   }
   if (format.hasProperty(QTextFormat::IsAnchor)) {
      //   bool   isAnchor () const  TODO
   }
   if (format.hasProperty(QTextFormat::AnchorHref)) {
      //   QString   anchorHref () const  TODO
   }
   if (format.hasProperty(QTextFormat::AnchorName)) {
      //   QString   anchorName () const  TODO
   }
   if (format.hasProperty(QTextFormat::ForegroundBrush)) {
      QBrush brush = format.foreground();
      writer.writeAttribute(foNS, QString::fromLatin1("color"), brush.color().name());
   }
   if (format.hasProperty(QTextFormat::BackgroundBrush)) {
      QBrush brush = format.background();
      writer.writeAttribute(foNS, QString::fromLatin1("background-color"), brush.color().name());
   }

   writer.writeEndElement(); // style
}

void QTextOdfWriter::writeListFormat(QXmlStreamWriter &writer, QTextListFormat format, int formatIndex) const
{
   writer.writeStartElement(textNS, QString::fromLatin1("list-style"));
   writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("L%1").formatArg(formatIndex));

   QTextListFormat::Style style = format.style();
   if (style == QTextListFormat::ListDecimal || style == QTextListFormat::ListLowerAlpha
      || style == QTextListFormat::ListUpperAlpha
      || style == QTextListFormat::ListLowerRoman
      || style == QTextListFormat::ListUpperRoman) {
      writer.writeStartElement(textNS, QString::fromLatin1("list-level-style-number"));
      writer.writeAttribute(styleNS, QString::fromLatin1("num-format"), bulletChar(style));

      if (format.hasProperty(QTextFormat::ListNumberSuffix)) {
         writer.writeAttribute(styleNS, QString::fromLatin1("num-suffix"), format.numberSuffix());
      } else {
         writer.writeAttribute(styleNS, QString::fromLatin1("num-suffix"), QString::fromLatin1("."));
      }

      if (format.hasProperty(QTextFormat::ListNumberPrefix)) {
         writer.writeAttribute(styleNS, QString::fromLatin1("num-prefix"), format.numberPrefix());
      }

   } else {
      writer.writeStartElement(textNS, QString::fromLatin1("list-level-style-bullet"));
      writer.writeAttribute(textNS, QString::fromLatin1("bullet-char"), bulletChar(style));
   }

   writer.writeAttribute(textNS, QString::fromLatin1("level"), QString::number(format.indent()));
   writer.writeEmptyElement(styleNS, QString::fromLatin1("list-level-properties"));
   writer.writeAttribute(foNS, QString::fromLatin1("text-align"), QString::fromLatin1("start"));
   QString spacing = QString::fromLatin1("%1mm").formatArg(format.indent() * 8);
   writer.writeAttribute(textNS, QString::fromLatin1("space-before"), spacing);
   //writer.writeAttribute(textNS, QString::fromLatin1("min-label-width"), spacing);

   writer.writeEndElement(); // list-level-style-*
   writer.writeEndElement(); // list-style
}

void QTextOdfWriter::writeFrameFormat(QXmlStreamWriter &writer, QTextFrameFormat format, int formatIndex) const
{
   writer.writeStartElement(styleNS, QString::fromLatin1("style"));
   writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("s%1").formatArg(formatIndex));
   writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("section"));
   writer.writeEmptyElement(styleNS, QString::fromLatin1("section-properties"));

   if (format.hasProperty(QTextFormat::FrameTopMargin)) {
      writer.writeAttribute(foNS, QString::fromLatin1("margin-top"), pixelToPoint(qMax(qreal(0.), format.topMargin())) );
   }

   if (format.hasProperty(QTextFormat::FrameBottomMargin)) {
      writer.writeAttribute(foNS, QString::fromLatin1("margin-bottom"), pixelToPoint(qMax(qreal(0.),
               format.bottomMargin())) );
   }

   if (format.hasProperty(QTextFormat::FrameLeftMargin)) {
      writer.writeAttribute(foNS, QString::fromLatin1("margin-left"), pixelToPoint(qMax(qreal(0.), format.leftMargin())) );
   }

   if (format.hasProperty(QTextFormat::FrameRightMargin)) {
      writer.writeAttribute(foNS, QString::fromLatin1("margin-right"), pixelToPoint(qMax(qreal(0.), format.rightMargin())) );
   }

   writer.writeEndElement(); // style

   // TODO consider putting the following properties in a qt-namespace.
   // Position   position () const
   // qreal   border () const
   // QBrush   borderBrush () const
   // BorderStyle   borderStyle () const
   // qreal   padding () const
   // QTextLength   width () const
   // QTextLength   height () const
   // PageBreakFlags   pageBreakPolicy () const
}

void QTextOdfWriter::writeTableCellFormat(QXmlStreamWriter &writer, QTextTableCellFormat format, int formatIndex) const
{
   writer.writeStartElement(styleNS, QString::fromLatin1("style"));
   writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("T%1").formatArg(formatIndex));
   writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("table"));
   writer.writeEmptyElement(styleNS, QString::fromLatin1("table-properties"));


   qreal padding = format.topPadding();
   if (padding > 0 && padding == format.bottomPadding()
      && padding == format.leftPadding() && padding == format.rightPadding()) {
      writer.writeAttribute(foNS, QString::fromLatin1("padding"), pixelToPoint(padding));
   } else {
      if (padding > 0) {
         writer.writeAttribute(foNS, QString::fromLatin1("padding-top"), pixelToPoint(padding));
      }
      if (format.bottomPadding() > 0) {
         writer.writeAttribute(foNS, QString::fromLatin1("padding-bottom"), pixelToPoint(format.bottomPadding()));
      }
      if (format.leftPadding() > 0) {
         writer.writeAttribute(foNS, QString::fromLatin1("padding-left"), pixelToPoint(format.leftPadding()));
      }
      if (format.rightPadding() > 0) {
         writer.writeAttribute(foNS, QString::fromLatin1("padding-right"), pixelToPoint(format.rightPadding()));
      }
   }

   if (format.hasProperty(QTextFormat::TextVerticalAlignment)) {
      QString pos;

      switch (format.verticalAlignment()) {
         case QTextCharFormat::AlignMiddle:
            pos = QString::fromLatin1("middle");
            break;

         case QTextCharFormat::AlignTop:
            pos = QString::fromLatin1("top");
            break;

         case QTextCharFormat::AlignBottom:
            pos = QString::fromLatin1("bottom");
            break;

         default:
            pos = QString::fromLatin1("automatic");
            break;
      }
      writer.writeAttribute(styleNS, QString::fromLatin1("vertical-align"), pos);
   }

   // TODO
   // ODF just search for style-table-cell-properties-attlist)
   // QTextFormat::BackgroundImageUrl
   // format.background
   // QTextFormat::FrameBorder

   writer.writeEndElement(); // style
}

QTextOdfWriter::QTextOdfWriter(const QTextDocument &document, QIODevice *device)
   : officeNS("urn:oasis:names:tc:opendocument:xmlns:office:1.0"),
     textNS("urn:oasis:names:tc:opendocument:xmlns:text:1.0"),
     styleNS("urn:oasis:names:tc:opendocument:xmlns:style:1.0"),
     foNS("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"),
     tableNS("urn:oasis:names:tc:opendocument:xmlns:table:1.0"),
     drawNS("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"),
     xlinkNS("http://www.w3.org/1999/xlink"),
     svgNS("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"),
     m_document(&document), m_device(device), m_strategy(nullptr), m_codec(nullptr), m_createArchive(true)
{
}

bool QTextOdfWriter::writeAll()
{
   if (m_createArchive) {
      m_strategy = new QZipStreamStrategy(m_device);
   } else {
      m_strategy = new QXmlStreamStrategy(m_device);
   }

   if (! m_device->isWritable() && ! m_device->open(QIODevice::WriteOnly)) {
      qWarning() << "QTextOdfWriter::writeAll() Device can not be opened for writing";
      return false;
   }

   QXmlStreamWriter writer(m_strategy->contentStream);

#ifndef QT_NO_TEXTCODEC
   if (m_codec) {
      writer.setCodec(m_codec);
   }
#endif

   // prettyfy
   writer.setAutoFormatting(true);
   writer.setAutoFormattingIndent(2);

   writer.writeNamespace(officeNS, "office");
   writer.writeNamespace(textNS,   "text");
   writer.writeNamespace(styleNS,  "style");
   writer.writeNamespace(foNS,     "fo");
   writer.writeNamespace(tableNS,  "table");
   writer.writeNamespace(drawNS,   "draw");
   writer.writeNamespace(xlinkNS,  "xlink");
   writer.writeNamespace(svgNS,    "svg");
   writer.writeStartDocument();
   writer.writeStartElement(officeNS, "document-content");
   writer.writeAttribute(officeNS,    "version", "1.2");

   // add fragments. (for character formats)
   QTextDocumentPrivate::FragmentIterator fragIt = m_document->docHandle()->begin();

   QSet<int> formats;
   while (fragIt != m_document->docHandle()->end()) {
      const QTextFragmentData *const frag = fragIt.value();
      formats << frag->format;
      ++fragIt;
   }

   // add blocks (for blockFormats)
   QTextDocumentPrivate::BlockMap &blocks = m_document->docHandle()->blockMap();
   QTextDocumentPrivate::BlockMap::iterator blockIt = blocks.begin();
   while (blockIt != blocks.end()) {
      const QTextBlockData *const block = blockIt.value();
      formats << block->format;
      ++blockIt;
   }

   // add objects for lists, frames and tables
   QVector<QTextFormat> allFormats = m_document->allFormats();
   QList<int> copy = formats.toList();
   for (QList<int>::iterator iter = copy.begin(); iter != copy.end(); ++iter) {
      QTextObject *object = m_document->objectForFormat(allFormats[*iter]);
      if (object) {
         formats << object->formatIndex();
      }
   }

   writeFormats(writer, formats);

   writer.writeStartElement(officeNS, QString::fromLatin1("body"));
   writer.writeStartElement(officeNS, QString::fromLatin1("text"));
   QTextFrame *rootFrame = m_document->rootFrame();
   writeFrame(writer, rootFrame);
   writer.writeEndElement(); // text
   writer.writeEndElement(); // body
   writer.writeEndElement(); // document-content
   writer.writeEndDocument();
   delete m_strategy;
   m_strategy = nullptr;

   return true;
}

#endif // QT_NO_TEXTODFWRITER
