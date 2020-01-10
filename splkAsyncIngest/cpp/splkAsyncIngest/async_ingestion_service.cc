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

#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

bool gEnableOutSink = false;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void sendCurl( std::string url, std::string data, std::string token) {
    //printf("hi, sendCurl2, data is %s \n", data.c_str());

    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    //string data = "##IgnoreRawDataQueue_234##108";
    curl = curl_easy_init();
    if(curl) {
        //printf("url is %s \n", url.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        struct curl_slist *headers=NULL; /* init to NULL is important */

        std::string auth = "Authorization: Splunk " + token;

        headers = curl_slist_append(headers, auth.c_str());
        //headers = curl_slist_append(headers, "X-silly-content: yes");

        /* pass our list of custom made headers */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        //std::cout << readBuffer << std::endl;
    }
}

std::mutex m_mutex;

class SplkInst {
public:
    SplkInst() {
        url = "";
        token = "";
    }

    std::string url;
    std::string token;
};

SplkInst gInstData[100];
int g_totalInsts = 10;

class ServerImpl final {
 public:
  ~ServerImpl() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
/*
    http://10.202.17.42:8000	c0ea298e-2181-4138-addb-5d2a60841b33
            http://10.202.16.24:8000	36b54446-b59d-416b-9215-bb1acd4f9478

    http://10.202.22.77:8000	be3f39bc-91af-4212-8681-900c932e2269

            http://10.202.20.111:8000	260b35d0-d39f-49c1-9246-55563d035b71

    http://10.202.18.223:8000	ef735913-0070-46cf-8c28-2d25e9f8c21c

            http://10.202.20.1:8000	51d0d14e-b5f5-4a1f-b34b-401b197b7e23

    http://10.202.23.250:8000	9ae4f265-b5f3-46a2-958c-b71d0ac95f8a
            http://10.202.21.112:8000	bbc77c2b-bcd3-46a9-ae12-8509e66dd61d
*/
  void Run() {
    std::string server_address("0.0.0.0:50051");
    gInstData[0].url = "http://10.202.22.52:8088/services/collector/raw?channel=93479C4A";
    gInstData[0].token = "bfc8d5e3-6ce6-4498-b0eb-e4612df8ea7b";

    gInstData[1].url = "http://10.202.19.87:8088/services/collector/raw?channel=93479C4A";
    gInstData[1].token = "7aa8db3d-57eb-4f37-bf39-27f8a19e6372";

      gInstData[2].url = "http://10.202.17.42::8088/services/collector/raw?channel=93479C4A";
      gInstData[2].token = "c0ea298e-2181-4138-addb-5d2a60841b33";

      gInstData[3].url = "http://10.202.16.24:8088/services/collector/raw?channel=93479C4A";
      gInstData[3].token = "36b54446-b59d-416b-9215-bb1acd4f9478";

      gInstData[4].url = "http://10.202.22.77:8088/services/collector/raw?channel=93479C4A";
      gInstData[4].token = "be3f39bc-91af-4212-8681-900c932e2269";

      gInstData[5].url = "http://10.202.20.111:8088/services/collector/raw?channel=93479C4A";
      gInstData[5].token = "b260b35d0-d39f-49c1-9246-55563d035b71";

      gInstData[6].url = "http://10.202.18.223:8088/services/collector/raw?channel=93479C4A";
      gInstData[6].token = "ef735913-0070-46cf-8c28-2d25e9f8c21c";

      gInstData[7].url = "http://10.202.20.1:8088/services/collector/raw?channel=93479C4A";
      gInstData[7].token = "51d0d14e-b5f5-4a1f-b34b-401b197b7e23";

      gInstData[8].url = "http://10.202.23.250:8088/services/collector/raw?channel=93479C4A";
      gInstData[8].token = "9ae4f265-b5f3-46a2-958c-b71d0ac95f8a";

      gInstData[9].url = "http://10.202.21.112:8088/services/collector/raw?channel=93479C4A";
      gInstData[9].token = "bbc77c2b-bcd3-46a9-ae12-8509e66dd61d";


    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.

    //sendCurl("http://localhost:8088/services/collector/raw?channel=93479C4A", "replyMsg.c_str()");
    sendCurl(gInstData[0].url, "replyMsg.c_str()",gInstData[0].token);

    for(int i=0;i<10;i++) {
      new std::thread( &ServerImpl::HandleRpcs, this,i);
    }

    // Sleep forever
    std::this_thread::sleep_for(std::chrono::seconds(20000000000));
    //HandleRpcs();
  }

public:
  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(Greeter::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      // Invoke the serving logic right away.
      Proceed(0);
    }

    void Proceed(int inst) {
        //gInstData[0].url = "http://10.202.22.52:8088/services/collector/raw?channel=93479C4A";

      if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);

        // The actual processing.
        std::string replyMsg = "Instance " + std::to_string(inst) + " : ";
        //replyMsg.append(std:string.);
        replyMsg.append(request_.name());
        reply_.set_message(replyMsg);
        //m_mutex.lock();
        //sendCurl("http://localhost:8088/services/collector/raw?channel=93479C4A", replyMsg.c_str());
        if(gEnableOutSink) {
            //std::cerr << "outsink enabled" << std::endl;
            sendCurl(gInstData[inst].url, replyMsg.c_str(), gInstData[inst].token);
        }else{
            //std::cerr << "outsink disabled" << std::endl;
        }



        //m_mutex.unlock();

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
      } else {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

   public:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    Greeter::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue* cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;

    // What we get from the client.
    HelloRequest request_;
    // What we send back to the client.
    HelloReply reply_;

    // The means to get back to the client.
    ServerAsyncResponseWriter<HelloReply> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };



  // This can be run in multiple threads if needed.
  void HandleRpcs(int channel) {

    std::cout << "bar.channel is " << channel << std::endl;

    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    void* tag;  // uniquely identifies a request.
    bool ok;

    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData*>(tag)->Proceed(channel);


        //gInstData[0].url.append("00");
      //m_mutex.lock();
      //std::cerr << "bar3.channel processed : " << channel << std::endl << std::flush;
      //sendCurl()
      //m_mutex.unlock();
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  Greeter::AsyncService service_;
  std::unique_ptr<Server> server_;
};



int main(int argc, char** argv) {

    if(argc >1)
        gEnableOutSink = true;

  std::cout << "gEnableOutSink is " << gEnableOutSink << std::endl;

  ServerImpl server;
  server.Run();

  return 0;
}
