win32:INCLUDEPATH += "c:\Program Files\Mega-Nerd\libsndfile\include"
win32:LIBS += "c:\Program Files\Mega-Nerd\libsndfile\libsndfile-1.lib" \
    libfftw3-3.lib
win32 { 
    LIBS += user32.lib \
        dbghelp.lib
    CONFIG(debug, debug|release):LIBS += qwtd5.lib
    else:LIBS += qwt5.lib
}
else {
    LIBS += -lqwt \
        -lsndfile \
        -lfftw3 \
        -lm
    INCLUDEPATH += /usr/include/qwt5
}
TARGET = wavplot
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    paramsdialog.cpp
HEADERS += mainwindow.h \
    paramsdialog.h
FORMS += mainwindow.ui \
    paramsdialog.ui
