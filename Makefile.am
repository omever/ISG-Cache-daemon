AM_CXXFLAGS = -DDATADIR='"$(datadir)"' -Isrc -Isrc/dispatcher -Isrc/oracle -Isrc/radius $(ORACLE_OCI_CFLAGS) $(XML_CFLAGS) $(MEMCACHED_CFLAGS) -I/usr/local/include/libmemcached
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = isg_cached
# find * -name \*.h -or -name \*.cpp | xargs echo
isg_cached_SOURCES = config.h src/listener.h src/main.cpp src/isg_cached.cpp src/oracle/billing.h src/oracle/billing.cpp src/oracle/queryresult.h src/oracle/queryresult.cpp src/radius/radclient.h src/radius/coapacket.h src/radius/radpacket.cpp src/radius/radpacket.h src/radius/dictionary.cpp src/radius/dictionary.h src/radius/coapacket.cpp src/radius/radclient.cpp src/listener.cpp src/dispatcher/coa.cpp src/dispatcher/oracle.cpp src/dispatcher/coa.h src/dispatcher/dispatcher.cpp src/dispatcher/oracle.h src/dispatcher/dispatcher.h
isg_cached_LDADD = -locci -lclntsh -lxml2 -lmemcached -lssl -lpthread $(ORACLE_OCI_LDFLAGS) $(XML_LIBS) $(MEMCACHED_LIBS)
isg_cached_DATA = radius/dictionary
isg_cacheddir = $(datadir)/isg_cached/radius

