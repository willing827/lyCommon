######################################################################
# Automatically generated by qmake (2.01a) ?? ?? 23 19:27:45 2014
######################################################################

TEMPLATE = lib
TARGET = render-gdi
CONFIG(x64){
TARGET = $$TARGET"64"
}
!LIB_ALL:!COM_LIB{
	RC_FILE += render-gdi.rc
	CONFIG += dll
}
else{
	CONFIG += staticlib
}

DEPENDPATH += .
INCLUDEPATH += . \
			   ../../soui/include \
			   ../../utilities/include \

dir = ../..
include($$dir/common.pri)

CONFIG(debug,debug|release){
	LIBS += utilitiesd.lib
}
else{
	LIBS += utilities.lib
}

PRECOMPILED_HEADER = stdafx.h

# Input
HEADERS += GradientFillHelper.h render-gdi.h
SOURCES += GradientFillHelper.cpp render-gdi.cpp

