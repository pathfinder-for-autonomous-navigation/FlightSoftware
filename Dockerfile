FROM python:3.7-buster
MAINTAINER Tanishq Aggarwal (ta335@cornell.edu)

RUN pip install -U platformio

RUN pio upgrade
RUN pio update

RUN mkdir -p /FlightSoftware
WORKDIR /FlightSoftware
COPY . .

RUN ./tools/run_desktop_tests.sh
