TEMPLATE = lib
TARGET = Utils
QT += network

DEFINES += QWORKBENCH_UTILS_LIBRARY

include(../../qworkbenchlibrary.pri)

SOURCES += \
    reloadpromptutils.cpp \
    settingsutils.cpp \
    filesearch.cpp \
    pathchooser.cpp \
    filewizardpage.cpp \
    filewizarddialog.cpp \
    projectintropage.cpp \
    basevalidatinglineedit.cpp \
    filenamevalidatinglineedit.cpp \
    projectnamevalidatinglineedit.cpp \
    codegeneration.cpp \
    newclasswidget.cpp \
    classnamevalidatinglineedit.cpp \
    linecolumnlabel.cpp \
    fancylineedit.cpp \
    qtcolorbutton.cpp \
    submiteditorwidget.cpp \
    synchronousprocess.cpp

win32:SOURCES += consoleprocess_win.cpp
else:SOURCES += consoleprocess_unix.cpp

HEADERS += \
    utils_global.h \
    reloadpromptutils.h \
    settingsutils.h \
    filesearch.h \
    listutils.h \
    pathchooser.h \
    filewizardpage.h \
    filewizarddialog.h \
    projectintropage.h \
    basevalidatinglineedit.h \
    filenamevalidatinglineedit.h \
    projectnamevalidatinglineedit.h \
    codegeneration.h \
    newclasswidget.h \
    classnamevalidatinglineedit.h \
    linecolumnlabel.h \
    fancylineedit.h \
    qtcolorbutton.h \
    submiteditorwidget.h \
    abstractprocess.h \
    consoleprocess.h \
    synchronousprocess.h

FORMS += filewizardpage.ui \
         projectintropage.ui \
         newclasswidget.ui \
         submiteditorwidget.ui
