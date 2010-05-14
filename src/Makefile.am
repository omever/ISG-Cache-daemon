AM_CXXFLAGS = -DDATADIR='"$(datadir)"' -Idispatcher -Ioracle -Iradius -Icbilling -I/usr/include/libxml2 -I/usr/local/include/libmemcached $(ORACLE_OCI_CFLAGS)

bin_PROGRAMS = isg_cached
# find *  -path testing -prune -or -name \*.cpp -printf "%p " -or -name \*.h -printf "%p " && echo
isg_cached_SOURCES = cbilling/session.cpp cbilling/service.cpp cbilling/balance.cpp cbilling/cbilling.cpp dispatcher/dis_oracle.cpp dispatcher/dispatcher.cpp dispatcher/dis_coa.cpp isg_cached.cpp listener.cpp oracle/billing.cpp radius/dictionary.cpp radius/radpacket.cpp radius/coapacket.cpp radius/radclient.cpp request_params.cpp utils.cpp cbilling/service.h cbilling/session.h cbilling/balance.h cbilling/cbilling.h dispatcher/dis_coa.h dispatcher/dis_oracle.h dispatcher/dispatcher.h listener.h oracle/billing.h radius/radpacket.h radius/coapacket.h radius/dictionary.h radius/radclient.h request_params.h utils.h dispatcher/dis_cbilling.h dispatcher/dis_cbilling.cpp
isg_cached_LDADD = -locci -lxml2 -lmemcached -lssl -lpthread $(ORACLE_OCI_LDFLAGS)
# isg_cached_DATA = radius/dictionary
isg_cacheddir = $(datadir)/isg_cached/radius

	
