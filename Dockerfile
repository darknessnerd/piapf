FROM debian:buster-slim

LABEL maintainer="Brunino Criniti <crinitib@gmail.com>"

VOLUME "/project"

WORKDIR "/project"

RUN apt-get update 
RUN apt-get -y upgrade 
RUN apt-get -y install build-essential manpages-dev python3 python3-pip cmake
RUN apt-get -y install deluge-console net-tools 
RUN pip3 install conan 

ENTRYPOINT [ "bash", "-c" ]
