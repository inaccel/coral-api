CORE = src/main/c
CORE_SHARED = $(CORE)/shared
CORE_STATIC = $(CORE)/static
GRPC = grpc

CORE_SHARED_SRCS = $(shell find $(CORE_SHARED) -name *.c)
CORE_SHARED_SRCS += $(shell find $(CORE_SHARED) -name *.cpp)
CORE_STATIC_SRCS = $(shell find $(CORE_STATIC) -name *.c)
CORE_STATIC_SRCS += $(shell find $(CORE_STATIC) -name *.cpp)

CORE_SHARED_OBJS = $(filter-out %.cpp, $(CORE_SHARED_SRCS:.c=.o))
CORE_SHARED_OBJS += $(filter-out %.c, $(CORE_SHARED_SRCS:.cpp=.o))
CORE_STATIC_OBJS = $(filter-out %.cpp, $(CORE_STATIC_SRCS:.c=.o))
CORE_STATIC_OBJS += $(filter-out %.c, $(CORE_STATIC_SRCS:.cpp=.o))

CPPFLAGS = -I $(CORE)/headers -I $(GRPC)

FLAGS = -fPIC -fvisibility=hidden -O3 -Wall
CFLAGS = $(FLAGS)
CXXFLAGS = $(FLAGS) -fvisibility-inlines-hidden -std=c++11

LDFLAGS = -shared -static-libgcc -static-libstdc++
LDLIBS = -Wl,-Bdynamic -ldl -lpthread -Wl,-Bstatic `pkg-config --libs --static grpc++ protobuf` -Wl,--exclude-libs,ALL

all: libcoral-api.a libcoral-api.so

lib%.a: lib%.so.o $(CORE_STATIC_OBJS)
	$(AR) $(ARFLAGS) $@ $^

lib%.so: $(GRPC)/coral.pb.o $(GRPC)/coral.grpc.pb.o $(CORE_SHARED_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@rm -f $(GRPC)/coral.pb.h $(GRPC)/coral.grpc.pb.h

lib%.so.o: lib%.so
	cd $(@D) && $(LD) -b binary -o $(@F) -r $(<F)

%.grpc.pb.cc %.grpc.pb.h: %.proto
	protoc -I $(GRPC) --grpc_out=$(GRPC) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $<

%.pb.cc %.pb.h: %.proto
	protoc -I $(GRPC) --cpp_out=$(GRPC) $<

clean:
	@rm -f libcoral-api.a libcoral-api.so
