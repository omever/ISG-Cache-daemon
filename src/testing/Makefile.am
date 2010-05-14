AM_CXXFLAGS = -DDATADIR='"$(datadir)"' -I.. -I../dispatcher -I../oracle -I../radius $(ORACLE_OCI_CFLAGS)
ACLOCAL_AMFLAGS = -I m4

# bin_PROGRAMS = cachedbillingtest
bin_PROGRAMS = request_params_access_op_test
# find * -name \*.h -or -name \*.cpp | xargs echo
#cachedbillingtest_SOURCES = cachedbillingtest.cpp ../oracle/billing.cpp ../oracle/billing.h ../radius/radpacket.h ../radius/radpacket.cpp ../radius/dictionary.cpp ../radius/dictionary.h ../dispatcher/cbilling.cpp ../dispatcher/cbilling.h ../utils.cpp ../utils.h
#cachedbillingtest_LDADD = -locci -lclntsh -lxml2 -lmemcached -lssl -lpthread $(ORACLE_OCI_LDFLAGS)

request_params_access_op_test_SOURCES = request_params_access_op_test.cpp ../request_params.cpp
	
