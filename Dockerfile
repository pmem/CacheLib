# syntax=docker/dockerfile:1
FROM registry.hub.docker.com/library/centos:8.3.2011
RUN dnf -y update && \
dnf -y install \
sudo \
git && \
echo 'Defaults    env_keep += "HTTPS_PROXY https_proxy HTTP_PROXY http_proxy NO_PROXY no_proxy"' >> /etc/sudoers
COPY . /CacheLib
RUN cd /CacheLib && ./contrib/build.sh -d -j -v
