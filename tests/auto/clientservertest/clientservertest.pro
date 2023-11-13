PROTOS_CLIENT = $$_PRO_FILE_PWD_/../protos/binarychat.proto
PROTOS_SERVER = $$_PRO_FILE_PWD_/../protos/binarychat.proto
include(../../../qt-grpc/proto_compile.pri)


QT = core gui testlib
CONFIG += c++14

TEMPLATE = app
CONFIG += console testcase no_testcase_installs
TARGET = clientservertest



SOURCES    += clientservertest.cpp


