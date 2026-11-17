# Reproducible build + test environment for the ricc:: STL implementations.
#
#   docker build -t stl-tests .                                         # GCC
#   docker build -t stl-tests-clang --build-arg CXX_COMPILER=clang++ .  # Clang
#   docker run --rm stl-tests                                           # run tests

FROM ubuntu:24.04

ARG CXX_COMPILER=g++

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        clang \
        libclang-rt-18-dev \
        cmake \
        ninja-build \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY . .

ENV CXX=${CXX_COMPILER}

RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
 && cmake --build build

CMD ["ctest", "--test-dir", "build", "--output-on-failure"]
