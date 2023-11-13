# Qt qmake integration with Google Protocol Buffers compiler protoc
#
# To compile protocol buffers with qt qmake, specify PROTOS variable and
# include this file
#
# Example:
# PROTOS = a.proto b.proto
# include(proto_compile.pri)

PROTOS_PB = $$PROTOS_CLIENT $$PROTOS_SERVER $$PROTOS_STRUCTS
PROTOS_GRPC = $$PROTOS_CLIENT $$PROTOS_SERVER
message("Generating protocol buffer classes from .proto files." $$PROTOS)

INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD


QMAKE_CXXFLAGS += -Wno-unused-parameter

! isEmpty(PROTOS_CLIENT) {
    HEADERS += $${PWD}/client/AsyncGenericGrpcClient.h
    HEADERS += $${PWD}/client/AbstractClientCall.h
}

! isEmpty(PROTOS_SERVER) {
    HEADERS += $${PWD}/server/CommonCallData.h
}


GRPC_PATH = /usr/local/lib/grpc-1-14-dev

GRPC_PATH_QMAKE = $$GRPC_PATH
message("Grpc path Is " $$GRPC_PATH)

GRPC_INCLUDE_PATH = $$GRPC_PATH_QMAKE/include
MOC_PATH=$$[QT_INSTALL_BINS]/moc

INCLUDEPATH += $$GRPC_INCLUDE_PATH
LIBS += -L$$GRPC_PATH_QMAKE -lgrpc++ -lgrpc -lprotobuf -lgrpc++_reflection  -lgpr

#protobuf

protobuf_decl.name = protobuf headers
protobuf_decl.input = PROTOS_PB
protobuf_decl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h
protobuf_decl.commands = $$GRPC_PATH_QMAKE/protoc --cpp_out=${QMAKE_FILE_IN_PATH} --proto_path=${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_NAME}
protobuf_decl.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += protobuf_decl

protobuf_impl.name = protobuf sources
protobuf_impl.input = PROTOS_PB
protobuf_impl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h
protobuf_impl.commands = $$escape_expand(\n)
protobuf_impl.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl


#grpc

grpc_decl.name = protobuf headers
grpc_decl.input = PROTOS_GRPC
grpc_decl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.grpc.pb.h
grpc_decl.commands = $$GRPC_PATH_QMAKE/protoc --grpc_out=${QMAKE_FILE_IN_PATH} --plugin=protoc-gen-grpc=$$GRPC_PATH_QMAKE/grpc_cpp_plugin  --proto_path=${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_NAME}
grpc_decl.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += grpc_decl

grpc_impl.name = protobuf sources
grpc_impl.input = PROTOS_GRPC
grpc_impl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.grpc.pb.cc
grpc_impl.depends = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.grpc.pb.h
grpc_impl.commands = $$escape_expand(\n)
grpc_impl.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += grpc_impl


#server headers

! isEmpty(PROTOS_SERVER) {
    QGRPC_PY_PATH = $$PWD/genQtGrpcServer.py

    qgrpc_server_h.name = protobuf headers
    OUT_H=${QMAKE_FILE_BASE}.qgrpc.server.h
    qgrpc_server_h.input = PROTOS_SERVER
    qgrpc_server_h.output = ${QMAKE_FILE_IN_PATH}/$${OUT_H}
    qgrpc_server_h.variable_out = HEADERS
    qgrpc_server_h.commands += python $${QGRPC_PY_PATH} ${QMAKE_FILE_IN} ${QMAKE_FILE_IN_PATH}/
    QMAKE_EXTRA_COMPILERS *= qgrpc_server_h

    for(file, PROTOS_SERVER) {
        message(generate qtfiles   $$PROTOS_SERVER)
        QGRPC_OUT_FILENAME=$$system('python $${QGRPC_PY_PATH} $${file} --get-outfile')
        QGRPC_OUT_H=$$dirname(file)/$${QGRPC_OUT_FILENAME}.h
        QGRPC_OUT_MOC_CPP=moc_$${QGRPC_OUT_FILENAME}.cpp
        MOC_OUT_FILENAME=$${QGRPC_OUT_MOC_CPP}

        #generate and compile moc file
        COMPILER_NAME=QGRPC_server_MOC_GEN_$${QGRPC_OUT_FILENAME}
        $${COMPILER_NAME}.name = protobuf headers
        $${COMPILER_NAME}.input = QGRPC_OUT_H
        $${COMPILER_NAME}.output = $${MOC_OUT_FILENAME}
        $${COMPILER_NAME}.variable_out = SOURCES
        $${COMPILER_NAME}.commands = $${MOC_PATH} $(DEFINES) $${QGRPC_OUT_H} -o $${MOC_OUT_FILENAME}
        QMAKE_EXTRA_COMPILERS *= $${COMPILER_NAME}
    }
}



#client headers

! isEmpty(PROTOS_CLIENT) {
    QGRPC_CLIENT_PY_PATH = $$PWD/genQtGrpcClient.py

    qgrpc_client_h.name = protobuf headers
    OUT_H=${QMAKE_FILE_BASE}.qgrpc.client.h
    qgrpc_client_h.input = PROTOS_CLIENT
    qgrpc_client_h.output = ${QMAKE_FILE_IN_PATH}/$${OUT_H}
    qgrpc_client_h.variable_out = HEADERS
    qgrpc_client_h.commands += python $${QGRPC_CLIENT_PY_PATH} ${QMAKE_FILE_IN} ${QMAKE_FILE_IN_PATH}/
    QMAKE_EXTRA_COMPILERS *= qgrpc_client_h

    for(file, PROTOS_CLIENT) {
        message(generate qtfiles   $$PROTOS_CLIENT)
        QGRPC_OUT_FILENAME=$$system('python $${QGRPC_CLIENT_PY_PATH} $${file} --get-outfile')
        QGRPC_OUT_H=$$dirname(file)/$${QGRPC_OUT_FILENAME}.h
        QGRPC_OUT_MOC_CPP=moc_$${QGRPC_OUT_FILENAME}.cpp
        MOC_OUT_FILENAME=$${QGRPC_OUT_MOC_CPP}

        #generate and compile moc file
        COMPILER_NAME=QGRPC_client_MOC_GEN_$${QGRPC_OUT_FILENAME}
        $${COMPILER_NAME}.name = protobuf headers
        $${COMPILER_NAME}.input = QGRPC_OUT_H
        $${COMPILER_NAME}.output = $${MOC_OUT_FILENAME}
        $${COMPILER_NAME}.variable_out = SOURCES
        $${COMPILER_NAME}.commands = $${MOC_PATH} $(DEFINES) $${QGRPC_OUT_H} -o $${MOC_OUT_FILENAME}
        QMAKE_EXTRA_COMPILERS *= $${COMPILER_NAME}
    }
}




message("Generating protocol buffer classes from .proto files. END")
