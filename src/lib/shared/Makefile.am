
lib_LTLIBRARIES += libGKShared.la

libGKShared_la_CXXFLAGS = \
						@AM_CXXFLAGS@ \
						@BOOST_CPPFLAGS@

if WITH_DBUS
libGKShared_la_CXXFLAGS += \
						$(SM_CFLAGS)
endif

libGKShared_la_LDFLAGS = \
						@AM_LDFLAGS@ \
						@BOOST_LDFLAGS@

libGKShared_la_LIBADD = \
						@BOOST_SERIALIZATION_LIB@ \
						libGKUtils.la

if WITH_DBUS
libGKShared_la_LIBADD += \
						$(SM_LIBS)
endif

libGKShared_la_SOURCES = \
	%D%/deviceProperties.cpp \
	%D%/deviceProperties.hpp \
	%D%/deviceConfigurationFile.cpp \
	%D%/deviceConfigurationFile.hpp \
	%D%/GKeysBanksCapability.cpp \
	%D%/GKeysBanksCapability.hpp \
	%D%/GKeysMacro.cpp \
	%D%/GKeysMacro.hpp \
	%D%/glogik.cpp \
	%D%/glogik.hpp

if WITH_DBUS
libGKShared_la_SOURCES += \
	%D%/sessionManager.cpp \
	%D%/sessionManager.hpp
endif

