# syntax=docker/dockerfile:1
FROM registry.hub.docker.com/library/centos:8.3.2011
RUN dnf -y update && \
dnf -y install \
sudo \
git
COPY . /CacheLib
RUN cd /CacheLib && ./contrib/build.sh -d -j -v
