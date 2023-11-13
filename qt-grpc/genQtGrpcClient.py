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

#include "client/AsyncGenericGrpcClient.h"
#include "client/OneToMClientCall.h"
#include <QMetaType>
#include <QObject>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "%(include)s.grpc.pb.h"
#pragma GCC diagnostic pop
""" % {"include": outfname}


def genClassHeader():
    return """
class Async%(service)sClient : public AsyncGenericGrpcClient<%(namespace)s::%(service)s>
{
public:
    Async%(service)sClient(std::string server_address, std::string clientId)
        : AsyncGenericGrpcClient(server_address, clientId)
    {
        run();
    }

""" % {"namespace": pckg, "service": services[0].name}


def genPublic():
    public = """ """
    for s in services:
        for rpc in s.rpc_list:
            public += """
    void %(rpcname)s(const %(paramnamespace)s::%(param)s& request, std::function<void(%(returnnamespace)s::%(return)s)> responseCallback) {
        auto call = std::make_shared<%(rpcname)sCallData>(responseCallback, this);

        auto& context = call->context();
        context.set_wait_for_ready(true);
        context.AddMetadata(kClientidStr, clientId_);
        call->reader_ = stub_->Async%(rpcname)s(&call->context(),
                                                 request,
                                                 &cq,
                                                 call.get());
       addClientCall(call);
     }""" % {"rpcname": rpc.rpcname, "paramnamespace": rpc.paramnamespace, "param": rpc.paramtype, "returnnamespace": rpc.returnnamespace, "return": rpc.returntype}
    public += "\n"
    return public



def genCallData():
    structs = """
private:
        """
    for s in services:
        for rpc in s.rpc_list:
            structs += """
    struct %(rpcname)sCallData : public OneToMClientCall< %(returnnamespace)s::%(return)s, Async%(service)sClient >
    {
        %(rpcname)sCallData(std::function<void(%(returnnamespace)s::%(return)s)> responseCallback, Async%(service)sClient *client) : OneToMClientCall<%(returnnamespace)s::%(return)s, Async%(service)sClient >(responseCallback, client) {}
    };
""" % {"package": pckg, "service": s.name, "rpcname": rpc.rpcname, "returnnamespace": rpc.returnnamespace, "return": rpc.returntype}
    return structs




def genClassEndBrace():
    return """ }; """


filename = "%(fname)s.qgrpc.client" % {"fname": outfname}
out = os.path.join(outpath, filename+".h")

if(sys.argv[2] == "--get-outfile"):
    print(filename)
else:
    f = open(out, 'w')
    f.write(genHeader())
    f.write(genClassHeader())
    f.write(genPublic())
    f.write(genCallData())

    f.write(genClassEndBrace())

    f.close()
