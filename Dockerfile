FROM ubuntu:xenial
MAINTAINER Tanishq Aggarwal (ta335@cornell.edu)

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install g++ valgrind python python-pip -y
RUN pip install -U platformio

RUN pio upgrade
RUN pio update

RUN mkdir -p /FlightSoftware
WORKDIR /FlightSoftware
COPY . .

RUN ./tools/run_desktop_tests.sh
