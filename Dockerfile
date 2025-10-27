# Example Dockerfile fix
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y g++ make
WORKDIR /app
COPY . .

RUN g++ producer.cpp -pthread -lrt -o producer
RUN g++ consumer.cpp -pthread -lrt -o consumer

# Run both in one container, background producer, then consumer
CMD ./producer & ./consumer &


