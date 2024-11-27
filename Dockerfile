FROM ubuntu:22.04

WORKDIR /app

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    clang \
    clang-format \
    make \
    sudo \
    googletest \
    libgtest-dev \
    cppcheck \
    ccache \
    && rm -rf /var/lib/apt/lists/*

COPY . /app

RUN  cd /app && make all && make -j 8 .

CMD ["bash", "-c", "cd /app && make -j 8; ./bin/afw_test"]
