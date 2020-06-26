QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG(release, debug|release): DLLDESTDIR +=  "$$PWD/build"

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Lua/lua_src/lapi.c \
        Lua/lua_src/lauxlib.c \
        Lua/lua_src/lbaselib.c \
        Lua/lua_src/lbitlib.c \
        Lua/lua_src/lcode.c \
        Lua/lua_src/lcorolib.c \
        Lua/lua_src/lctype.c \
        Lua/lua_src/ldblib.c \
        Lua/lua_src/ldebug.c \
        Lua/lua_src/ldo.c \
        Lua/lua_src/ldump.c \
        Lua/lua_src/lfunc.c \
        Lua/lua_src/lgc.c \
        Lua/lua_src/linit.c \
        Lua/lua_src/liolib.c \
        Lua/lua_src/llex.c \
        Lua/lua_src/lmathlib.c \
        Lua/lua_src/lmem.c \
        Lua/lua_src/loadlib.c \
        Lua/lua_src/lobject.c \
        Lua/lua_src/lopcodes.c \
        Lua/lua_src/loslib.c \
        Lua/lua_src/lparser.c \
        Lua/lua_src/lstate.c \
        Lua/lua_src/lstring.c \
        Lua/lua_src/lstrlib.c \
        Lua/lua_src/ltable.c \
        Lua/lua_src/ltablib.c \
        Lua/lua_src/ltm.c \
        Lua/lua_src/lundump.c \
        Lua/lua_src/lutf8lib.c \
        Lua/lua_src/lvm.c \
        Lua/lua_src/lzio.c \
        main.cpp \
        tags.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Lua/lua_script.h \
    Lua/lua_src/lapi.h \
    Lua/lua_src/lauxlib.h \
    Lua/lua_src/lcode.h \
    Lua/lua_src/lctype.h \
    Lua/lua_src/ldebug.h \
    Lua/lua_src/ldo.h \
    Lua/lua_src/lfunc.h \
    Lua/lua_src/lgc.h \
    Lua/lua_src/llex.h \
    Lua/lua_src/llimits.h \
    Lua/lua_src/lmem.h \
    Lua/lua_src/lobject.h \
    Lua/lua_src/lopcodes.h \
    Lua/lua_src/lparser.h \
    Lua/lua_src/lprefix.h \
    Lua/lua_src/lstate.h \
    Lua/lua_src/lstring.h \
    Lua/lua_src/ltable.h \
    Lua/lua_src/ltm.h \
    Lua/lua_src/lua.h \
    Lua/lua_src/lua.hpp \
    Lua/lua_src/luaconf.h \
    Lua/lua_src/lualib.h \
    Lua/lua_src/lundump.h \
    Lua/lua_src/lvm.h \
    Lua/lua_src/lzio.h \
    tags.h \
    worker.h
