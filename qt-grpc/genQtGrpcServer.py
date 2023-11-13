import proto3parser as proto
import os
import sys

protofile = os.path.abspath(sys.argv[1]) #Path to file to parse
outpath = sys.argv[2] #Path to file to be generated
outfname = os.path.splitext(os.path.basename(os.path.basename(protofile)))[0]

pckg, services, messages = proto.parseProtoFile(protofile)
#print(pckg)
#proto.printServices(services)
#print(messages)

def genHeader():
    return """
#pragma once

#include "server/AsyncGenericGrpcServer.h"
#include "server/OneToMCallData.h"
#include <QMetaType>
#include <QObject>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "%(include)s.grpc.pb.h"
#pragma GCC diagnostic pop
""" % {"include": outfname}


def genUsings():
    usings = """
using namespace %(namespace)s;
namespace %(namespace)s_server {

    """ % {"namespace": pckg}
    for s in services:
        for rpc in s.rpc_list:
            usings += """
using Request%(rpcname)sType = decltype(
    std::mem_fn(&%(package)s::%(service)s::AsyncService::Request%(rpcname)s));""" % {"package": pckg, "service": s.name, "rpcname": rpc.rpcname}
    return usings


def genClassHeader():
    return """
class Async%(service)sServer : public QObject, public AsyncGenericGrpcServer<%(service)s>
{
    Q_OBJECT
public:
    Async%(service)sServer(std::string server_address)
        : AsyncGenericGrpcServer(server_address)
    {
        run();
        initCallDatas();
    }

""" % {"service": services[0].name}


def genCallData():
    structs = "    //generated call Datas\n"
    for s in services:
        for rpc in s.rpc_list:
            structs += """
    struct %(rpcname)sCallData
        : public OnetoMCallData<%(service)s, %(paramnamespace)s::%(param)s, %(returnnamespace)s::%(return)s, Request%(rpcname)sType, Async%(service)sServer, %(rpcname)sCallData>
    {
        %(rpcname)sCallData(typename %(service)s::AsyncService *service,
                              ServerCompletionQueue *cq_,
                              Async%(service)sServer *server)
            : OnetoMCallData<%(service)s, %(paramnamespace)s::%(param)s, %(returnnamespace)s::%(return)s, Request%(rpcname)sType, Async%(service)sServer, %(rpcname)sCallData>(
                  service,
                  cq_,
                  std::mem_fn(&%(package)s::%(service)s::AsyncService::Request%(rpcname)s),
                  std::mem_fn(&Async%(service)sServer::newCallData%(rpcname)s),
                  std::mem_fn(&Async%(service)sServer::finishCallData%(rpcname)s),
                  server)

        {}
    };
""" % {"package": pckg, "service": s.name, "rpcname": rpc.rpcname,  "paramnamespace": rpc.paramnamespace, "param": rpc.paramtype, "returnnamespace": rpc.returnnamespace, "return": rpc.returntype}
    return structs


def genPublic():
    public = """
public:
    void initCallDatas()
    {"""
    for s in services:
        for rpc in s.rpc_list:
            public += """
        auto callData%(rpcname)s = std::make_shared<%(rpcname)sCallData>(&service_, cq_.get(), this);
        addCalldata(callData%(rpcname)s); """ % {"rpcname": rpc.rpcname}
    public += "\n    }\n"
    return public

def genSignals():
    signals = "signals:"
    for s in services:
        for rpc in s.rpc_list:
            signals += """
    void newCallData%(rpcname)s(std::shared_ptr<%(rpcname)sCallData> callData);
    void finishCallData%(rpcname)s(std::shared_ptr<%(rpcname)sCallData> callData); """ % {"rpcname": rpc.rpcname}
    signals += "\n};\n}\n"
    return signals


def genQMetatypes():
    metatypes = ""
    for s in services:
        for rpc in s.rpc_list:
            metatypes += """
Q_DECLARE_METATYPE(std::shared_ptr<%(namespace)s_server::Async%(service)sServer::%(rpcname)sCallData>)
static int %(package)s_%(rpcname)sCallDataId = qRegisterMetaType<
    std::shared_ptr<%(namespace)s_server::Async%(service)sServer::%(rpcname)sCallData>>();
static int %(package)s_%(rpcname)sCallDataIdString = qRegisterMetaType<
    std::shared_ptr<%(namespace)s_server::Async%(service)sServer::%(rpcname)sCallData>>(
    "std::shared_ptr<%(rpcname)sCallData>");\n""" % {"service": s.name, "rpcname": rpc.rpcname, "package": pckg, "namespace": pckg}
    return metatypes


filename = "%(fname)s.qgrpc.server" % {"fname": outfname}
out = os.path.join(outpath, filename+".h")

if(sys.argv[2] == "--get-outfile"):
    print(filename)
else:
    f = open(out, 'w')
    f.write(genHeader())
    f.write(genUsings())
    f.write(genClassHeader())
    f.write(genCallData())
    f.write(genPublic())
    f.write(genSignals())
    f.write(genQMetatypes())

    f.close()
