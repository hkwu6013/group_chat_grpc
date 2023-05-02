export PKG_CONFIG_PATH := /home/hkwu6013/.local/lib/pkgconfig:\
													$(PKG_CONFIG_PATH)

CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++14
LDFLAGS += -L/usr/local/lib `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

BUILD_PATH = ../build/
PROTOS_PATH = ../protos/
PROTOC = protoc


all: ../build/group_chat_server ../build/group_chat_client

../build/group_chat_server: ../build/group_chat.pb.o ../build/group_chat.grpc.pb.o ../build/group_chat_server.o
	$(CXX) $^ $(LDFLAGS) -o $@

../build/group_chat_server.o: group_chat_server.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

../build/group_chat_client: ../build/group_chat.pb.o ../build/group_chat.grpc.pb.o ../build/group_chat_client.o
	$(CXX) $^ $(LDFLAGS) -o $@

../build/group_chat_client.o: group_chat_client.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

../build/group_chat.pb.o: ../build/group_chat.pb.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

../build/group_chat.grpc.pb.o: ../build/group_chat.grpc.pb.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

#group_chat.pb.o: group_chat.pb.cc

#group_chat.grpc.pb.o: group_chat.grpc.pb.cc

../build/%.pb.cc: ../protos/%.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=$(BUILD_PATH) $<

../build/%.grpc.pb.cc: ../protos/%.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=$(BUILD_PATH) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $<

clean:
	rm -f $(BUILD_PATH)*