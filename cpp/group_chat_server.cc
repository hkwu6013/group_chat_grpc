#include <iostream>
#include <mutex>
#include <vector>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "../build/group_chat.grpc.pb.h"


using grpc::Status;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerReaderWriter;
using grpc::ServerContext;
using group_chat::GroupChat;
using group_chat::Message;

class GroupChatImpl final: public GroupChat::Service{
 public: 
  Status StartChat(ServerContext* context,
                   ServerReaderWriter<Message, Message>* stream){
    // TODO: fetch the client's username from context and add it to the usernames
    
    // TODO: Send a message to clients specifying the other users in the room
    {
      // register the new user stream
      std::unique_lock<std::mutex> lock(mu_);
      streams.push_back(stream);
      std::cout << "New user in the room, # users = "
      << streams.size() << std::endl;
    }
    Message msg;
    
    while (stream->Read(&msg)){
      std::unique_lock<std::mutex> lock(mu_);

      std::cout << "Received message: " << std::endl << msg.body() 
      << std::endl << "from " << msg.user()
      << ". Broadcasting..." << std::endl;
      // broadcast
      for (auto s: streams){
        if (s == stream) continue;
        s->Write(msg);
      }
    }
    // remove the current stream
    {
      std::unique_lock<std::mutex> lock(mu_);
      streams.erase(std::remove(streams.begin(), streams.end(), stream), 
                    streams.end());
    }
    std::cout << "User left the room, # of users = "
    << streams.size() << std::endl;
    return Status::OK;
  }
 private:
  // std::vector<std::string> usernames;
  std::vector<ServerReaderWriter<Message, Message>*> streams;
  std::mutex mu_;
};
void RunServer(){
  std::string server_address("localhost:50051");
  ServerBuilder builder;
  GroupChatImpl group_chat_service;
  builder.AddListeningPort(
    server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&group_chat_service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on "<<server_address << std::endl;
  server->Wait();
}

int main(){
  RunServer();
  return 0;
}