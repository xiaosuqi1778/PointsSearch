# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
TARGET = PointsSearch
DESTDIR = ./x64/Debug
CONFIG += debug
LIBS += -L"$(SolutionDir)ext/lib/$(Configuration)" \
    -lgdald
DEPENDPATH += .
MOC_DIR += GeneratedFiles/$(ConfigurationName)
OBJECTS_DIR += debug
UI_DIR += GeneratedFiles
RCC_DIR += GeneratedFiles
include(PointsSearch.pri)
