#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "../build/group_chat.grpc.pb.h"

using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::Channel;

using group_chat::GroupChat;
using group_chat::Message;


ABSL_FLAG(std::string, user, "YouKnowWho", "Enter your name.");

class GroupChatClient {
 public:
  GroupChatClient(std::shared_ptr<Channel> channel, std::string user)
    : stub_(GroupChat::NewStub(channel)), username(user){
  };
  void Chat(){
    ClientContext context;
    context.AddMetadata("username", username);
    std::shared_ptr<ClientReaderWriter<Message, Message>> stream(
      stub_->StartChat(&context)
    );
    std::thread sender_thread(&GroupChatClient::Sender, this, stream);
    Message msg;
    while (stream->Read(&msg)){
      std::cout << "[" << msg.user() << "]: " <<msg.body() << std::endl;
    }
    sender_thread.join();
    return;
  }
 private:
  void Sender(std::shared_ptr<ClientReaderWriter<Message, Message>> stream){
    std::string input;
    Message msg;
    msg.set_user(username);
    while (true){
      std::getline(std::cin, input);
      if (input == "!quit") break;
      msg.set_body(input);
      stream->Write(msg);
    }
    stream->WritesDone();
    return;
  }
  std::string username;
  std::unique_ptr<GroupChat::Stub> stub_;
};
int main(int argc, char** argv){
  absl::ParseCommandLine(argc, argv);
  std::string username = absl::GetFlag(FLAGS_user);
  std::string server_address("localhost:50051");
  
  std::shared_ptr<Channel> channel = grpc::CreateChannel(
    server_address, grpc::InsecureChannelCredentials());

  // Instantiate the GroupChatClient and start chatting
  GroupChatClient client(channel, username);
  client.Chat();

  return 0;
}