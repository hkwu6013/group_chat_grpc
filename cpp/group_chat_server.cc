#include <iostream>
#include <mutex>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "../build/group_chat.grpc.pb.h"


using grpc::Status;
using grpc::ServerReaderWriter;
using grpc::ServerContext;
using group_chat::GroupChat;
using group_chat::Message;

class GroupChatImpl final: public GroupChat::Service{
 public: 
  Status StartChat(ServerContext context,
                   ServerReaderWriter<Message, Message>* stream){
    {
      // register the new user
      std::unique_lock<std::mutex> lock(mu_);
      streams.push_back(stream);
    }
    Message msg;
    
    while (stream->Read(&msg)){
      // broadcast
      for (auto s: streams){
        s->Write(msg);
      }
    }
    // remove the current stream
    streams.erase(std::remove(streams.begin(), streams.end(), stream), 
                  streams.end());

    return Status::OK;
  }
 private:
  std::vector<ServerReaderWriter<Message, Message>*> streams;
  std::mutex mu_;
};