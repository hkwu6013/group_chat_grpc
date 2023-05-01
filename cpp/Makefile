BUILD_PATH = ../build
PROTOS_PATH = ../protos
PROTOC = protoc

all: headers grpc_headers

headers:
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=$(BUILD_PATH) group_chat.proto

grpc_headers:
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=$(BUILD_PATH) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` group_chat.proto