FROM debian:bookworm

# Install necessary tools and dependencies
RUN apt-get update
RUN apt-get install -y build-essential
RUN apt-get install -y gdb
RUN apt-get install -y bsdextrautils # This gives us the 'column' command
RUN apt-get install -y time
RUN apt-get install -y man-db
RUN apt-get install -y checksec

# Install and configure vim
RUN apt-get install -y vim
RUN printf "set textwidth=72\nsyn on\n" > /root/.vimrc

# Set the working directory in the container
WORKDIR /workspace

# Command to keep the container running
CMD ["/bin/bash"]
