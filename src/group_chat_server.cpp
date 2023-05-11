#include <iostream>
#include <mutex>
#include <vector>
#include <sstream>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "../generated/group_chat.grpc.pb.h"


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
                   ServerReaderWriter<Message, Message>* stream) override {
    // TODO: fetch the client's username from context and add it to the usernames
    auto metadata = context->client_metadata();
    grpc::string_ref usr = metadata.find("username")->second;
    Message msg;
    // TODO: Send a message to clients specifying the other users in the room
    {
      // register the new user stream
      std::unique_lock<std::mutex> lock(mu_);
      streams.push_back(stream);
      usrs.push_back(usr);
      std::cout << "New user(" << usr << ") in the room, # users = "
      << streams.size() << std::endl;


      std::stringstream ss;
      if (usrs.size()!=1){
        ss << "Welcome! You are in the room with: ";
        for (int i = 0; i < usrs.size()-1; i++){
          ss << (i ? ", " : "") << usrs[i];
        }
      }
      else{
        ss << "Welcome! You are the first in the room";
      }
      ss << "!";
      msg.set_user("GROUP CHAT SERVER");
      msg.set_body(ss.str());
      stream -> Write(msg);
      msg.set_body("New user " + std::string(usr.data(), usr.length())
      + " joined the chat.");
      broad_cast(msg, stream);
    }

    while (stream->Read(&msg)){
      std::unique_lock<std::mutex> lock(mu_);

      std::cout << "Received message: " << "\"" 
      << msg.body() << "\" " << "from " << msg.user()
      << ". Broadcasting to all other users ..." << std::endl;
      // broadcast
      broad_cast(msg, stream);
      std::cout << "Broadcast completed!" << std::endl;
    }
    // remove the current stream
    {
      std::unique_lock<std::mutex> lock(mu_);
      streams.erase(std::remove(streams.begin(), streams.end(), stream), 
                    streams.end());
      usrs.erase(std::remove(usrs.begin(), usrs.end(), usr), usrs.end());
      msg.set_user("GROUP CHAT SERVER");
      msg.set_body(std::string(usr.data(), usr.size()) + " left the room.");
      broad_cast(msg, stream);
    }
    std::cout << "User left the room, # of users = "
    << streams.size() << std::endl;
    return Status::OK;
  }
 private:
  // std::vector<std::string> usernames;
  void broad_cast(Message& msg, 
                  ServerReaderWriter<Message, Message>* local_stream){
    for (auto& s: streams){
        if (s == local_stream) continue;
        s->Write(msg);      
    }
  }
  std::vector<ServerReaderWriter<Message, Message>*> streams;
  std::mutex mu_;
  std::vector<grpc::string_ref> usrs;
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