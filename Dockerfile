FROM sourcemation/gcc-16

RUN apt-get update
RUN apt-get install -y cmake
RUN apt-get install -y ninja-build 
RUN curl -L https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64 -o /usr/local/bin/bazel \
    && chmod +x /usr/local/bin/bazel

CMD ["/bin/bash"]