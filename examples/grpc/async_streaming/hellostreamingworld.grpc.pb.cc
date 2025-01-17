// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: hellostreamingworld.proto

#include "hellostreamingworld.pb.h"
#include "hellostreamingworld.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace hellostreamingworld {

static const char* MultiGreeter_method_names[] = {
  "/hellostreamingworld.MultiGreeter/sayHello",
};

std::unique_ptr< MultiGreeter::Stub> MultiGreeter::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< MultiGreeter::Stub> stub(new MultiGreeter::Stub(channel));
  return stub;
}

MultiGreeter::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_sayHello_(MultiGreeter_method_names[0], ::grpc::internal::RpcMethod::SERVER_STREAMING, channel)
  {}

::grpc::ClientReader< ::hellostreamingworld::HelloReply>* MultiGreeter::Stub::sayHelloRaw(::grpc::ClientContext* context, const ::hellostreamingworld::HelloRequest& request) {
  return ::grpc::internal::ClientReaderFactory< ::hellostreamingworld::HelloReply>::Create(channel_.get(), rpcmethod_sayHello_, context, request);
}

void MultiGreeter::Stub::experimental_async::sayHello(::grpc::ClientContext* context, ::hellostreamingworld::HelloRequest* request, ::grpc::experimental::ClientReadReactor< ::hellostreamingworld::HelloReply>* reactor) {
  ::grpc::internal::ClientCallbackReaderFactory< ::hellostreamingworld::HelloReply>::Create(stub_->channel_.get(), stub_->rpcmethod_sayHello_, context, request, reactor);
}

::grpc::ClientAsyncReader< ::hellostreamingworld::HelloReply>* MultiGreeter::Stub::AsyncsayHelloRaw(::grpc::ClientContext* context, const ::hellostreamingworld::HelloRequest& request, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::hellostreamingworld::HelloReply>::Create(channel_.get(), cq, rpcmethod_sayHello_, context, request, true, tag);
}

::grpc::ClientAsyncReader< ::hellostreamingworld::HelloReply>* MultiGreeter::Stub::PrepareAsyncsayHelloRaw(::grpc::ClientContext* context, const ::hellostreamingworld::HelloRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::hellostreamingworld::HelloReply>::Create(channel_.get(), cq, rpcmethod_sayHello_, context, request, false, nullptr);
}

MultiGreeter::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MultiGreeter_method_names[0],
      ::grpc::internal::RpcMethod::SERVER_STREAMING,
      new ::grpc::internal::ServerStreamingHandler< MultiGreeter::Service, ::hellostreamingworld::HelloRequest, ::hellostreamingworld::HelloReply>(
          std::mem_fn(&MultiGreeter::Service::sayHello), this)));
}

MultiGreeter::Service::~Service() {
}

::grpc::Status MultiGreeter::Service::sayHello(::grpc::ServerContext* context, const ::hellostreamingworld::HelloRequest* request, ::grpc::ServerWriter< ::hellostreamingworld::HelloReply>* writer) {
  (void) context;
  (void) request;
  (void) writer;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace hellostreamingworld

