# Build
# ===================================================

FROM ubuntu:20.04 as build

RUN apt-get -y update \
    && apt-get -y upgrade

# Dependecies -----------------------------------

RUN apt-get install -y g++
RUN apt-get install -y cmake
RUN apt-get install -y git
RUN apt-get install -y curl
RUN apt-get install -y libssl-dev
RUN apt-get install -y libmongoc-1.0-0
RUN apt-get install -y libsasl2-dev
RUN apt-get install -y openssl

# MongoDB Driver -----------------------------------

RUN cd ~ \
    && curl -OL https://github.com/mongodb/mongo-c-driver/releases/download/1.19.0/mongo-c-driver-1.19.0.tar.gz \
    && tar xzf mongo-c-driver-1.19.0.tar.gz \
    && cd mongo-c-driver-1.19.0 \
    && mkdir cmake-build \
    && cd cmake-build \
    && cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF .. \
    && cmake --build . \
    && cmake --build . --target install \
    && cd ~ \
    && rm mongo-c-driver-1.19.0.tar.gz \
    && rm -rf mongo-c-driver-1.19.0

RUN cd ~ \
    && curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.6.5/mongo-cxx-driver-r3.6.5.tar.gz \
    && tar -xzf mongo-cxx-driver-r3.6.5.tar.gz \
    && cd mongo-cxx-driver-r3.6.5/build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build . --target EP_mnmlstc_core \
    && cmake --build . \
    && cmake --build . --target install \
    && cd ~ \
    && rm mongo-cxx-driver-r3.6.5.tar.gz \
    && rm -rf mongo-cxx-driver-r3.6.5

# Curl Client ---------------------------------------

RUN apt-get install -y libcurl4 libcurl4-gnutls-dev
RUN cd ~ \
 && git clone https://github.com/whoshuu/cpr.git \
 && cd cpr \
 && mkdir build && cd build \
 && cmake .. \
 && make \
 && make install \
 && cd ~ && rm -rf cpr

# Other Tools ---------------------------------------

RUN apt-get install -y mc

# SSH ---------------------------------------

# RUN apt-get -y install ssh \
# && systemctl ssh start \
# && systemctl ssh enable \
# && service ssh status

# Compliation ---------------------------------------

ADD ./src/ app/src/
ADD ./docker/* /app/

RUN cd app && make clean && make

# Run
# ===================================================

WORKDIR "/app"
CMD ["./server"]