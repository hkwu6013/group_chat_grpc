$(shell export PKG_CONFIG_PATH := $(HOME)/.local/lib/pkgconfig:$(PKG_CONFIG_PATH))
$(shell mkdir -p ./generated)
$(shell mkdir -p ./bin)

# specify compiler
CXX = g++
# specify compiler flags

CPPFLAGS = `pkg-config --cflags protobuf grpc`

CXXFLAGS = -std=c++14
# additional flags for linking
LDFLAGS = -L/usr/local/lib `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

# specify the Protocol Buffers compiler
PROTOC = protoc
# specify the gRPC plugin for the Protocol Buffers compiler
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

# specify the path to your .proto files
PROTO_PATH = ./protos
# specify the path to put the generated files
CPP_OUT_PATH = ./generated
# specify the path to put the binary files
BINARY_PATH = ./bin

# specify your source files
SERVER_SRCS = src/group_chat_server.cpp
CLIENT_SRCS = src/group_chat_client.cpp


# specify your generated .cc files
GEN_SRCS = $(wildcard $(CPP_OUT_PATH)/*.cc)

# specify the corresponding object files
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o) $(CPP_OUT_PATH)/group_chat.pb.o $(CPP_OUT_PATH)/group_chat.grpc.pb.o
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o) $(CPP_OUT_PATH)/group_chat.pb.o $(CPP_OUT_PATH)/group_chat.grpc.pb.o

# specify your .proto files
PROTOS = $(wildcard $(PROTO_PATH)/*.proto)
# specify the corresponding .pb.h, .pb.cc, .grpc.pb.h, .grpc.pb.cc files
# PROTO_GEN = $(patsubst $(PROTO_PATH)/%.proto,$(CPP_OUT_PATH)/%.grpc.pb.h $(CPP_OUT_PATH)/%.grpc.pb.cc $(CPP_OUT_PATH)/%.pb.h $(CPP_OUT_PATH)/%.pb.cc, $(PROTOS))
PROTO_GEN := $(foreach proto,$(PROTOS),$(CPP_OUT_PATH)/$(basename $(notdir $(proto))).grpc.pb.h $(CPP_OUT_PATH)/$(basename $(notdir $(proto))).grpc.pb.cc $(CPP_OUT_PATH)/$(basename $(notdir $(proto))).pb.h $(CPP_OUT_PATH)/$(basename $(notdir $(proto))).pb.cc)

$(CPP_OUT_PATH)/%.grpc.pb.h $(CPP_OUT_PATH)/%.grpc.pb.cc $(CPP_OUT_PATH)/%.pb.h $(CPP_OUT_PATH)/%.pb.cc : $(PROTO_PATH)/%.proto
	$(PROTOC) -I $(PROTO_PATH) --grpc_out=$(CPP_OUT_PATH) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<
	$(PROTOC) -I $(PROTO_PATH) --cpp_out=$(CPP_OUT_PATH) $<

# specify the target file 
SERVER_TARGET = $(BINARY_PATH)/group_chat_server
CLIENT_TARGET = $(BINARY_PATH)/group_chat_client

# default target
all: $(PROTO_GEN) $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(CPP_OUT_PATH)/%.o : $(CPP_OUT_PATH)/%.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(SERVER_TARGET) $(CLIENT_TARGET) $(PROTO_GEN)
	rmdir $(BINARY_PATH) $(CPP_OUT_PATH)
