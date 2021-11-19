FROM centos:7
LABEL maintainer=InAccel

RUN yum install --assumeyes \
        gcc-c++ \
        git \
        glibc-static \
        libstdc++-static \
        make \
        wget \
        which \
 && yum clean all

ARG CMAKE_VERSION=3.21.4
RUN wget --output-document=cmake.sh https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
 && sh cmake.sh --prefix=/usr --skip-license \
 && rm --force cmake.sh

ARG OPENSSL_VERSION=1.1.1
RUN wget --directory-prefix=/usr/local/openssl --no-check-certificate https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz \
 && cd /usr/local/openssl \
 && tar --extract --file=openssl-${OPENSSL_VERSION}.tar.gz \
 && openssl-${OPENSSL_VERSION}/config \
 && make install \
 && rm --force --recursive /usr/local/openssl

ENV CC=cc
ENV CXX=c++

ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

ARG GRPC_VERSION=1.33.2
RUN git clone --branch=v${GRPC_VERSION} --depth=1 --recursive --single-branch https://github.com/grpc/grpc.git /usr/local/grpc \
 && cd /usr/local/grpc \
 && mkdir --parents cmake/build \
 && cd cmake/build \
 && cmake ../.. -DCMAKE_BUILD_TYPE=Release -DgRPC_SSL_PROVIDER=package \
 && make --jobs=`nproc` install \
 && rm --force --recursive /usr/local/grpc
