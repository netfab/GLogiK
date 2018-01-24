
lib_LTLIBRARIES += libGLogiKShared.la

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
						libGLogiKUtils.la

libGLogiKShared_la_SOURCES = \
	%D%/keyEvent.h \
	%D%/macrosBanks.cpp \
	%D%/macrosBanks.h \
	%D%/deviceConfigurationFile.cpp \
	%D%/deviceConfigurationFile.h \
	%D%/deviceProperties.cpp \
	%D%/deviceProperties.h \
	%D%/glogik.cpp \
	%D%/glogik.h \
	%D%/sessionManager.cpp \
	%D%/sessionManager.h
