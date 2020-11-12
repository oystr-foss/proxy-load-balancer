FROM alpine:latest

RUN apk update
RUN apk add vim bash git g++ make cmake boost-dev curl curl-dev jsoncpp-dev libc6-compat

COPY . /opt/proxy-load-balancer
COPY proxy.conf /opt/proxy-load-balancer/proxy.conf

WORKDIR /opt/proxy-load-balancer
RUN ./build-and-install.sh

WORKDIR /
RUN rm -rf /opt/proxy-load-balancer
EXPOSE 8080
CMD oplb