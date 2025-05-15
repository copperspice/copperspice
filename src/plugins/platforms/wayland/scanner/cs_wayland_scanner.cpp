/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qcoreapplication.h>
#include <qfile.h>
#include <qxmlstreamreader.h>

#include <unistd.h>

enum Option {
   ClientHeader,
   ServerHeader,
   ClientCode,
   ServerCode
} option;

bool isServerSide()
{
   return option == ServerHeader || option == ServerCode;
}

bool parseOption(const QString &str, Option *option)
{
   if (str == "client-header") {
      *option = ClientHeader;

   } else if (str == "server-header") {
      *option = ServerHeader;

   } else if (str == "client-code") {
      *option = ClientCode;

   } else if (str == "server-code") {
      *option = ServerCode;

   } else {
      return false;
   }

   return true;
}

struct WaylandEnumEntry {
   QByteArray name;
   QByteArray value;
   QByteArray summary;
};

struct WaylandEnum {
   QByteArray name;

   QList<WaylandEnumEntry> entries;
};

struct WaylandArgument {
   bool allowNull;

   QByteArray name;
   QByteArray type;
   QByteArray interface;
   QByteArray summary;
};

struct WaylandEvent {
   bool request;

   QByteArray name;
   QByteArray type;

   QList<WaylandArgument> arguments;
};

struct WaylandInterface {
   QByteArray name;
   int version;

   QList<WaylandEnum>  enums;
   QList<WaylandEvent> events;
   QList<WaylandEvent> requests;
};

QByteArray byteArrayValue(const QXmlStreamReader &xml, const QString &name)
{
   if (xml.attributes().hasAttribute(name)) {
      return xml.attributes().value(name).toUtf8();
   }

   return QByteArray();
}

int intValue(const QXmlStreamReader &xml, const QString &name, int defaultValue = 0)
{
   bool ok;
   int result = byteArrayValue(xml, name).toInt(&ok);

   return ok ? result : defaultValue;
}

bool boolValue(const QXmlStreamReader &xml, const QString &name)
{
   return byteArrayValue(xml, name) == "true";
}

WaylandEvent readEvent(QXmlStreamReader &xml, bool request)
{
   WaylandEvent event;
   event.request = request;
   event.name = byteArrayValue(xml, "name");
   event.type = byteArrayValue(xml, "type");

   while (xml.readNextStartElement()) {
      if (xml.name() == "arg") {
         WaylandArgument argument;

         argument.name      = byteArrayValue(xml, "name");
         argument.type      = byteArrayValue(xml, "type");
         argument.interface = byteArrayValue(xml, "interface");
         argument.summary   = byteArrayValue(xml, "summary");
         argument.allowNull = boolValue(xml, "allowNull");

         event.arguments.append(std::move(argument));
      }

      xml.skipCurrentElement();
   }

   return event;
}

WaylandEnum readEnum(QXmlStreamReader &xml)
{
   WaylandEnum result;
   result.name = byteArrayValue(xml, "name");

   while (xml.readNextStartElement()) {
      if (xml.name() == "entry") {
         WaylandEnumEntry entry;
         entry.name    = byteArrayValue(xml, "name");
         entry.value   = byteArrayValue(xml, "value");
         entry.summary = byteArrayValue(xml, "summary");

         result.entries.append(std::move(entry));
      }

      xml.skipCurrentElement();
   }

   return result;
}

WaylandInterface readInterface(QXmlStreamReader &xml)
{
   WaylandInterface interface;
   interface.name    = byteArrayValue(xml, "name");
   interface.version = intValue(xml, "version", 1);

   while (xml.readNextStartElement()) {
      if (xml.name() == "event") {
         interface.events.append(readEvent(xml, false));

      } else if (xml.name() == "request") {
         interface.requests.append(readEvent(xml, true));

      } else if (xml.name() == "enum") {
         interface.enums.append(readEnum(xml));

      } else {
         xml.skipCurrentElement();
      }
   }

   return interface;
}

QByteArray waylandTo_CType(const QByteArray &waylandType, const QByteArray &interface)
{
   if (waylandType == "string") {
      return "const char *";

   } else if (waylandType == "int") {
      return "int32_t";

   } else if (waylandType == "uint") {
      return "uint32_t";

   } else if (waylandType == "fixed") {
      return "wl_fixed_t";

   } else if (waylandType == "fd") {
      return "int32_t";

   } else if (waylandType == "array") {
      return "wl_array *";

   } else if (waylandType == "object" || waylandType == "new_id") {
      if (isServerSide()) {
         return "struct ::wl_resource *";
      }

      if (interface.isEmpty()) {
         return "struct ::wl_object *";
      }

      return "struct ::" + interface + " *";
   }

   return waylandType;
}

QByteArray waylandTo_CsType(const QByteArray &waylandType, const QByteArray &interface, bool cStyleArray)
{
   if (waylandType == "string") {
      return "const QString &";

   } else if (waylandType == "array") {
      return cStyleArray ? "wl_array *" : "const QByteArray &";

   } else {
      return waylandTo_CType(waylandType, interface);
   }
}

const WaylandArgument *newIdArgument(const QList<WaylandArgument> &arguments)
{
   for (const auto &item : arguments) {
      if (item.type == "new_id") {
         return &item;
      }
   }

   return nullptr;
}

