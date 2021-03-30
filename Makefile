CORE = src/main/c
GRPC = grpc

CORE_SRCS = $(shell find $(CORE) -name *.c)
CORE_SRCS += $(shell find $(CORE) -name *.cpp)

CORE_OBJS = $(filter-out %.cpp, $(CORE_SRCS:.c=.o))
CORE_OBJS += $(filter-out %.c, $(CORE_SRCS:.cpp=.o))

CPPFLAGS = -I $(CORE)/headers -I $(GRPC)

FLAGS = -fPIC -fvisibility=hidden -O3 -Wall
CFLAGS = $(FLAGS)
CXXFLAGS = $(FLAGS) -fvisibility-inlines-hidden -std=c++11

LDFLAGS = -shared -static-libgcc -static-libstdc++
LDLIBS = -Wl,-Bdynamic -ldl -lpthread -Wl,-Bstatic `pkg-config --libs --static grpc++ protobuf` -Wl,--exclude-libs,ALL

all: libcoral-api.so

lib%.so: $(GRPC)/coral.pb.o $(GRPC)/coral.grpc.pb.o $(CORE_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@rm -f $(GRPC)/coral.pb.h $(GRPC)/coral.grpc.pb.h

%.grpc.pb.cc %.grpc.pb.h: %.proto
	protoc -I $(GRPC) --grpc_out=$(GRPC) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $<

%.pb.cc %.pb.h: %.proto
	protoc -I $(GRPC) --cpp_out=$(GRPC) $<

clean:
	@rm -f libcoral-api.so
