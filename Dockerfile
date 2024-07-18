FROM debian:bookworm

# Install necessary tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update
RUN apt-get install -y vim

# This gives us the 'column' command
RUN apt-get install -y bsdextrautils
RUN apt-get install -y time
RUN apt-get install -y man-db
RUN apt-get install -y checksec

RUN printf "set textwidth=80\nsyn on\n" > /root/.vimrc

# Set the working directory in the container
WORKDIR /workspace

# Command to keep the container running
CMD ["/bin/bash"]
