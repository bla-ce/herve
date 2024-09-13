FROM debian:latest

# Step 2: Install necessary dependencies
RUN apt-get update && \
    apt-get install -y nasm gcc make && \
    rm -rf /var/lib/apt/lists/*

# Step 3: Set the working directory in the container
WORKDIR /app

# Step 4: Copy the source code into the container
COPY examples/ /app
COPY inc/  /app
COPY Makefile /app

# Step 5: Build the application
CMD ["make", "prod"]
