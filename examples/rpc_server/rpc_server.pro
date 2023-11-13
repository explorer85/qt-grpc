PROTOS_SERVER = $$_PRO_FILE_PWD_/../protos/binarychat.proto
PROTOS_STRUCTS = $$_PRO_FILE_PWD_/../protos/commontypes.proto
include(../../qt-grpc/proto_compile.pri)

QT += quick
QT += concurrent


DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -std=c++17



SOURCES += \
  ../common/MessagesModel.cpp \
  Backend.cpp \
  main.cpp

HEADERS += \
    ../common/MessagesModel.h \
  #../protos/AsyncChatServer.h \
  Backend.h \

DISTFILES += \
    main.qml

RESOURCES += \
    qml.qrc





