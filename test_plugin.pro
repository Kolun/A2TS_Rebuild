TEMPLATE = lib
TARGET = ts3plugin

HEADERS += \
    include/ts3plugin.h \
	include/parser.h \
    include/ts3_functions.h \
    include/public_rare_definitions.h \
    include/public_errors_rare.h \
    include/public_errors.h \
    include/public_definitions.h \
    include/plugin_events.h \
    include/plugin_definitions.h \
    include/clientlib_publicdefinitions.h

SOURCES += \
    src/ts3plugin.cpp \
    src/parser.cpp