void printEvent(const WaylandEvent &e, bool omitNames = false, bool withResource = false)
{
   printf("%s(", e.name.constData());

   bool needsComma = false;

   if (isServerSide()) {
      if (e.request) {
         printf("Resource *%s", omitNames ? "" : "resource");
         needsComma = true;

      } else if (withResource) {
         printf("struct ::wl_resource *%s", omitNames ? "" : "resource");
         needsComma = true;
      }
   }

   for (const auto &item : e.arguments) {
      bool isNewId = item.type == "new_id";

      if (isNewId && ! isServerSide() && (item.interface.isEmpty() != e.request)) {
         continue;
      }

      if (needsComma) {
         printf(", ");
      }

      needsComma = true;

      if (isNewId) {
         if (isServerSide()) {
            if (e.request) {
               printf("uint32_t");

               if (! omitNames) {
                  printf(" %s", item.name.constData());
               }

               continue;
            }

         } else {
            if (e.request) {
               printf("const struct ::wl_interface *%s, uint32_t%s", omitNames ? "" : "interface", omitNames ? "" : " version");
               continue;
            }
         }
      }

      QByteArray csType = waylandTo_CsType(item.type, item.interface, e.request == isServerSide());

      printf("%s%s%s", csType.constData(), csType.endsWith("&") || csType.endsWith("*") ? "" : " ",
            omitNames ? "" : item.name.constData());
   }

   printf(")");
}

void printEventHandlerSignature(const WaylandEvent &e, const char *interfaceName, bool deepIndent)
{
   const char *indent = deepIndent ? "   " : "";

   printf("handle_%s(\n", e.name.constData());

   if (isServerSide()) {
      printf("      %s::wl_client *client,\n", indent);
      printf("      %sstruct wl_resource *resource", indent);
   } else {
      printf("      %svoid *data,\n", indent);
      printf("      %sstruct ::%s *object", indent, interfaceName);
   }

   for (const auto &item : e.arguments) {
      printf(",\n");

      bool isNewId = item.type == "new_id";

      if (isServerSide() && isNewId) {
         printf("      %suint32_t %s", indent, item.name.constData());
      } else {
         QByteArray cType = waylandTo_CType(item.type, item.interface);
         printf("      %s%s%s%s", indent, cType.constData(), cType.endsWith("*") ? "" : " ", item.name.constData());
      }
   }

   printf(")");
}

void printEnums(const QList<WaylandEnum> &enums)
{
   for (const auto &item : enums) {
      printf("      enum %s {\n", item.name.constData());

      for (const auto &info : item.entries) {
         printf("         %s_%s = %s,", item.name.constData(), info.name.constData(), info.value.constData());

         if (! info.summary.isNull()) {
            printf("      // %s", info.summary.constData());
         }

         printf("\n");
      }

      printf("      };\n");
      printf("\n");
   }
}

QByteArray stripInterfaceName(const QByteArray &name, const QByteArray &prefix)
{
   if (! prefix.isEmpty() && name.startsWith(prefix)) {
      return name.mid(prefix.size());
   }

   if (name.startsWith("qt_") || name.startsWith("wl_")) {
      return name.mid(3);
   }

   return name;
}

bool ignoreInterface(const QByteArray &name)
{
   return name == "wl_display" || (isServerSide() && name == "wl_registry");
}

