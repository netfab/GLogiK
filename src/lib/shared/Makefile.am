
lib_LTLIBRARIES += libGKShared.la

libGKShared_la_CXXFLAGS = \
						@AM_CXXFLAGS@ \
						@BOOST_CPPFLAGS@ \
						$(SM_CFLAGS)

libGKShared_la_LDFLAGS = \
						@AM_LDFLAGS@ \
						@BOOST_LDFLAGS@

libGKShared_la_LIBADD = \
						$(SM_LIBS) \
						@BOOST_SERIALIZATION_LIB@ \
						@BOOST_SYSTEM_LIB@ \
						@BOOST_FILESYSTEM_LIB@ \
						libGKUtils.la

libGKShared_la_SOURCES = \
	%D%/macrosBanks.cpp \
	%D%/macrosBanks.h \
	%D%/deviceProperties.cpp \
	%D%/deviceProperties.h \
	%D%/glogik.cpp \
	%D%/glogik.h \
	%D%/sessionManager.cpp \
	%D%/sessionManager.h
