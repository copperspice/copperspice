AUTOMAKE_OPTIONS = subdir-objects nostdinc
ACLOCAL_AMFLAGS = -I autotools/m4 --install
AM_CFLAGS =  -DQT_SHARED
AM_CXXFLAGS = $(AM_CFLAGS)

# for objective c
OBJCXXFLAGS = $(CXXFLAGS)

AM_LDFLAGS =
bin_PROGRAMS=
BUILT_SOURCES =
CLEANFILES =
noinst_HEADERS =
noinst_LTLIBRARIES =
lib_LTLIBRARIES =
EXTRA_LTLIBRARIES =
EXTRA_DIST= \
	README.md \
	CMakeLists.txt \
	cmake/modules/FindGLib2.cmake \
	cmake/modules/TestLargeFiles.cmake \
	cmake/modules/FindIconv.cmake \
	cmake/modules/FindNAS.cmake \
	cmake/modules/FindMySQL.cmake \
	cmake/modules/FindGStreamer.cmake \
	cmake/modules/CopperSpiceBuildMacros.cmake \
	cmake/modules/FindFontconfig.cmake \
	cmake/modules/FindGOBJECT2.cmake \
	cmake/modules/FindFreetype.cmake \
	cmake/CopperSpiceMacros.cmake \
	cmake/archlinux-mingw.cmake \
	cmake/CopperSpiceConfig.cmake \
	cmake/CopperSpiceConfigVersion.cmake \
	cmake/InstallMinGW.cmake \
	cmake/pkgconfig.cmake \
	cmake/qt-acconfig.h.cmake \
	src/tools/lconvert/CMakeLists.txt \
	src/tools/lrelease/CMakeLists.txt \
	src/tools/lupdate/CMakeLists.txt \
	src/tools/rcc/CMakeLists.txt \
	src/tools/shared/CMakeLists.txt \
	src/tools/uic/CMakeLists.txt \
	src/webkit/CMakeLists.txt


SUBDIRS = . src/webkit
DIST_SUBDIRS = src/webkit

# Do not alter lines above this comment

if OSTYPE_LINUX
AM_LDFLAGS += -avoid-version
endif

if OSTYPE_BSD
AM_LDFLAGS += -avoid-version
endif

if OSTYPE_WIN
AM_LDFLAGS += -avoid-version
endif

if OSTYPE_DARWIN
AM_LDFLAGS += -avoid-version
endif

# set other flags
if OSTYPE_WIN
AM_CFLAGS += -DUNICODE
endif

if CUPS
else
AM_CFLAGS += -DQT_NO_CUPS
endif

if GLIB
AM_CFLAGS += $(GLIB_CFLAGS)
AM_LDFLAGS += $(GLIB_LIBS)
else
AM_CFLAGS += -DQT_NO_GLIB
endif

if FREETYPE
AM_CFLAGS += $(FREETYPE_CFLAGS)
AM_LDFLAGS += $(FREETYPE_LIBS)
else
AM_CFLAGS += -DQT_NO_FREETYPE
endif

if ZLIB
AM_CFLAGS += $(ZLIB_CFLAGS)
AM_LDFLAGS += $(ZLIB_LIBS)
else
AM_CFLAGS += -I$(top_srcdir)/src/3rdparty/zlib
endif

include src/tools/uic/Makefile.am
include src/tools/rcc/Makefile.am
include src/tools/lrelease/Makefile.am
include src/tools/lupdate/Makefile.am
include src/tools/lconvert/Makefile.am

include src/core/Makefile.am
include src/gui/Makefile.am
include src/multimedia/Makefile.am
include src/network/Makefile.am
include src/opengl/Makefile.am
include src/sql/Makefile.am
include src/svg/Makefile.am
include src/phonon/Makefile.am
include src/script/Makefile.am
#out for now   include src/scripttools/Makefile.am
include src/xml/Makefile.am
include src/xmlpatterns/Makefile.am
#out for now   include src/declarative/Makefile.am