void process(QXmlStreamReader &xml, QFile &outputFile, const QByteArray &headerPath, const QByteArray &prefix)
{
   if (! xml.readNextStartElement()) {
      return;
   }

   if (xml.name() != "protocol") {
      xml.raiseError("Input file is not a wayland protocol file.");
      return;
   }

   QByteArray protocolName = byteArrayValue(xml, "name");

   if (protocolName.isEmpty()) {
      xml.raiseError("Missing protocol name.");
      return;
   }

   // redirect output
   dup2(outputFile.handle(), 1);

   // consider converting "-" to "_" so the preprocessor will not generate code which could lead to unexpected behavior
   // However, the wayland-scanner does not do this, leave as is for now
   // QByteArray preProcessorProtocolName = QByteArray(protocolName).replace('-', '_').toUpper();

   QByteArray preProcessorProtocolName = protocolName.toUpper();

   QList<WaylandInterface> interfaces;

   while (xml.readNextStartElement()) {
      if (xml.name() == "interface") {
         interfaces.append(readInterface(xml));
      } else {
         xml.skipCurrentElement();
      }
   }

   if (xml.hasError()) {
      return;
   }

   if (option == ServerHeader) {
      QByteArray inclusionGuard = QByteArray("CS_WAYLAND_SERVER_") + preProcessorProtocolName.constData();

      printf("#ifndef %s\n", inclusionGuard.constData());
      printf("#define %s\n", inclusionGuard.constData());
      printf("\n");

      printf("#include <QByteArray>\n");
      printf("#include <QMultiMap>\n");
      printf("#include <QString>\n");
      printf("\n");

      printf("#include \"wayland-server-core.h\"\n");

      if (headerPath.isEmpty()) {
         printf("#include \"wayland-%s-server-protocol.h\"\n", QByteArray(protocolName).replace('_', '-').constData());
      } else {
         printf("#include <%s/wayland-%s-server-protocol.h>\n", headerPath.constData(), QByteArray(protocolName).replace('_', '-').constData());
      }
      printf("\n");

      printf("#ifndef WAYLAND_VERSION_CHECK\n");
      printf("#define WAYLAND_VERSION_CHECK(major, minor, micro) \\\n");
      printf("   ((WAYLAND_VERSION_MAJOR > (major)) || \\\n");
      printf("   (WAYLAND_VERSION_MAJOR == (major) && WAYLAND_VERSION_MINOR > (minor)) || \\\n");
      printf("   (WAYLAND_VERSION_MAJOR == (major) && WAYLAND_VERSION_MINOR == (minor) && WAYLAND_VERSION_MICRO >= (micro)))\n");
      printf("#endif\n");
      printf("\n");

      QByteArray serverExport;

      if (headerPath.size()) {
         serverExport = QByteArray("Q_WAYLAND_SERVER_") + preProcessorProtocolName + "_EXPORT";

         printf("\n");
         printf("#if ! defined(%s)\n", csPrintable(serverExport));
         printf("#   define %s Q_DECL_EXPORT\n", csPrintable(serverExport));
         printf("#endif\n");
         printf("\n");
      }

      printf("namespace QtWaylandServer {\n");

      for (int j = 0; j < interfaces.size(); ++j) {
         const WaylandInterface &interface = interfaces.at(j);

         if (ignoreInterface(interface.name)) {
            continue;
         }

         const char *interfaceName = interface.name.constData();

         QByteArray stripped = stripInterfaceName(interface.name, prefix);
         const char *interfaceNameStripped = stripped.constData();

         printf("   class %s %s\n   {\n", serverExport.constData(), interfaceName);
         printf("    public:\n");
         printf("      %s(struct ::wl_client *client, int id, int version);\n", interfaceName);
         printf("      %s(struct ::wl_display *display, int version);\n", interfaceName);
         printf("      %s(struct ::wl_resource *resource);\n", interfaceName);
         printf("      %s();\n", interfaceName);
         printf("\n");
         printf("      virtual ~%s();\n", interfaceName);
         printf("\n");
         printf("      class Resource\n");
         printf("      {\n");
         printf("       public:\n");
         printf("         Resource()\n            : %s_object(nullptr), handle(nullptr)\n", interfaceNameStripped);
         printf("         { }\n");
         printf("\n");
         printf("         virtual ~Resource()\n         { }\n");
         printf("\n");
         printf("         %s *%s_object;\n", interfaceName, interfaceNameStripped);
         printf("         %s *object() { return %s_object; }\n", interfaceName, interfaceNameStripped);
         printf("\n");
         printf("         struct ::wl_resource *handle;\n");
         printf("\n");
         printf("         struct ::wl_client *client() const { return wl_resource_get_client(handle); }\n");
         printf("         int version() const { return wl_resource_get_version(handle); }\n");
         printf("\n");
         printf("         static Resource *fromResource(struct ::wl_resource *resource);\n");
         printf("      };\n");
         printf("\n");
         printf("      void init(struct ::wl_client *client, int id, int version);\n");
         printf("      void init(struct ::wl_display *display, int version);\n");
         printf("      void init(struct ::wl_resource *resource);\n");
         printf("\n");
         printf("      Resource *add(struct ::wl_client *client, int version);\n");
         printf("      Resource *add(struct ::wl_client *client, int id, int version);\n");
         printf("      Resource *add(struct wl_list *resource_list, struct ::wl_client *client, int id, int version);\n");
         printf("\n");
         printf("      Resource *resource() { return m_resource; }\n");
         printf("      const Resource *resource() const { return m_resource; }\n");
         printf("\n");
         printf("      QMultiMap<struct ::wl_client *, Resource *> resourceMap() { return m_resource_map; }\n");
         printf("      const QMultiMap<struct ::wl_client*, Resource *> resourceMap() const { return m_resource_map; }\n");
         printf("\n");
         printf("      bool isGlobal() const { return m_global != nullptr; }\n");
         printf("      bool isResource() const { return m_resource != nullptr; }\n");
         printf("\n");
         printf("      static const struct ::wl_interface *interface();\n");
         printf("      static QByteArray interfaceName() { return interface()->name; }\n");
         printf("      static int interfaceVersion() { return interface()->version; }\n");
         printf("\n");

         printEnums(interface.enums);

         bool hasEvents = ! interface.events.isEmpty();

         if (hasEvents) {
            for (const WaylandEvent &e : interface.events) {
               printf("      void send_");
               printEvent(e);
               printf(";\n");

               printf("      void send_");
               printEvent(e, false, true);
               printf(";\n");
            }

            printf("\n");
         }

         printf("    protected:\n");
         printf("      virtual Resource *%s_allocate();\n", interfaceNameStripped);
         printf("\n");
         printf("      virtual void %s_bind_resource(Resource *resource);\n", interfaceNameStripped);
         printf("      virtual void %s_destroy_resource(Resource *resource);\n", interfaceNameStripped);

         bool hasRequests = ! interface.requests.isEmpty();

         if (hasRequests) {
            printf("\n");

            for (const WaylandEvent &item : interface.requests) {
               printf("      virtual void %s_", interfaceNameStripped);
               printEvent(item);
               printf(";\n");
            }
         }

         printf("\n");
         printf("    private:\n");
         printf("      static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);\n");
         printf("      static void destroy_func(struct ::wl_resource *client_resource);\n");
         printf("      static void display_destroy_func(struct ::wl_listener *listener, void *data);\n");
         printf("\n");

         printf("      Resource *bind(struct ::wl_client *client, uint32_t id, int version);\n");
         printf("      Resource *bind(struct ::wl_resource *handle);\n");

         if (hasRequests) {
            printf("\n");
            printf("      static const struct ::%s_interface m_%s_interface;\n", interfaceName, interfaceName);

            printf("\n");

            for (auto &item : interface.requests) {
               printf("      static void ");
               printEventHandlerSignature(item, interfaceName, true);
               printf(";\n\n");
            }

         } else {
            printf("\n");
         }

         printf("      QMultiMap<struct ::wl_client *, Resource *> m_resource_map;\n");
         printf("      Resource *m_resource;\n");
         printf("      struct ::wl_global *m_global;\n");
         printf("      uint32_t m_globalVersion;\n");
         printf("\n");

         printf("      struct DisplayDestroyedListener : public ::wl_listener {\n");
         printf("         %s *parent;\n", interfaceName);
         printf("      };\n\n");

         printf("      DisplayDestroyedListener m_displayDestroyedListener;\n");
         printf("   };\n");

         if (j < interfaces.size() - 1) {
            printf("\n");
         }
      }

      printf("}\n");
      printf("\n");
      printf("\n");
      printf("#endif\n");
   }

   if (option == ServerCode) {
      if (headerPath.isEmpty()) {
         printf("#include \"qwayland-server-%s.h\"\n", QByteArray(protocolName).replace('_', '-').constData());
      } else {
         printf("#include <%s/qwayland-server-%s.h>\n", headerPath.constData(), QByteArray(protocolName).replace('_', '-').constData());
      }
      printf("\n");

      printf("namespace QtWaylandServer {\n");

      bool needsNewLine = false;

      for (int j = 0; j < interfaces.size(); ++j) {
         const WaylandInterface &interface = interfaces.at(j);

         if (ignoreInterface(interface.name)) {
            continue;
         }

         if (needsNewLine) {
            printf("\n");
         }

         needsNewLine = true;

         const char *interfaceName = interface.name.constData();

         QByteArray stripped = stripInterfaceName(interface.name, prefix);
         const char *interfaceNameStripped = stripped.constData();

         printf("   %s::%s(struct ::wl_client *client, int id, int version)\n", interfaceName, interfaceName);
         printf("      : m_resource_map(), m_resource(nullptr), m_global(nullptr)\n");
         printf("   {\n");
         printf("      init(client, id, version);\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::%s(struct ::wl_display *display, int version)\n", interfaceName, interfaceName);
         printf("      : m_resource_map(), m_resource(nullptr), m_global(nullptr)\n");
         printf("   {\n");
         printf("      init(display, version);\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::%s(struct ::wl_resource *resource)\n", interfaceName, interfaceName);
         printf("      : m_resource_map(), m_resource(nullptr), m_global(nullptr)\n");
         printf("   {\n");
         printf("      init(resource);\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::%s()\n", interfaceName, interfaceName);
         printf("      : m_resource_map(), m_resource(nullptr), m_global(nullptr)\n");
         printf("   {\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::~%s()\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("      for (const auto &item : m_resource_map) {\n");
         printf("         item->%s_object = nullptr;\n", interfaceNameStripped);
         printf("      }\n");
         printf("\n");

         printf("      if (m_resource != nullptr) {\n");
         printf("         m_resource->%s_object = nullptr;\n", interfaceNameStripped);
         printf("      }\n");
         printf("\n");

         printf("      if (m_global != nullptr) {\n");
         printf("         wl_global_destroy(m_global);\n");
         printf("         wl_list_remove(&m_displayDestroyedListener.link);\n");
         printf("      }\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::init(struct ::wl_client *client, int id, int version)\n", interfaceName);
         printf("   {\n");
         printf("      m_resource = bind(client, id, version);\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::init(struct ::wl_display *display, int version)\n", interfaceName);
         printf("   {\n");
         printf("      m_global = wl_global_create(display, &::%s_interface, version, this, bind_func);\n", interfaceName);
         printf("      m_globalVersion = version;\n");
         printf("      m_displayDestroyedListener.notify = %s::display_destroy_func;\n", interfaceName);
         printf("      m_displayDestroyedListener.parent = this;\n");
         printf("      wl_display_add_destroy_listener(display, &m_displayDestroyedListener);\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::init(struct ::wl_resource *resource)\n", interfaceName);
         printf("   {\n");
         printf("      m_resource = bind(resource);\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::Resource *%s::add(struct ::wl_client *client, int version)\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("      Resource *resource = bind(client, 0, version);\n");
         printf("      m_resource_map.insert(client, resource);\n");
         printf("\n");
         printf("      return resource;\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::Resource *%s::add(struct ::wl_client *client, int id, int version)\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("      Resource *resource = bind(client, id, version);\n");
         printf("      m_resource_map.insert(client, resource);\n");
         printf("\n");
         printf("      return resource;\n");
         printf("   }\n");
         printf("\n");

         printf("   const struct wl_interface *%s::interface()\n", interfaceName);
         printf("   {\n");
         printf("      return &::%s_interface;\n", interfaceName);
         printf("   }\n");
         printf("\n");
         printf("   %s::Resource *%s::%s_allocate()\n", interfaceName, interfaceName, interfaceNameStripped);
         printf("   {\n");
         printf("      return new Resource;\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::%s_bind_resource(Resource *)\n", interfaceName, interfaceNameStripped);
         printf("   {\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::%s_destroy_resource(Resource *)\n", interfaceName, interfaceNameStripped);
         printf("   {\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)\n", interfaceName);
         printf("   {\n");
         printf("      %s *self = static_cast<%s *>(data);\n", interfaceName, interfaceName);
         printf("      self->add(client, id, qMin(self->m_globalVersion, version));\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::display_destroy_func(struct ::wl_listener *listener, void *data)\n", interfaceName);
         printf("   {\n");
         printf("      (void) data;\n");
         printf("\n");
         printf("      %s *self = static_cast<%s::DisplayDestroyedListener *>(listener)->parent;\n", interfaceName, interfaceName);
         printf("      self->m_global = nullptr;\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::destroy_func(struct ::wl_resource *client_resource)\n", interfaceName);
         printf("   {\n");
         printf("      Resource *resource = Resource::fromResource(client_resource);\n");
         printf("      %s *self = resource->%s_object;\n", interfaceName, interfaceNameStripped);
         printf("\n");

         printf("      if (self != nullptr) {\n");
         printf("         self->m_resource_map.remove(resource->client(), resource);\n");
         printf("         self->%s_destroy_resource(resource);\n", interfaceNameStripped);
         printf("\n");
         printf("         self = resource->%s_object;\n", interfaceNameStripped);
         printf("\n");
         printf("         if (self != nullptr && self->m_resource == resource) {\n");
         printf("            self->m_resource = nullptr;\n");
         printf("         }\n");
         printf("      }\n");
         printf("\n");

         printf("      delete resource;\n");
         printf("   }\n");
         printf("\n");

         bool hasRequests = ! interface.requests.isEmpty();

         QByteArray interfaceMember = hasRequests ? "&m_" + interface.name + "_interface" : QByteArray("nullpr");

         printf("   %s::Resource *%s::bind(struct ::wl_client *client, uint32_t id, int version)\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("      struct ::wl_resource *handle = wl_resource_create(client, &::%s_interface, version, id);\n", interfaceName);
         printf("      return bind(handle);\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::Resource *%s::bind(struct ::wl_resource *handle)\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("      Resource *resource = %s_allocate();\n", interfaceNameStripped);
         printf("      resource->%s_object = this;\n", interfaceNameStripped);
         printf("\n");

         printf("      wl_resource_set_implementation(handle, %s, resource, destroy_func);", interfaceMember.constData());
         printf("\n");
         printf("      resource->handle = handle;\n");
         printf("      %s_bind_resource(resource);\n", interfaceNameStripped);
         printf("\n");
         printf("      return resource;\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::Resource *%s::Resource::fromResource(struct ::wl_resource *resource)\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("      if (resource == nullptr) {\n");
         printf("         return nullptr;\n");
         printf("      } else if (wl_resource_instance_of(resource, &::%s_interface, %s)) {\n", interfaceName, interfaceMember.constData());
         printf("         return static_cast<Resource *>(wl_resource_get_user_data(resource));\n");
         printf("      }\n");
         printf("\n");
         printf("      return nullptr;\n");
         printf("   }\n");

         if (hasRequests) {
            printf("\n");
            printf("   const struct ::%s_interface %s::m_%s_interface = {", interfaceName, interfaceName, interfaceName);

            for (int i = 0; i < interface.requests.size(); ++i) {
               if (i > 0) {
                  printf(",");
               }
               printf("\n");

               const WaylandEvent &e = interface.requests.at(i);
               printf("      %s::handle_%s", interfaceName, e.name.constData());
            }

            printf("\n");
            printf("   };\n");

            for (const WaylandEvent &e : interface.requests) {
               printf("\n");
               printf("   void %s::%s_", interfaceName, interfaceNameStripped);
               printEvent(e, true);
               printf("\n");
               printf("   {\n");
               printf("   }\n");
            }

            for (int i = 0; i < interface.requests.size(); ++i) {
               printf("\n");
               printf("   void %s::", interfaceName);

               const WaylandEvent &e = interface.requests.at(i);
               printEventHandlerSignature(e, interfaceName, false);
               printf("\n");

               printf("   {\n");
               printf("      (void) client;\n");
               printf("\n");

               printf("      Resource *r = Resource::fromResource(resource);\n");

               printf("      if (! r->%s_object) {\n", interfaceNameStripped);
               if (e.type == "destructor") {
                  printf("         wl_resource_destroy(resource);\n");
               }

               printf("         return;\n");
               printf("      }\n");
               printf("\n");

               printf("      static_cast<%s *>(r->%s_object)->%s_%s(r", interfaceName, interfaceNameStripped,
                     interfaceNameStripped, e.name.constData());

               for (int i = 0; i < e.arguments.size(); ++i) {
                  printf(",\n");
                  const WaylandArgument &a = e.arguments.at(i);

                  QByteArray cType  = waylandTo_CType(a.type, a.interface);
                  QByteArray csType = waylandTo_CsType(a.type, a.interface, e.request);

                  const char *argumentName = a.name.constData();

                  if (cType == csType) {
                     printf("            %s", argumentName);
                  }

                  else if (a.type == "string") {
                     printf("            QString::fromUtf8(%s)", argumentName);
                  }
               }

               printf(");\n");
               printf("   }\n");
            }
         }

         for (int i = 0; i < interface.events.size(); ++i) {
            const WaylandEvent &e = interface.events.at(i);

            printf("\n");
            printf("   void %s::send_", interfaceName);
            printEvent(e);
            printf("\n");
            printf("   {\n");
            printf("      if (m_resource == nullptr) {\n");
            printf("         qWarning(\"Unable to call %s::%s, not initialised\");\n", interfaceName, e.name.constData());
            printf("         return;\n");
            printf("      }\n");
            printf("\n");
            printf("      send_%s(m_resource->handle", e.name.constData());

            for (auto &item : e.arguments) {
               printf(",\n");
               printf("            %s", item.name.constData());
            }

            printf(");\n");
            printf("   }\n");
            printf("\n");

            printf("   void %s::send_", interfaceName);
            printEvent(e, false, true);
            printf("\n");
            printf("   {\n");

            for (int i = 0; i < e.arguments.size(); ++i) {
               const WaylandArgument &a = e.arguments.at(i);

               if (a.type != "array") {
                  continue;
               }

               QByteArray array = a.name + "_data";
               const char *arrayName = array.constData();
               const char *variableName = a.name.constData();

               printf("      struct wl_array %s;\n", arrayName);
               printf("      %s.size = %s.size();\n", arrayName, variableName);
               printf("      %s.data = static_cast<void *>(const_cast<char *>(%s.constData()));\n", arrayName, variableName);
               printf("      %s.alloc = 0;\n", arrayName);
               printf("\n");
            }

            printf("      %s_send_%s(resource", interfaceName, e.name.constData());

            for (int i = 0; i < e.arguments.size(); ++i) {
               const WaylandArgument &a = e.arguments.at(i);
               printf(",\n");

               QByteArray cType  = waylandTo_CType(a.type, a.interface);
               QByteArray csType = waylandTo_CsType(a.type, a.interface, e.request);

               if (a.type == "string") {
                  printf("            %s.constData()", a.name.constData());

               } else if (a.type == "array") {
                  printf("            &%s_data", a.name.constData());

               } else if (cType == csType) {
                  printf("            %s", a.name.constData());
               }
            }

            printf(");\n");
            printf("   }\n");
         }
      }

      printf("}\n");
      printf("\n");
   }

   if (option == ClientHeader) {
      QByteArray inclusionGuard = QByteArray("WAYLAND_") + preProcessorProtocolName.constData();

      printf("#ifndef %s\n", csPrintable(inclusionGuard));
      printf("#define %s\n", csPrintable(inclusionGuard));
      printf("\n");

      printf("#include <QByteArray>\n");
      printf("#include <QString>\n");
      printf("\n");

      if (headerPath.isEmpty()) {
         printf("#include \"wayland-%s-client-protocol.h\"\n", QByteArray(protocolName).replace('_', '-').constData());
      } else {
         printf("#include <%s/wayland-%s-client-protocol.h>\n", headerPath.constData(), QByteArray(protocolName).replace('_', '-').constData());
      }
      printf("\n");

      printf("struct wl_registry;\n\n");

      QByteArray clientExport;

      if (headerPath.size()) {
         clientExport = QByteArray("Q_WAYLAND_CLIENT_") + preProcessorProtocolName + "_EXPORT";

         printf("\n");
         printf("#if ! defined(%s)\n", csPrintable(clientExport));
         printf("#   define %s Q_DECL_EXPORT\n", csPrintable(clientExport));
         printf("#endif\n");
         printf("\n");
      }

      printf("namespace QtWayland {\n");

      for (int j = 0; j < interfaces.size(); ++j) {
         const WaylandInterface &interface = interfaces.at(j);

         if (ignoreInterface(interface.name)) {
            continue;
         }

         const char *interfaceName = interface.name.constData();

         QByteArray stripped = stripInterfaceName(interface.name, prefix);
         const char *interfaceNameStripped = stripped.constData();

         printf("   class %s %s\n   {\n", clientExport.constData(), interfaceName);
         printf("    public:\n");
         printf("      %s(struct ::wl_registry *registry, int id, int version);\n", interfaceName);
         printf("      %s(struct ::%s *object);\n", interfaceName, interfaceName);
         printf("      %s();\n", interfaceName);
         printf("\n");
         printf("      virtual ~%s();\n", interfaceName);
         printf("\n");
         printf("      void init(struct ::wl_registry *registry, int id, int version);\n");
         printf("      void init(struct ::%s *object);\n", interfaceName);
         printf("\n");
         printf("      struct ::%s *object() { return m_%s; }\n", interfaceName, interfaceName);
         printf("      const struct ::%s *object() const { return m_%s; }\n", interfaceName, interfaceName);
         printf("      static %s *fromObject(struct ::%s *object);\n", interfaceName, interfaceName);
         printf("\n");
         printf("      bool isInitialized() const;\n");
         printf("\n");
         printf("      static const struct ::wl_interface *interface();\n");
         printf("\n");

         printEnums(interface.enums);

         if (! interface.requests.isEmpty()) {
            for (const WaylandEvent &e : interface.requests) {
               const WaylandArgument *new_id = newIdArgument(e.arguments);
               QByteArray new_id_str = "void ";

               if (new_id) {
                  if (new_id->interface.isEmpty()) {
                     new_id_str = "void *";
                  } else {
                     new_id_str = "struct ::" + new_id->interface + " *";
                  }
               }

               printf("      %s", new_id_str.constData());
               printEvent(e);
               printf(";\n");
            }
         }

         bool hasEvents = ! interface.events.isEmpty();

         if (hasEvents) {
            printf("\n");
            printf("    protected:\n");

            for (const WaylandEvent &e : interface.events) {
               printf("      virtual void %s_", interfaceNameStripped);
               printEvent(e);
               printf(";\n");
            }
         }

         printf("\n");
         printf("    private:\n");

         if (hasEvents) {
            printf("      void init_listener();\n");
            printf("      static const struct %s_listener m_%s_listener;\n", interfaceName, interfaceName);

            for (const auto &item : interface.events) {
               printf("      static void ");
               printEventHandlerSignature(item, interfaceName, true);
               printf(";\n\n");
            }
         }

         printf("      struct ::%s *m_%s;\n", interfaceName, interfaceName);
         printf("   };\n");

         if (j < interfaces.size() - 1) {
            printf("\n");
         }
      }

      printf("}\n");
      printf("\n");
      printf("\n");
      printf("#endif\n");
   }

   if (option == ClientCode) {
      if (headerPath.isEmpty()) {
         printf("#include \"qwayland-%s.h\"\n", QByteArray(protocolName).replace('_', '-').constData());
      } else {
         printf("#include <%s/qwayland-%s.h>\n", headerPath.constData(), QByteArray(protocolName).replace('_', '-').constData());
      }
      printf("\n");

      printf("namespace QtWayland {\n");
      printf("\n");

      for (int j = 0; j < interfaces.size(); ++j) {
         const WaylandInterface &interface = interfaces.at(j);

         if (ignoreInterface(interface.name)) {
            continue;
         }

         const char *interfaceName = interface.name.constData();

         QByteArray stripped = stripInterfaceName(interface.name, prefix);
         const char *interfaceNameStripped = stripped.constData();

         bool hasEvents = ! interface.events.isEmpty();

         printf("   %s::%s(struct ::wl_registry *registry, int id, int version)\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("      init(registry, id, version);\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::%s(struct ::%s *obj)\n", interfaceName, interfaceName, interfaceName);
         printf("      : m_%s(obj)\n", interfaceName);
         printf("   {\n");

         if (hasEvents) {
            printf("      init_listener();\n");
         }

         printf("   }\n");
         printf("\n");

         printf("   %s::%s()\n", interfaceName, interfaceName);
         printf("      : m_%s(nullptr)\n", interfaceName);
         printf("   {\n");
         printf("   }\n");
         printf("\n");

         printf("   %s::~%s()\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("   }\n");
         printf("\n");

         printf("   void %s::init(struct ::wl_registry *registry, int id, int version)\n", interfaceName);
         printf("   {\n");
         printf("      m_%s = static_cast<struct ::%s *>(wl_registry_bind(registry, id, &%s_interface, version));\n",
               interfaceName, interfaceName, interfaceName);

         if (hasEvents) {
            printf("      init_listener();\n");
         }

         printf("   }\n");
         printf("\n");

         printf("   void %s::init(struct ::%s *obj)\n", interfaceName, interfaceName);
         printf("   {\n");
         printf("      m_%s = obj;\n", interfaceName);

         if (hasEvents) {
            printf("      init_listener();\n");
         }
         printf("   }\n");
         printf("\n");

         printf("   %s *%s::fromObject(struct ::%s *object)\n", interfaceName, interfaceName, interfaceName);
         printf("   {\n");
         if (hasEvents) {
            printf("      if (wl_proxy_get_listener((struct ::wl_proxy *)object) != (void *)&m_%s_listener)  {\n", interfaceName);
            printf("         return nullptr;\n");
            printf("      }\n");
            printf("\n");
         }

         printf("      return static_cast<%s *>(%s_get_user_data(object));\n", interfaceName, interfaceName);
         printf("   }\n");
         printf("\n");

         printf("   bool %s::isInitialized() const\n", interfaceName);
         printf("   {\n");
         printf("      return m_%s != nullptr;\n", interfaceName);
         printf("   }\n");
         printf("\n");

         printf("   const struct wl_interface *%s::interface()\n", interfaceName);
         printf("   {\n");
         printf("      return &::%s_interface;\n", interfaceName);
         printf("   }\n");

         for (int i = 0; i < interface.requests.size(); ++i) {
            printf("\n");

            const WaylandEvent &e = interface.requests.at(i);
            const WaylandArgument *new_id = newIdArgument(e.arguments);
            QByteArray new_id_str = "void ";

            if (new_id) {
               if (new_id->interface.isEmpty()) {
                  new_id_str = "void *";
               } else {
                  new_id_str = "struct ::" + new_id->interface + " *";
               }
            }

            printf("   %s%s::", new_id_str.constData(), interfaceName);
            printEvent(e);
            printf("\n");

            printf("   {\n");

            for (int i = 0; i < e.arguments.size(); ++i) {
               const WaylandArgument &a = e.arguments.at(i);

               if (a.type != "array") {
                  continue;
               }

               QByteArray array = a.name + "_data";
               const char *arrayName = array.constData();
               const char *variableName = a.name.constData();

               printf("      struct wl_array %s;\n", arrayName);
               printf("      %s.size  = %s.size();\n", arrayName, variableName);
               printf("      %s.data  = static_cast<void *>(const_cast<char *>(%s.constData()));\n", arrayName, variableName);
               printf("      %s.alloc = 0;\n", arrayName);
               printf("\n");
            }

            int actualArgumentCount = new_id ? e.arguments.size() - 1 : e.arguments.size();
            printf("      %s%s_%s(\n", new_id ? "return " : "", interfaceName, e.name.constData());
            printf("            m_%s%s", interfaceName, actualArgumentCount > 0 ? "," : "");

            bool needsComma = false;

            for (int i = 0; i < e.arguments.size(); ++i) {
               const WaylandArgument &a = e.arguments.at(i);
               bool isNewId = a.type == "new_id";

               if (isNewId && ! a.interface.isEmpty()) {
                  continue;
               }

               if (needsComma) {
                  printf(",");
               }

               needsComma = true;
               printf("\n");

               if (isNewId) {
                  printf("            interface,\n");
                  printf("            version");

               } else {
                  QByteArray cType  = waylandTo_CType(a.type, a.interface);
                  QByteArray csType = waylandTo_CsType(a.type, a.interface, e.request);

                  if (a.type == "string") {
                     printf("            %s.constData()", a.name.constData());

                  } else if (a.type == "array") {
                     printf("            &%s_data", a.name.constData());

                  } else if (cType == csType) {
                     printf("            %s", a.name.constData());

                  }
               }
            }

            printf(");\n");

            if (e.type == "destructor") {
               printf("      m_%s = nullptr;\n", interfaceName);
            }

            printf("   }\n");
         }

         if (hasEvents) {
            printf("\n");

            for (int i = 0; i < interface.events.size(); ++i) {
               const WaylandEvent &e = interface.events.at(i);

               printf("   void %s::%s_", interfaceName, interfaceNameStripped);
               printEvent(e, true);

               printf("\n");
               printf("   {\n");
               printf("   }\n");
               printf("\n");

               printf("   void %s::", interfaceName);
               printEventHandlerSignature(e, interfaceName, false);
               printf("\n");

               printf("   {\n");
               printf("      (void) object;\n\n");
               printf("      static_cast<%s *>(data)->%s_%s(", interfaceName, interfaceNameStripped, e.name.constData());

               for (int i = 0; i < e.arguments.size(); ++i) {
                  printf("\n");
                  const WaylandArgument &a = e.arguments.at(i);

                  const char *argumentName = a.name.constData();

                  if (a.type == "string") {
                     printf("         QString::fromUtf8(%s)", argumentName);
                  } else {
                     printf("         %s", argumentName);
                  }

                  if (i < e.arguments.size() - 1) {
                     printf(",");
                  }
               }

               printf(");\n");

               printf("   }\n");
               printf("\n");
            }

            printf("   const struct %s_listener %s::m_%s_listener = {\n", interfaceName, interfaceName, interfaceName);

            for (const auto &item : interface.events) {
               printf("      %s::handle_%s,\n", interfaceName, item.name.constData());
            }

            printf("   };\n");
            printf("\n");

            printf("   void %s::init_listener()\n", interfaceName);
            printf("   {\n");
            printf("      %s_add_listener(m_%s, &m_%s_listener, this);\n", interfaceName, interfaceName, interfaceName);
            printf("   }\n");
         }

         if (j < interfaces.size() - 1) {
            printf("\n");
         }
      }

      printf("}\n");
      printf("\n");
   }
}

int main(int argc, char **argv)
{
   if (argc <= 3 || ! parseOption(QString::fromUtf8(argv[1]), &option)) {
      fprintf(stderr, "Usage: %s [client-header|server-header|client-code|server-code] inputfile outputfile [header-path] [prefix]\n", argv[0]);
      return EXIT_FAILURE;
   }

   QCoreApplication app(argc, argv);

   QByteArray headerPath;

   if (argc >= 5) {
      headerPath = QByteArray(argv[4]);
   }

   QByteArray prefix;

   if (argc == 6) {
      prefix = QByteArray(argv[5]);
   }

   QFile inputfile(QString::fromUtf8(argv[2]));

   if (! inputfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      fprintf(stderr, "Unable to open input file %s\n", argv[2]);
      return EXIT_FAILURE;
   }

   QFile outputfile(QString::fromUtf8(argv[3]));

   if (outputfile.exists()) {
      outputfile.remove();
   }

   if (! outputfile.open(QIODevice::ReadWrite | QIODevice::Text)) {
      fprintf(stderr, "Unable to open output file %s\n", argv[3]);
      return EXIT_FAILURE;
   }

   QXmlStreamReader xml(&inputfile);
   process(xml, outputfile, headerPath, prefix);

   if (xml.hasError()) {
      fprintf(stderr, "XML error: %s\nLine %lld, column %lld\n", csPrintable(xml.errorString()), xml.lineNumber(), xml.columnNumber());
      return EXIT_FAILURE;
   }
}
