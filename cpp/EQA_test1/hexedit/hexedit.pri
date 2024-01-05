
INCLUDEPATH += $$PWD

SOURCES += $$PWD/chunks.cpp \
    $$PWD/dialogshowmodulechecksum.cpp
SOURCES += $$PWD/commands.cpp
SOURCES += $$PWD/optionsdialog.cpp
SOURCES += $$PWD/qhexedit.cpp
SOURCES += $$PWD/searchdialog.cpp

HEADERS += $$PWD/chunks.h \
    $$PWD/dialogshowmodulechecksum.h
HEADERS += $$PWD/commands.h
HEADERS += $$PWD/optionsdialog.h
HEADERS += $$PWD/qhexedit.h
HEADERS += $$PWD/searchdialog.h

FORMS   += $$PWD/optionsdialog.ui \
    $$PWD/dialogshowmodulechecksum.ui
FORMS   += $$PWD/searchdialog.ui

RESOURCES += $$PWD/qhexedit.qrc
