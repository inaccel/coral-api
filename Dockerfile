FROM centos:7
LABEL maintainer=InAccel

RUN yum install --assumeyes \
        centos-release-scl \
 && yum install --assumeyes \
        devtoolset-11-gcc-c++ \
        git \
        glibc-static \
        libstdc++-static \
        make \
        wget \
        which \
 && yum clean all

SHELL ["scl", "enable", "devtoolset-11"]

ARG CMAKE_VERSION=3.25.0
RUN wget --output-document=cmake.sh https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
 && sh cmake.sh --prefix=/usr --skip-license \
 && rm --force cmake.sh

ARG OPENSSL_VERSION=1.1.1s
RUN wget --directory-prefix=/usr/local/openssl --no-check-certificate https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz \
 && cd /usr/local/openssl \
 && tar --extract --file=openssl-${OPENSSL_VERSION}.tar.gz \
 && openssl-${OPENSSL_VERSION}/config \
 && make install \
 && rm --force --recursive /usr/local/openssl

ENV CC=cc
ENV CXX=c++

ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

ARG GRPC_VERSION=1.51.0
RUN git clone --branch=v${GRPC_VERSION} --depth=1 --recursive https://github.com/grpc/grpc.git /usr/local/grpc \
 && cd /usr/local/grpc \
 && mkdir cmake/build \
 && cd cmake/build \
 && cmake ../.. -DCMAKE_BUILD_TYPE=Release -DgRPC_SSL_PROVIDER=package \
 && make --jobs=`nproc` install \
 && rm --force --recursive /usr/local/grpc

ENTRYPOINT ["scl", "enable", "devtoolset-11", "--", "make"]
