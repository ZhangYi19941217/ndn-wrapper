CC=gcc
CPP=g++
CFLAGS += -O2 -Wall -g -fPIC
CPPFLAGS += -std=c++11 -O2 -Wall -g -fPIC
LDFLAGS += -shared -fPIC -L. -lndn-cxx -lboost_system -lboost_filesystem -lboost_date_time -lboost_iostreams -lboost_regex -lboost_program_options -lboost_chrono -lboost_random -lssl -lcrypto -lcryptopp -lsqlite3 -lpthread 
#VPATH = ../src
all: libwrapperndn.so
    echo "All Done"
libwrapperndn.so: ndngetfile.o pipe.o
    $(CPP) $^ -o $@ $(LDFLAGS)
#pipemain: pipemain.o libwrapperndn.so
    $(CC) $^ -o $@ $(LDFLAGS)
clean:
    -rm -rf *.o *.so
.PHONY: clean
csources = pipe.c pipemain.c
cppsources = ndngetfile.cpp
CDEF = ${csources:.c=.d}
CPPDEF =  ${cppsources:.cpp=.d}
#include $(CDEF)
#include $(CPPDEF)
$(CDEF) : %.d : %.c
    set -e; rm -f $@; \
    $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$
$(CPPDEF) : %.d : %.cpp
    set -e; rm -f $@; \
    $(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$
prefix=/usr/local
install: libwrapperndn.so
    install -m 0755 libwrapperndn.so $(prefix)/lib
.PHONY: install
