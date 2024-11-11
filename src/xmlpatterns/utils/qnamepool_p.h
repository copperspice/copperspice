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

#ifndef QNamePool_P_H
#define QNamePool_P_H

#include <qhash.h>
#include <qreadlocker.h>
#include <qreadwritelock.h>
#include <qshareddata.h>
#include <qstring.h>
#include <qvector.h>
#include <qxmlname.h>

#include <qprimitives_p.h>

namespace QPatternist {

class NamePool : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<NamePool> Ptr;

   NamePool();

   QXmlName allocateBinding(const QString &prefix, const QString &uri);
   QXmlName allocateQName(const QString &uri, const QString &localName, const QString &prefix = QString());

   QXmlName allocateQName(const QXmlName::NamespaceCode uri, const QString &ln) {
      // do not lock here, do this in allocateLocalName()
      return QXmlName(uri, allocateLocalName(ln));
   }

   const QString &stringForLocalName(const QXmlName::LocalNameCode code) const {
      const QReadLocker l(&lock);
      return m_localNames.at(code);
   }

   const QString &stringForPrefix(const QXmlName::PrefixCode code) const {
      const QReadLocker l(&lock);
      return m_prefixes.at(code);
   }

   const QString &stringForNamespace(const QXmlName::NamespaceCode code) const {
      const QReadLocker l(&lock);
      return m_namespaces.at(code);
   }

   QString displayName(const QXmlName qName) const;

   QString toLexical(const QXmlName qName) const {
      const QReadLocker l(&lock);
      Q_ASSERT_X(!qName.isNull(), "", "It makes no sense to call toLexical() on a null name.");

      if (qName.hasPrefix()) {
         const QString &p = m_prefixes.at(qName.prefix());
         return p + QLatin1Char(':') + m_localNames.at(qName.localName());
      } else {
         return m_localNames.at(qName.localName());
      }
   }

   QXmlName::NamespaceCode allocateNamespace(const QString &uri) {
      const QWriteLocker l(&lock);
      return unlockedAllocateNamespace(uri);
   }

   QXmlName::LocalNameCode allocateLocalName(const QString &ln) {
      const QWriteLocker l(&lock);
      return unlockedAllocateLocalName(ln);
   }

   QXmlName::PrefixCode allocatePrefix(const QString &prefix) {
      const QWriteLocker l(&lock);
      return unlockedAllocatePrefix(prefix);
   }

   QString toClarkName(const QXmlName &name) const;
   QXmlName fromClarkName(const QString &clarkName);

 private:
   static constexpr const int NoSuchValue            = -1;
   static constexpr const int StandardNamespaceCount = 11;
   static constexpr const int StandardPrefixCount    = 9;
   static constexpr const int StandardLocalNameCount = 141;

   enum DefaultCapacities {
      DefaultPrefixCapacity    = 10,
      DefaultURICapacity       = DefaultPrefixCapacity,
      DefaultLocalNameCapacity = 60
   };

   QVector<QString> m_prefixes;
   QVector<QString> m_namespaces;
   QVector<QString> m_localNames;

   QHash<QString, QXmlName::PrefixCode> m_prefixMapping;
   QHash<QString, QXmlName::NamespaceCode> m_namespaceMapping;
   QHash<QString, QXmlName::LocalNameCode> m_localNameMapping;

   QXmlName::NamespaceCode unlockedAllocateNamespace(const QString &uri);
   QXmlName::LocalNameCode unlockedAllocateLocalName(const QString &ln);
   QXmlName::PrefixCode unlockedAllocatePrefix(const QString &prefix);

   NamePool(const NamePool &) = delete;
   NamePool &operator=(const NamePool &) = delete;

   const QString &displayPrefix(const QXmlName::NamespaceCode nc) const;

   mutable QReadWriteLock lock;

   friend class StandardNamespaces;
};

static inline QString formatKeyword(const NamePool::Ptr &np, const QXmlName name)
{
   return QLatin1String("<span class='XQuery-keyword'>")   +
          escape(np->displayName(name))                    +
          QLatin1String("</span>");
}

class StandardNamespaces
{
 public:
   enum ID {
      empty = 0,
      fn,
      local,
      xml,
      xmlns,
      xs,
      xsi,
      xslt,
      UndeclarePrefix,
      StopNamespaceInheritance,
      InternalXSLT
   };
};

