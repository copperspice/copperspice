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

#include "qdesigner_utils_p.h"
#include "qdesigner_propertycommand_p.h"
#include "abstractformbuilder.h"
#include "formwindowbase_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerResourceBrowserInterface>
#include <QtDesigner/QDesignerLanguageExtension>
#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QtDesigner/QExtensionManager>

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDebug>
#include <QtCore/QQueue>
#include <QtCore/QSharedData>

#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QComboBox>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal
{
    QDESIGNER_SHARED_EXPORT void designerWarning(const QString &message)
    {
        qWarning("Designer: %s", qPrintable(message));
    }

    void reloadTreeItem(DesignerIconCache *iconCache, QTreeWidgetItem *item)
    {
        if (!item)
            return;

        for (int c = 0; c < item->columnCount(); c++) {
            const QVariant v = item->data(c, Qt::DecorationPropertyRole);
            if (v.canConvert<PropertySheetIconValue>())
                item->setIcon(c, iconCache->icon(qvariant_cast<PropertySheetIconValue>(v)));
        }
    }

    void reloadListItem(DesignerIconCache *iconCache, QListWidgetItem *item)
    {
        if (!item)
            return;

        const QVariant v = item->data(Qt::DecorationPropertyRole);
        if (v.canConvert<PropertySheetIconValue>())
            item->setIcon(iconCache->icon(qvariant_cast<PropertySheetIconValue>(v)));
    }

    void reloadTableItem(DesignerIconCache *iconCache, QTableWidgetItem *item)
    {
        if (!item)
            return;

        const QVariant v = item->data(Qt::DecorationPropertyRole);
        if (v.canConvert<PropertySheetIconValue>())
            item->setIcon(iconCache->icon(qvariant_cast<PropertySheetIconValue>(v)));
    }

    void reloadIconResources(DesignerIconCache *iconCache, QObject *object)
    {
        if (QListWidget *listWidget = qobject_cast<QListWidget *>(object)) {
            for (int i = 0; i < listWidget->count(); i++)
                reloadListItem(iconCache, listWidget->item(i));
        } else if (QComboBox *comboBox = qobject_cast<QComboBox *>(object)) {
            for (int i = 0; i < comboBox->count(); i++) {
                const QVariant v = comboBox->itemData(i, Qt::DecorationPropertyRole);
                if (v.canConvert<PropertySheetIconValue>()) {
                    QIcon icon = iconCache->icon(qvariant_cast<PropertySheetIconValue>(v));
                    comboBox->setItemIcon(i, icon);
                    comboBox->setItemData(i, icon);
                }
            }
        } else if (QTreeWidget *treeWidget = qobject_cast<QTreeWidget *>(object)) {
            reloadTreeItem(iconCache, treeWidget->headerItem());
            QQueue<QTreeWidgetItem *> itemsQueue;
            for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
                itemsQueue.enqueue(treeWidget->topLevelItem(i));
            while (!itemsQueue.isEmpty()) {
                QTreeWidgetItem *item = itemsQueue.dequeue();
                for (int i = 0; i < item->childCount(); i++)
                    itemsQueue.enqueue(item->child(i));
                reloadTreeItem(iconCache, item);
            }
        } else if (QTableWidget *tableWidget = qobject_cast<QTableWidget *>(object)) {
            const int columnCount = tableWidget->columnCount();
            const int rowCount = tableWidget->rowCount();
            for (int c = 0; c < columnCount; c++)
                reloadTableItem(iconCache, tableWidget->horizontalHeaderItem(c));
            for (int r = 0; r < rowCount; r++)
                reloadTableItem(iconCache, tableWidget->verticalHeaderItem(r));
            for (int c = 0; c < columnCount; c++)
                for (int r = 0; r < rowCount; r++)
                    reloadTableItem(iconCache, tableWidget->item(r, c));
        }
    }

    // ------------- DesignerMetaEnum
    DesignerMetaEnum::DesignerMetaEnum(const QString &name, const QString &scope, const QString &separator) :
        MetaEnum<int>(name, scope, separator)
    {
    }


    QString DesignerMetaEnum::toString(int value, SerializationMode sm, bool *ok) const
    {
        // find value
        bool valueOk;
        const QString item = valueToKey(value, &valueOk);
        if (ok)
            *ok = valueOk;

        if (!valueOk || sm == NameOnly)
            return item;

        QString qualifiedItem;
        appendQualifiedName(item,  qualifiedItem);
        return qualifiedItem;
    }

    QString DesignerMetaEnum::messageToStringFailed(int value) const
    {
        return QCoreApplication::translate("DesignerMetaEnum", "%1 is not a valid enumeration value of '%2'.").arg(value).arg(name());
    }

    QString DesignerMetaEnum::messageParseFailed(const QString &s) const
    {
        return QCoreApplication::translate("DesignerMetaEnum", "'%1' could not be converted to an enumeration value of type '%2'.").arg(s).arg(name());
    }
    // -------------- DesignerMetaFlags
    DesignerMetaFlags::DesignerMetaFlags(const QString &name, const QString &scope, const QString &separator) :
       MetaEnum<uint>(name, scope, separator)
    {
    }

    QStringList DesignerMetaFlags::flags(int ivalue) const
    {
        typedef MetaEnum<uint>::KeyToValueMap::const_iterator KeyToValueMapIterator;
        QStringList rc;
        const uint v = static_cast<uint>(ivalue);
        const KeyToValueMapIterator cend = keyToValueMap().constEnd();
        for (KeyToValueMapIterator it = keyToValueMap().constBegin();it != cend; ++it )  {
            const uint itemValue = it.value();
            // Check for equality first as flag values can be 0 or -1, too. Takes preference over a bitwise flag
            if (v == itemValue) {
                rc.clear();
                rc.push_back(it.key());
                return rc;
            }
            // Do not add 0-flags (None-flags)
            if (itemValue)
                if ((v & itemValue) == itemValue)
                    rc.push_back(it.key());
        }
        return rc;
    }


    QString DesignerMetaFlags::toString(int value, SerializationMode sm) const
    {
        const QStringList flagIds = flags(value);
        if (flagIds.empty())
            return QString();

        const QChar delimiter = QLatin1Char('|');
        QString rc;
        const QStringList::const_iterator cend = flagIds.constEnd();
        for (QStringList::const_iterator it = flagIds.constBegin(); it != cend; ++it) {
            if (!rc.isEmpty())
                rc += delimiter ;
            if (sm == FullyQualified)
                appendQualifiedName(*it, rc);
            else
                rc += *it;
        }
        return rc;
    }


    int DesignerMetaFlags::parseFlags(const QString &s, bool *ok) const
    {
        if (s.isEmpty()) {
            if (ok)
                *ok = true;
            return 0;
        }
        uint flags = 0;
        bool valueOk = true;
        QStringList keys = s.split(QString(QLatin1Char('|')));
        const QStringList::iterator cend = keys.end();
        for (QStringList::iterator it = keys.begin(); it != cend; ++it) {
            const uint flagValue = keyToValue(*it, &valueOk);
            if (!valueOk) {
                flags = 0;
                break;
            }
            flags |= flagValue;
        }
        if (ok)
            *ok = valueOk;
        return static_cast<int>(flags);
    }

    QString DesignerMetaFlags::messageParseFailed(const QString &s) const
    {
        return QCoreApplication::translate("DesignerMetaFlags", "'%1' could not be converted to a flag value of type '%2'.").arg(s).arg(name());
    }

    // ---------- PropertySheetEnumValue

    PropertySheetEnumValue::PropertySheetEnumValue(int v, const DesignerMetaEnum &me) :
       value(v),
       metaEnum(me)
    {
    }
    PropertySheetEnumValue::PropertySheetEnumValue() :
       value(0)
    {
    }

    // ---------------- PropertySheetFlagValue
    PropertySheetFlagValue::PropertySheetFlagValue(int v, const DesignerMetaFlags &mf) :
        value(v),
        metaFlags(mf)
    {
    }

    PropertySheetFlagValue::PropertySheetFlagValue() :
        value(0)
    {
    }

    // ---------------- PropertySheetPixmapValue
    PropertySheetPixmapValue::PropertySheetPixmapValue(const QString &path) : m_path(path)
    {
    }

    PropertySheetPixmapValue::PropertySheetPixmapValue()
    {
    }

    PropertySheetPixmapValue::PixmapSource PropertySheetPixmapValue::getPixmapSource(QDesignerFormEditorInterface *core, const QString & path)
    {
        if (const QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core))
            return lang->isLanguageResource(path) ?  LanguageResourcePixmap : FilePixmap;
        return path.startsWith(QLatin1Char(':')) ? ResourcePixmap : FilePixmap;
    }

    int PropertySheetPixmapValue::compare(const PropertySheetPixmapValue &other) const
    {
        return m_path.compare(other.m_path);
    }

    QString PropertySheetPixmapValue::path() const
    {
        return m_path;
    }

    void PropertySheetPixmapValue::setPath(const QString &path)
    {
        if (m_path == path)
            return;
        m_path = path;
    }

    // ---------- PropertySheetIconValue

    class PropertySheetIconValueData : public QSharedData {
    public:
        PropertySheetIconValue::ModeStateToPixmapMap m_paths;
        QString m_theme;
    };

    PropertySheetIconValue::PropertySheetIconValue(const PropertySheetPixmapValue &pixmap) :
        m_data(new PropertySheetIconValueData)
    {
        setPixmap(QIcon::Normal, QIcon::Off, pixmap);
    }

    PropertySheetIconValue::PropertySheetIconValue() :
        m_data(new PropertySheetIconValueData)
    {
    }

    PropertySheetIconValue::~PropertySheetIconValue()
    {
    }

    PropertySheetIconValue::PropertySheetIconValue(const PropertySheetIconValue &rhs) :
        m_data(rhs.m_data)
    {
    }

    PropertySheetIconValue &PropertySheetIconValue::operator=(const PropertySheetIconValue &rhs)
    {
        if (this != &rhs)
            m_data.operator=(rhs.m_data);
        return *this;
    }

    bool PropertySheetIconValue::equals(const PropertySheetIconValue &rhs) const
    {
        return m_data->m_theme == rhs.m_data->m_theme && m_data->m_paths == rhs.m_data->m_paths;
    }

    bool PropertySheetIconValue::operator<(const PropertySheetIconValue &other) const
    {
        if (const int themeCmp = m_data->m_theme.compare(other.m_data->m_theme))
            return themeCmp < 0;
        QMapIterator<ModeStateKey, PropertySheetPixmapValue> itThis(m_data->m_paths);
        QMapIterator<ModeStateKey, PropertySheetPixmapValue> itOther(other.m_data->m_paths);
        while (itThis.hasNext() && itOther.hasNext()) {
            const ModeStateKey thisPair = itThis.next().key();
            const ModeStateKey otherPair = itOther.next().key();
            if (thisPair < otherPair)
                return true;
            else if (otherPair < thisPair)
                return false;
            const int crc = itThis.value().compare(itOther.value());
            if (crc < 0)
                return true;
            if (crc > 0)
                return false;
        }
        if (itOther.hasNext())
            return true;
        return false;
    }

    bool PropertySheetIconValue::isEmpty() const
    {
        return m_data->m_theme.isEmpty() && m_data->m_paths.isEmpty();
    }

    QString PropertySheetIconValue::theme() const
    {
        return m_data->m_theme;
    }

    void PropertySheetIconValue::setTheme(const QString &t)
    {
        m_data->m_theme = t;
    }

    PropertySheetPixmapValue PropertySheetIconValue::pixmap(QIcon::Mode mode, QIcon::State state) const
    {
        const ModeStateKey pair = qMakePair(mode, state);
        return m_data->m_paths.value(pair);
    }

    void PropertySheetIconValue::setPixmap(QIcon::Mode mode, QIcon::State state, const PropertySheetPixmapValue &pixmap)
    {
        const ModeStateKey pair = qMakePair(mode, state);
        if (pixmap.path().isEmpty())
            m_data->m_paths.remove(pair);
        else
            m_data->m_paths.insert(pair, pixmap);
    }

    QPixmap DesignerPixmapCache::pixmap(const PropertySheetPixmapValue &value) const
    {
        QMap<PropertySheetPixmapValue, QPixmap>::const_iterator it = m_cache.constFind(value);
        if (it != m_cache.constEnd())
            return it.value();

        QPixmap pix = QPixmap(value.path());
        m_cache.insert(value, pix);
        return pix;
    }

    void DesignerPixmapCache::clear()
    {
        m_cache.clear();
    }

    DesignerPixmapCache::DesignerPixmapCache(QObject *parent)
        : QObject(parent)
    {
    }

    QIcon DesignerIconCache::icon(const PropertySheetIconValue &value) const
    {
        typedef PropertySheetIconValue::ModeStateToPixmapMap::const_iterator ModeStateToPixmapMapConstIt;

        QMap<PropertySheetIconValue, QIcon>::const_iterator it = m_cache.constFind(value);
        if (it != m_cache.constEnd())
            return it.value();

        // Match on the theme first if it is available.
        if (!value.theme().isEmpty()) {
            const QString theme = value.theme();
            if (QIcon::hasThemeIcon(theme)) {
                const QIcon themeIcon = QIcon::fromTheme(theme);
                m_cache.insert(value, themeIcon);
                return themeIcon;
            }
        }

        QIcon icon;
        const PropertySheetIconValue::ModeStateToPixmapMap &paths = value.paths();
        const ModeStateToPixmapMapConstIt cend = paths.constEnd();
        for (ModeStateToPixmapMapConstIt it = paths.constBegin(); it != cend; ++it) {
            const QPair<QIcon::Mode, QIcon::State> pair = it.key();
            icon.addFile(it.value().path(), QSize(), pair.first, pair.second);
        }
        m_cache.insert(value, icon);
        return icon;
    }

    void DesignerIconCache::clear()
    {
        m_cache.clear();
    }

    DesignerIconCache::DesignerIconCache(DesignerPixmapCache *pixmapCache, QObject *parent)
        : QObject(parent),
        m_pixmapCache(pixmapCache)
    {

    }

    PropertySheetStringValue::PropertySheetStringValue(const QString &value,
                    bool translatable, const QString &disambiguation, const QString &comment)
        : m_value(value), m_translatable(translatable), m_disambiguation(disambiguation), m_comment(comment)
    {  }

    QString PropertySheetStringValue::value() const
    {
        return m_value;
    }

    void PropertySheetStringValue::setValue(const QString &value)
    {
        m_value = value;
    }

    bool PropertySheetStringValue::translatable() const
    {
        return m_translatable;
    }

    void PropertySheetStringValue::setTranslatable(bool translatable)
    {
        m_translatable = translatable;
    }

    QString PropertySheetStringValue::disambiguation() const
    {
        return m_disambiguation;
    }

    void PropertySheetStringValue::setDisambiguation(const QString &disambiguation)
    {
        m_disambiguation = disambiguation;
    }

    QString PropertySheetStringValue::comment() const
    {
        return m_comment;
    }

    void PropertySheetStringValue::setComment(const QString &comment)
    {
        m_comment = comment;
    }

    bool PropertySheetStringValue::equals(const PropertySheetStringValue &rhs) const
    {
        return (m_value == rhs.m_value) && (m_translatable == rhs.m_translatable)
            && (m_disambiguation == rhs.m_disambiguation) && (m_comment == rhs.m_comment);
    }

    PropertySheetKeySequenceValue::PropertySheetKeySequenceValue(const QKeySequence &value,
                    bool translatable, const QString &disambiguation, const QString &comment)
        : m_value(value),
          m_standardKey(QKeySequence::UnknownKey),
          m_translatable(translatable),
          m_disambiguation(disambiguation),
          m_comment(comment)
    {  }

    PropertySheetKeySequenceValue::PropertySheetKeySequenceValue(const QKeySequence::StandardKey &standardKey,
                    bool translatable, const QString &disambiguation, const QString &comment)
        : m_value(QKeySequence(standardKey)),
          m_standardKey(standardKey),
          m_translatable(translatable),
          m_disambiguation(disambiguation),
          m_comment(comment)
    {  }

    QKeySequence PropertySheetKeySequenceValue::value() const
    {
        return m_value;
    }

    void PropertySheetKeySequenceValue::setValue(const QKeySequence &value)
    {
        m_value = value;
        m_standardKey = QKeySequence::UnknownKey;
    }

    QKeySequence::StandardKey PropertySheetKeySequenceValue::standardKey() const
    {
        return m_standardKey;
    }

    void PropertySheetKeySequenceValue::setStandardKey(const QKeySequence::StandardKey &standardKey)
    {
        m_value = QKeySequence(standardKey);
        m_standardKey = standardKey;
    }

    bool PropertySheetKeySequenceValue::isStandardKey() const
    {
        return m_standardKey != QKeySequence::UnknownKey;
    }

    QString PropertySheetKeySequenceValue::comment() const
    {
        return m_comment;
    }

    void PropertySheetKeySequenceValue::setComment(const QString &comment)
    {
        m_comment = comment;
    }

    QString PropertySheetKeySequenceValue::disambiguation() const
    {
        return m_disambiguation;
    }

    void PropertySheetKeySequenceValue::setDisambiguation(const QString &disambiguation)
    {
        m_disambiguation = disambiguation;
    }

    bool PropertySheetKeySequenceValue::translatable() const
    {
        return m_translatable;
    }

    void PropertySheetKeySequenceValue::setTranslatable(bool translatable)
    {
        m_translatable = translatable;
    }

    bool PropertySheetKeySequenceValue::equals(const PropertySheetKeySequenceValue &rhs) const
    {
        return (m_value == rhs.m_value) && (m_standardKey == rhs.m_standardKey)
            && (m_translatable == rhs.m_translatable) && (m_disambiguation == rhs.m_disambiguation) && (m_comment == rhs.m_comment);
    }


    /* IconSubPropertyMask: Assign each icon sub-property (pixmaps for the
     * various states/modes and the theme) a flag bit (see QFont) so that they
     * can be handled individually when assigning property values to
     * multiselections in the set-property-commands (that is, do not clobber
     * other subproperties when assigning just one).
     * Provide back-and-forth mapping functions for the icon states. */

    enum IconSubPropertyMask {
        NormalOffIconMask   = 0x01,
        NormalOnIconMask    = 0x02,
        DisabledOffIconMask = 0x04,
        DisabledOnIconMask  = 0x08,
        ActiveOffIconMask   = 0x10,
        ActiveOnIconMask    = 0x20,
        SelectedOffIconMask = 0x40,
        SelectedOnIconMask  = 0x80,
        ThemeIconMask       = 0x10000
    };

    static inline uint iconStateToSubPropertyFlag(QIcon::Mode mode, QIcon::State state)
    {
        switch (mode) {
        case QIcon::Disabled:
            return state == QIcon::On ? DisabledOnIconMask : DisabledOffIconMask;
        case QIcon::Active:
            return state == QIcon::On ?   ActiveOnIconMask :   ActiveOffIconMask;
        case QIcon::Selected:
            return state == QIcon::On ? SelectedOnIconMask : SelectedOffIconMask;
        case QIcon::Normal:
            break;
        }
        return     state == QIcon::On ?   NormalOnIconMask :   NormalOffIconMask;
    }

    static inline QPair<QIcon::Mode, QIcon::State> subPropertyFlagToIconModeState(unsigned flag)
    {
        switch (flag) {
        case NormalOnIconMask:
            return qMakePair(QIcon::Normal,   QIcon::On);
        case DisabledOffIconMask:
            return qMakePair(QIcon::Disabled, QIcon::Off);
        case DisabledOnIconMask:
            return qMakePair(QIcon::Disabled, QIcon::On);
        case ActiveOffIconMask:
            return qMakePair(QIcon::Active,   QIcon::Off);
        case ActiveOnIconMask:
            return qMakePair(QIcon::Active,   QIcon::On);
        case SelectedOffIconMask:
            return qMakePair(QIcon::Selected, QIcon::Off);
        case SelectedOnIconMask:
            return qMakePair(QIcon::Selected, QIcon::On);
        case NormalOffIconMask:
        default:
            break;
        }
        return     qMakePair(QIcon::Normal,   QIcon::Off);
    }

    uint PropertySheetIconValue::mask() const
    {
        typedef ModeStateToPixmapMap::const_iterator ModeStateToPixmapMapConstIt;

        uint flags = 0;
        const ModeStateToPixmapMapConstIt cend = m_data->m_paths.constEnd();
        for (ModeStateToPixmapMapConstIt it = m_data->m_paths.constBegin(); it != cend; ++it)
            flags |= iconStateToSubPropertyFlag(it.key().first, it.key().second);
        if (!m_data->m_theme.isEmpty())
            flags |= ThemeIconMask;
        return flags;
    }

    uint PropertySheetIconValue::compare(const PropertySheetIconValue &other) const
    {
        uint diffMask = mask() | other.mask();
        for (int i = 0; i < 8; i++) {
            const uint flag = 1 << i;
            if (diffMask & flag) { // if state is set in both icons, compare the values
                const QPair<QIcon::Mode, QIcon::State> state = subPropertyFlagToIconModeState(flag);
                if (pixmap(state.first, state.second) == other.pixmap(state.first, state.second))
                    diffMask &= ~flag;
            }
        }
        if ((diffMask & ThemeIconMask) && theme() == other.theme())
            diffMask &= ~ThemeIconMask;
        return diffMask;
    }

    PropertySheetIconValue PropertySheetIconValue::themed() const
    {
        PropertySheetIconValue rc(*this);
        rc.m_data->m_paths.clear();
        return rc;
    }

    PropertySheetIconValue PropertySheetIconValue::unthemed() const
    {
        PropertySheetIconValue rc(*this);
        rc.m_data->m_theme.clear();
        return rc;
    }

    void PropertySheetIconValue::assign(const PropertySheetIconValue &other, uint mask)
    {
        for (int i = 0; i < 8; i++) {
            uint flag = 1 << i;
            if (mask & flag) {
                const ModeStateKey state = subPropertyFlagToIconModeState(flag);
                setPixmap(state.first, state.second, other.pixmap(state.first, state.second));
            }
        }
        if (mask & ThemeIconMask)
            setTheme(other.theme());
    }

    const PropertySheetIconValue::ModeStateToPixmapMap &PropertySheetIconValue::paths() const
    {
        return m_data->m_paths;
    }

    QDESIGNER_SHARED_EXPORT QDebug operator<<(QDebug d, const PropertySheetIconValue &p)
    {
        typedef PropertySheetIconValue::ModeStateToPixmapMap::const_iterator ModeStateToPixmapMapConstIt;

        QDebug nospace = d.nospace();
        nospace << "PropertySheetIconValue theme='" << p.theme() << "' ";

        const PropertySheetIconValue::ModeStateToPixmapMap &paths = p.paths();
        const ModeStateToPixmapMapConstIt cend = paths.constEnd();
        for (ModeStateToPixmapMapConstIt it = paths.constBegin(); it != cend; ++it)
            nospace << " mode=" << it.key().first << ",state=" << it.key().second
                       << ",'" << it.value().path() << '\'';
        nospace << " mask=0x" << QString::number(p.mask(), 16);
        return d;
    }

    QDESIGNER_SHARED_EXPORT QDesignerFormWindowCommand *createTextPropertyCommand(const QString &propertyName, const QString &text, QObject *object, QDesignerFormWindowInterface *fw)
    {
        if (text.isEmpty()) {
            ResetPropertyCommand *cmd = new ResetPropertyCommand(fw);
            cmd->init(object, propertyName);
            return cmd;
        }
        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(object, propertyName, text);
        return cmd;
    }

    QDESIGNER_SHARED_EXPORT QAction *preferredEditAction(QDesignerFormEditorInterface *core, QWidget *managedWidget)
    {
        QAction *action = 0;
        if (const QDesignerTaskMenuExtension *taskMenu = qt_extension<QDesignerTaskMenuExtension*>(core->extensionManager(), managedWidget)) {
            action = taskMenu->preferredEditAction();
            if (!action) {
                const QList<QAction *> actions = taskMenu->taskActions();
                if (!actions.isEmpty())
                    action = actions.first();
            }
        }
        if (!action) {
            if (const QDesignerTaskMenuExtension *taskMenu = qobject_cast<QDesignerTaskMenuExtension *>(
                        core->extensionManager()->extension(managedWidget, QLatin1String("QDesignerInternalTaskMenuExtension")))) {
                action = taskMenu->preferredEditAction();
                if (!action) {
                    const QList<QAction *> actions = taskMenu->taskActions();
                    if (!actions.isEmpty())
                        action = actions.first();
                }
            }
        }
        return action;
    }

    QDESIGNER_SHARED_EXPORT bool runUIC(const QString &fileName, UIC_Mode mode, QByteArray& ba, QString &errorMessage)
    {
        QStringList argv;
        QString binary = QLibraryInfo::location(QLibraryInfo::BinariesPath);
        binary += QDir::separator();
        switch (mode) {
        case UIC_GenerateCode:
            binary += QLatin1String("uic");
            break;
        case UIC_ConvertV3:
            binary += QLatin1String("uic3");
            argv += QLatin1String("-convert");
            break;
        }
        argv += fileName;
        QProcess uic;
        uic.start(binary, argv);
        if (!uic.waitForStarted()) {
            errorMessage = QApplication::translate("Designer", "Unable to launch %1.").arg(binary);
            return false;
        }
        if (!uic.waitForFinished()) {
            errorMessage = QApplication::translate("Designer", "%1 timed out.").arg(binary);
            return false;
        }
        if (uic.exitCode()) {
            errorMessage =  QString::fromAscii(uic.readAllStandardError());
            return false;
        }
        ba = uic.readAllStandardOutput();
        return true;
    }

    QDESIGNER_SHARED_EXPORT QString qtify(const QString &name)
    {
        QString qname = name;

        Q_ASSERT(qname.isEmpty() == false);


        if (qname.count() > 1 && qname.at(1).isUpper()) {
            const QChar first = qname.at(0);
            if (first == QLatin1Char('Q') || first == QLatin1Char('K'))
                qname.remove(0, 1);
        }

        const int len = qname.count();
        for (int i = 0; i < len && qname.at(i).isUpper(); i++)
            qname[i] = qname.at(i).toLower();

        return qname;
    }

    // --------------- UpdateBlocker
    UpdateBlocker::UpdateBlocker(QWidget *w) :
        m_widget(w),
        m_enabled(w->updatesEnabled() && w->isVisible())
    {
        if (m_enabled)
            m_widget->setUpdatesEnabled(false);
    }

    UpdateBlocker::~UpdateBlocker()
    {
        if (m_enabled)
            m_widget->setUpdatesEnabled(true);
    }

} // namespace qdesigner_internal

QT_END_NAMESPACE
