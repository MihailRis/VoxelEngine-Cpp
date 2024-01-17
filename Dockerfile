# Build docker container: docker build -t voxel-engine .
# Build project: docker run --rm -it -v$(pwd):/project voxel-engine bash -c "cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && cmake --build build"
# Run project in docker: docker run --rm -it -v$(pwd):/project -v/tmp/.X11-unix:/tmp/.X11-unix -v${XAUTHORITY}:/home/user/.Xauthority:ro -eDISPLAY --network=host voxel-engine ./build/VoxelEngine

FROM debian:bullseye-slim
LABEL Description="Docker container for building VoxelEngine for Linux"

# Install dependencies
RUN apt-get update && apt-get install --no-install-recommends -y \
    git \
    g++ \
    make \
    cmake \
    xauth \
    gdb \
    gdbserver \
    libglfw3-dev \
    libglfw3 \
    libglew-dev \
    libglm-dev \
    libpng-dev \
    libopenal-dev \
    libluajit-5.1-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# CMake missing LUA_INCLUDE_DIR and LUA_LIBRARIES fix:
RUN ln -s /usr/lib/x86_64-linux-gnu/libluajit-5.1.a /usr/lib/x86_64-linux-gnu/liblua5.1.a \
    && ln -s /usr/include/luajit-2.1 /usr/include/lua

# Install LuaJIT:
RUN git clone https://luajit.org/git/luajit.git \
    && cd luajit \
    && make && make install INSTALL_INC=/usr/include/lua \
    && cd .. && rm -rf luajit

# Create default user, due to:
#   - Build and test artifacts have user permissions and not root permissions
#   - Don't give root privileges from host to containers (security)
ARG USER=user
ARG UID=1000
ARG GID=1000
RUN useradd -m ${USER} --uid=${UID}
USER ${UID}:${GID}

# Project workspace
WORKDIR /project
