lib_LTLIBRARIES += lib/libCsSql1.la
lib_libCsSql1_la_SOURCES =

lib_libCsSql1_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore  -Iinclude/QtSql  \
   -Iprivateinclude/QtCore/private  -Iprivateinclude/QtSql/private  \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"   \
   -DQT_SQL_SQLITE   \
	-DQT_BUILD_SQL_LIB \
	-DQT_SHARED   \
	-DQT_MAKEDLL

lib_libCsSql1_la_CFLAGS =
lib_libCsSql1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsSql1_la_LIBADD = lib/libCsCore1.la

if OSTYPE_WIN
lib_libCsSql1_la_LDFLAGS += -lws2_32
endif

SQL_INCLUDES =
SQL_PUBLIC_INCLUDES =
SQL_PRIVATE_INCLUDES =
SQL_DIRS =

EXTRA_DIST += \
	src/sql/CMakeLists.txt \
	src/sql/drivers/db2/db2.cmake \
	src/sql/drivers/ibase/ibase.cmake \
	src/sql/drivers/mysql/mysql.cmake \
	src/sql/drivers/oci/oci.cmake \
	src/sql/drivers/odbc/odbc.cmake \
	src/sql/drivers/psql/psql.cmake \
	src/sql/drivers/sqlite/sqlite.cmake \
	src/sql/kernel/kernel.cmake \
	src/sql/models/models.cmake

include src/sql/drivers/db2/Makefile.am
include src/sql/drivers/ibase/Makefile.am
include src/sql/drivers/mysql/Makefile.am
include src/sql/drivers/oci/Makefile.am
include src/sql/drivers/odbc/Makefile.am
include src/sql/drivers/psql/Makefile.am
include src/sql/drivers/sqlite/Makefile.am
include src/sql/kernel/Makefile.am
include src/sql/models/Makefile.am

SQL_HEADERS_INSTALLED= \
	$(patsubst %,include/QtSql/%,$(SQL_INCLUDES)) \
	$(patsubst %,include/QtSql/%,$(SQL_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtSql/private/%,$(SQL_PRIVATE_INCLUDES)) \
	include/QtSql privateinclude/QtSql/private

BUILT_SOURCES+=.sql-header-deps.mk

$(lib_libCsSql1_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(SQL_HEADERS_INSTALLED)

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
