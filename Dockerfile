FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest

WORKDIR /app

COPY . .

RUN mkdir build && cd build && cmake .. && make -j$(nproc)

CMD ["./build/wms-userver", "--config", "config/config.yaml"]