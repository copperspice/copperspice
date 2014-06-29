#############################################################################
# Qt Makefile - gencmap
#############################################################################

####### Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
LFLAGS	=	-L$(LIBDIR)
CC	=	gcc

####### Files

SOURCES	=	gencmap.cpp
OBJECTS	=	gencmap.o
TARGET	=	gencmap

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	-rm -f *.o *.bak *~ *% #*
	-rm -f $(TARGET)


# DO NOT DELETE THIS LINE -- make depend depends on it.
