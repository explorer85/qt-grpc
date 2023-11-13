#pragma once

#include <memory>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <grpc++/grpc++.h>
#pragma GCC diagnostic pop
#include "../logging/LoggingCategories.h"

using grpc::ClientContext;
using grpc::ServerCompletionQueue;
using grpc::Status;

class AbstractClientCall : public QObject
{
    Q_OBJECT
public:
    virtual void Proceed(bool ok) = 0;
    ClientContext &context() { return context_; }
signals:
    void newData();

private:
    ClientContext context_;
};
