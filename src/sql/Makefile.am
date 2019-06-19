lib_LTLIBRARIES += lib/libCsSql@BUILD_ABI@.la
lib_libCsSql@BUILD_ABI@_la_SOURCES =

lib_libCsSql@BUILD_ABI@_la_CXXFLAGS = -Iinclude \
	-Iinclude/QtCore -Iprivateinclude/QtCore/private \
	-Iinclude/QtSql -Iprivateinclude/QtSql/private  \
	$(AM_CXXFLAGS) \
	-DBUILD_DATE=\"`date +%Y-%m-%d`\"  \
	-DQT_SQL_SQLITE   \
	-DQT_BUILD_SQL_LIB

lib_libCsSql@BUILD_ABI@_la_CFLAGS =
lib_libCsSql@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsSql@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la

if OSTYPE_WIN
lib_libCsSql@BUILD_ABI@_la_LDFLAGS += -lws2_32
endif

SQL_INCLUDES =
SQL_PUBLIC_INCLUDES =
SQL_PRIVATE_INCLUDES =
SQL_DIRS =

include src/sql/kernel/Makefile.am
include src/sql/models/Makefile.am

# plugins
include src/plugins/sqldrivers/db2/Makefile.am
include src/plugins/sqldrivers/ibase/Makefile.am
include src/plugins/sqldrivers/mysql/Makefile.am
include src/plugins/sqldrivers/oci/Makefile.am
include src/plugins/sqldrivers/odbc/Makefile.am
include src/plugins/sqldrivers/psql/Makefile.am
include src/plugins/sqldrivers/sqlite/Makefile.am

EXTRA_DIST += \
	src/sql/CMakeLists.txt \
	src/sql/kernel/kernel.cmake \
	src/sql/models/models.cmake \
	\
	src/plugins/sqldrivers/db2/db2.cmake \
	src/plugins/sqldrivers/ibase/ibase.cmake \
	src/plugins/sqldrivers/mysql/mysql.cmake \
	src/plugins/sqldrivers/oci/oci.cmake \
	src/plugins/sqldrivers/odbc/odbc.cmake \
	src/plugins/sqldrivers/psql/psql.cmake \
	src/plugins/sqldrivers/sqlite/sqlite.cmake

SQL_HEADERS_INSTALLED= \
	$(patsubst %,include/QtSql/%,$(SQL_INCLUDES)) \
	$(patsubst %,include/QtSql/%,$(SQL_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtSql/private/%,$(SQL_PRIVATE_INCLUDES)) \
	include/QtSql privateinclude/QtSql/private

BUILT_SOURCES+=.sql-header-deps.mk

$(lib_libCsSql@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(SQL_HEADERS_INSTALLED)

-include .sql-header-deps.mk

.sql-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtSql:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtSql/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(SQL_PUBLIC_INCLUDES); do \
		echo "include/QtSql/$$i: include/QtSql/$$(echo $$i | tr A-Z a-z).h | include/QtSql" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(SQL_DIRS); do \
		echo "include/QtSql/%: $$i/% | include/QtSql" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(SQL_DIRS); do \
		echo "privateinclude/QtSql/private/%: $$i/% | privateinclude/QtSql/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done