class StandardLocalNames
{
 public:
   enum {
      abs,
      adjust_dateTime_to_timezone,
      adjust_date_to_timezone,
      adjust_time_to_timezone,
      all,
      arity,
      avg,
      base,
      base_uri,
      boolean,
      ceiling,
      codepoint_equal,
      codepoints_to_string,
      collection,
      compare,
      concat,
      contains,
      count,
      current,
      current_date,
      current_dateTime,
      current_time,
      data,
      dateTime,
      day_from_date,
      day_from_dateTime,
      days_from_duration,
      deep_equal,
      Default,
      default_collation,
      distinct_values,
      doc,
      doc_available,
      document,
      document_uri,
      element_available,
      empty,
      encode_for_uri,
      ends_with,
      error,
      escape_html_uri,
      exactly_one,
      exists,
      False,
      floor,
      function_available,
      function_name,
      generate_id,
      generic_string_join,
      hours_from_dateTime,
      hours_from_duration,
      hours_from_time,
      id,
      idref,
      implicit_timezone,
      index_of,
      in_scope_prefixes,
      insert_before,
      iri_to_uri,
      is_schema_aware,
      key,
      lang,
      last,
      local_name,
      local_name_from_QName,
      lower_case,
      matches,
      max,
      min,
      minutes_from_dateTime,
      minutes_from_duration,
      minutes_from_time,
      month_from_date,
      month_from_dateTime,
      months_from_duration,
      name,
      namespace_uri,
      namespace_uri_for_prefix,
      namespace_uri_from_QName,
      nilled,
      node_name,
      normalize_space,
      normalize_unicode,
      Not,
      number,
      one_or_more,
      position,
      prefix_from_QName,
      product_name,
      product_version,
      property_name,
      QName,
      remove,
      replace,
      resolve_QName,
      resolve_uri,
      reverse,
      root,
      round,
      round_half_to_even,
      seconds_from_dateTime,
      seconds_from_duration,
      seconds_from_time,
      sourceValue,
      starts_with,
      static_base_uri,
      string,
      string_join,
      string_length,
      string_to_codepoints,
      subsequence,
      substring,
      substring_after,
      substring_before,
      sum,
      supports_backwards_compatibility,
      supports_serialization,
      system_property,
      timezone_from_date,
      timezone_from_dateTime,
      timezone_from_time,
      tokenize,
      trace,
      translate,
      True,
      type_available,
      unordered,
      unparsed_entity_public_id,
      unparsed_entity_uri,
      unparsed_text,
      unparsed_text_available,
      upper_case,
      vendor,
      vendor_url,
      version,
      xml,
      xmlns,
      year_from_date,
      year_from_dateTime,
      years_from_duration,
      zero_or_one
   };
};

class StandardPrefixes
{
 public:
   enum {
      empty = 0,
      fn,
      local,
      xml,
      xmlns,
      xs,
      xsi,
      ns0,
      StopNamespaceInheritance
   };
};

}

inline QXmlName::LocalNameCode QXmlName::localName() const
{
   return (m_qNameCode & LocalNameMask) >> LocalNameOffset;
}

inline QXmlName::PrefixCode QXmlName::prefix() const
{
   return (m_qNameCode & PrefixMask) >> PrefixOffset;
}

inline bool QXmlName::hasPrefix() const
{
   return prefix() != 0;
}

inline bool QXmlName::hasNamespace() const
{
   return namespaceURI() != 0;
}

inline QXmlName::NamespaceCode QXmlName::namespaceURI() const
{
   return (m_qNameCode & NamespaceMask) >> NamespaceOffset;
}

inline bool QXmlName::isLexicallyEqual(const QXmlName &other) const
{
   return (m_qNameCode & LexicalQNameMask) == (other.m_qNameCode & LexicalQNameMask);
}

inline void QXmlName::setPrefix(const PrefixCode c)
{
   m_qNameCode |= (c << PrefixOffset);
}

inline void QXmlName::setNamespaceURI(const NamespaceCode c)
{
   m_qNameCode |= (c << NamespaceOffset);
}

inline void QXmlName::setLocalName(const LocalNameCode c)
{
   m_qNameCode |= (c << LocalNameOffset);
}

inline QXmlName::Code QXmlName::code() const
{
   return m_qNameCode;
}

inline QXmlName::QXmlName(const NamespaceCode uri, const LocalNameCode ln, const PrefixCode p)
   : m_qNameCode((uri << NamespaceOffset) + (ln << LocalNameOffset)  + (p << PrefixOffset))
{
   Q_ASSERT_X(p <= MaximumPrefixes, "",
              csPrintable(QString("NamePool prefix limits: max is %1, therefore %2 exceeds.").formatArg(MaximumPrefixes).formatArg(p)));

   Q_ASSERT_X(ln <= MaximumLocalNames, "",
              csPrintable(QString("NamePool local name limits: max is %1, therefore %2 exceeds.").formatArg(MaximumLocalNames).formatArg(ln)));

   Q_ASSERT_X(uri <= MaximumNamespaces, "",
              csPrintable(QString("NamePool namespace limits: max is %1, therefore %2 exceeds.").formatArg(MaximumNamespaces).formatArg(uri)));
}

#endif
