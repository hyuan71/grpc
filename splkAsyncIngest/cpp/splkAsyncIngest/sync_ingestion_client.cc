/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  std::string ip = "localhost";
  std::string repeatStr="100";

  if(argc >1)
    ip = argv[1];
  if(argc>2)
     repeatStr = argv[2];

  int repeatInt = atoi(repeatStr.c_str());

  std::string des = ip + ":50051";

  std::cout << "des is " << des << std::endl;

  GreeterClient greeter(grpc::CreateChannel(
      des.c_str(), grpc::InsecureChannelCredentials()));
  std::string user("world");
    auto start = std::chrono::high_resolution_clock::now();

  for(long long i=1;i<=repeatInt;i++) {
    std::string reply = greeter.SayHello(user);
    if(i % 100 == 0) {
      //std::cout << " counterSuc is " << counterSuc << ", counterFail is " << counterFail << std::endl;
      //clock_t time2 = clock() - time;

      //long ms = (double)time2 / CLOCKS_PER_SEC * 1000*100;
      //int kbps = (nMsgKB * 4  * 1000 / ms) ;

      //fprintf(stderr, "yh101.03: time taken : %ld ms, eps is %lld \n" , ms, 1000* i / ms);

        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "Elapsed time: " << elapsed.count() << " s\n";

      //fprintf(stderr, "yh101: kbps is : %d \n" , kbps);
    }
  }


  //std::cout << "Greeter received: " << reply << std::endl;

  return 0;
}
