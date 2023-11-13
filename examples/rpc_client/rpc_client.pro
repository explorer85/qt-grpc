PROTOS_CLIENT = $$_PRO_FILE_PWD_/../protos/binarychat.proto
PROTOS_STRUCTS = $$_PRO_FILE_PWD_/../protos/commontypes.proto
include(../../qt-grpc/proto_compile.pri)


QT += quick
QT += concurrent
QT += core



DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -std=c++14


HEADERS += \
    ../common/MessagesModel.h \
    ChatBackend.h

SOURCES += \
  ../common/MessagesModel.cpp \
  main.cpp

RESOURCES += \
    qml.qrc

DISTFILES += \
    main.qml















