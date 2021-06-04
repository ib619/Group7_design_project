FROM ubuntu:18.04

WORKDIR /home
RUN apt-get update
RUN apt-get install -y mosquitto
RUN apt-get install -y gcc
RUN apt-get install -y nano
RUN apt-get install -y python3
RUN apt-get install -y python3-pip

COPY requirements.txt ./
RUN pip3 install --no-cache-dir -r requirements.txt

WORKDIR /etc/mosquitto
COPY mosquitto.conf .
COPY pwdfile .
COPY acl .

WORKDIR /home
COPY  . .

CMD ./start_server.sh

