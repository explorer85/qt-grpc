#pragma once

#include <memory>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <grpc++/grpc++.h>
#pragma GCC diagnostic pop
#include "../logging/LoggingCategories.h"

using grpc::Server;
using grpc::ServerAsyncReader;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerAsyncWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;

class AbstractCommonCallData : public QObject
{
    Q_OBJECT
public:
    virtual void Proceed(bool) = 0;
    ServerContext &context() { return ctx_; }

protected:
    ServerContext ctx_;
};
