
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
	%D%/macrosBanks.hpp \
	%D%/deviceProperties.cpp \
	%D%/deviceProperties.hpp \
	%D%/glogik.cpp \
	%D%/glogik.hpp \
	%D%/sessionManager.cpp \
	%D%/sessionManager.hpp
