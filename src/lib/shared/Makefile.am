
lib_LTLIBRARIES=libGLogiKShared.la

libGLogiKShared_la_CXXFLAGS = \
						@AM_CXXFLAGS@ \
						@BOOST_CPPFLAGS@ \
						$(SM_CFLAGS)

libGLogiKShared_la_LDFLAGS = \
						@AM_LDFLAGS@ \
						@BOOST_LDFLAGS@

libGLogiKShared_la_LIBADD = \
						$(SM_LIBS) \
						@BOOST_SERIALIZATION_LIB@ \
						@BOOST_SYSTEM_LIB@ \
						@BOOST_FILESYSTEM_LIB@ \
						../utils/libGLogiKUtils.la

libGLogiKShared_la_SOURCES = \
	keyEvent.h \
	macrosBanks.cpp macrosBanks.h \
	deviceConfigurationFile.cpp deviceConfigurationFile.h \
	deviceProperties.cpp deviceProperties.h \
	sessionManager.cpp sessionManager.h
