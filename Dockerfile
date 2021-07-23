FROM ubuntu:20.10

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y git wget automake autoconf clang llvm lld libtool libgmp-dev make libc++-dev libc++abi-dev g++ libclang-dev

RUN mkdir /isl
WORKDIR /isl

RUN git clone https://github.com/patacca/isl . && git checkout polly-generator

RUN ./autogen.sh && ./configure --with-clang=system && make include/isl/isl-noexceptions.h

ENTRYPOINT /bin/bash