# Extra files that need to be distributed
EXTRA_DIST+= autotools/build-aux/git-version-gen \
	include/.empty-directory include/qt-acconfig.h.in \
	$(wildcard $(top_srcdir)/translations/*.ts) \
	$(wildcard $(top_srcdir)/translations/*.qph)
DISTCLEANFILES = .version.cpp .version.o .build-stamp-*

# Tag the distribution
dist-hook:
	echo $(VERSION) > $(distdir)/autotools/.tarball-version	
	find $(distdir) -name qrc_\*.cpp -exec rm -f '{}' ';'

# Workaround automake issue with large distributions
# This requires GNU make
write_entries_to_file_split=50
DISTFILES.list:
	$(shell rm -f DISTFILES.list || :) $(shell touch DISTFILES.list) \
	$(foreach range, \
	 $(shell i=1; while test $$i -le $(words $(DISTFILES)); do \
	  echo $$i; i=`expr $$i + $(write_entries_to_file_split)`; done), \
	$(shell echo $(wordlist $(range), \
	 $(shell expr $(range) + $(write_entries_to_file_split) - 1), $(DISTFILES)) \
	  | tr ' ' '\n' >> DISTFILES.list))


.PHONY:DISTFILES.list

distdir: $(DISTFILES) DISTFILES.list
	$(am__remove_distdir)
	test -d "$(distdir)" || mkdir "$(distdir)"
	@srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	topsrcdirstrip=`echo "$(top_srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	list=`cat DISTFILES.list`; \
	  dist_files=`for file in $$list; do echo $$file; done | \
	  sed -e "s|^$$srcdirstrip/||;t" \
	      -e "s|^$$topsrcdirstrip/|$(top_builddir)/|;t"`; \
	case $$dist_files in \
	  */*) $(MKDIR_P) `echo "$$dist_files" | \
	                   sed '/\//!d;s|^|$(distdir)/|;s,/[^/]*$$,,' | \
	                   sort -u` ;; \
	esac; \
	for file in $$dist_files; do \
	  if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
	  if test -d $$d/$$file; then \
	    dir=`echo "/$$file" | sed -e 's,/[^/]*$$,,'`; \
	    if test -d "$(distdir)/$$file"; then \
	      find "$(distdir)/$$file" -type d ! -perm -700 -exec chmod u+rwx {} \;; \
	    fi; \
	    if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
	      cp -fpR $(srcdir)/$$file "$(distdir)$$dir" || exit 1; \
	      find "$(distdir)/$$file" -type d ! -perm -700 -exec chmod u+rwx {} \;; \
	    fi; \
	    cp -fpR $$d/$$file "$(distdir)$$dir" || exit 1; \
	  else \
	    test -f "$(distdir)/$$file" \
	    || cp -p $$d/$$file "$(distdir)/$$file" \
	    || exit 1; \
	  fi; \
	done
	@list='$(DIST_SUBDIRS)'; for subdir in $$list; do \
	  if test "$$subdir" = .; then :; else \
	    $(am__make_dryrun) \
	      || test -d "$(distdir)/$$subdir" \
	      || $(MKDIR_P) "$(distdir)/$$subdir" \
	      || exit 1; \
	    dir1=$$subdir; dir2="$(distdir)/$$subdir"; \
	    $(am__relativize); \
	    new_distdir=$$reldir; \
	    dir1=$$subdir; dir2="$(top_distdir)"; \
	    $(am__relativize); \
	    new_top_distdir=$$reldir; \
	    echo " (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) top_distdir="$$new_top_distdir" distdir="$$new_distdir" \\"; \
	    echo "     am__remove_distdir=: am__skip_length_check=: am__skip_mode_fix=: distdir)"; \
	    ($(am__cd) $$subdir && \
	      $(MAKE) $(AM_MAKEFLAGS) \
	        top_distdir="$$new_top_distdir" \
	        distdir="$$new_distdir" \
	        am__remove_distdir=: \
	        am__skip_length_check=: \
	        am__skip_mode_fix=: \
	        distdir) \
	      || exit 1; \
	  fi; \
	done
	$(MAKE) $(AM_MAKEFLAGS) \
	  top_distdir="$(top_distdir)" distdir="$(distdir)" \
	  dist-hook
	-test -n "$(am__skip_mode_fix)" \
	|| find "$(distdir)" -type d ! -perm -755 \
	        -exec chmod u+rwx,go+rx {} \; -o \
	  ! -type d ! -perm -444 -links 1 -exec chmod a+r {} \; -o \
	  ! -type d ! -perm -400 -exec chmod a+r {} \; -o \
	  ! -type d ! -perm -444 -exec $(install_sh) -c -m a+r {} {} \; \
	|| chmod -R a+r "$(distdir)"
	find $(distdir) -exec touch -r $(distdir) \{\} \;


$(distdir)/%: $(DISTFILES)
	$(INSTALL) $^ $@

windows-zip: distdir
	-rm -f $(distdir).zip
	(cd $(distdir) && zip -rq ../$(distdir).zip .)
	$(am__remove_distdir)

webkit-headers:
	-cd src/webkit && $(MAKE) create-webkit-headers

lib/libCsWebKit@BUILD_ABI@.la: webkit-headers
	-cd src/webkit && $(MAKE) ../../lib/libCsWebKit@BUILD_ABI@.la


# Stamp the build with the correct version
.version.cpp: .build-stamp-$(VERSION)
	echo 'const char* copperspice_version = "copperspice $(VERSION)";' > .version.cpp
.build-stamp-$(VERSION):
	@rm -f .build-stamp*
	@touch $@

clean-local: clean-cs

clean-cs:
	rm -rf include/Qt* include/phonon privateinclude .*.mk	
	find . -name 'qrc_*.cpp' -exec rm -f \{\} \;
	find . -name 'ui_*.h' -exec rm -f \{\} \;	

install-data-hook: webkit-headers
	mkdir -p $(DESTDIR)$(includedir)
	cp -r include/Qt* include/qt-acconfig.h $(DESTDIR)$(includedir)
	if [ -d include/phonon ] ; then cp -r include/phonon $(DESTDIR)$(includedir) ; fi

uninstall-hook:
	for i in `cd include; find Qt* -type f`; do \
		rm  $(DESTDIR)$(includedir)/$$i; \
	done
	for i in `cd include; find Qt* -type f`; do \
		rmdir $(DESTDIR)$(includedir); \
	done

.PHONY: clean-qrc

ui_%.h: %.ui | bin/uic$(EXEEXT)
	bin/uic$(EXEEXT) $(UIC_FLAGS) $< -o $@

qrc_%.cpp: %.qrc | bin/rcc$(EXEEXT)
	bin/rcc$(EXEEXT) $(RCC_FLAGS) $< -name $(basename $(notdir $<)) -o $@
