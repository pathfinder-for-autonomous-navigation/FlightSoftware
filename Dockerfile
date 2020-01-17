FROM ubuntu:bionic
MAINTAINER Tanishq Aggarwal (ta335@cornell.edu)

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install git g++ valgrind python python-pip -y
RUN pip install -U platformio

RUN pio upgrade
RUN pio update

RUN mkdir -p /FlightSoftware
WORKDIR /FlightSoftware
COPY . .
RUN chmod +x tools/*
