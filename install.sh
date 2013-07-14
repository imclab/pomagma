#!/bin/sh

sudo apt-get install -y \
  cmake make g++ \
  libboost1.48-all-dev \
  libtbb-dev \
  libsparsehash-dev \
  libprotobuf-dev protobuf-compiler python-protobuf \
  libhdf5-serial-dev \
  libssl-dev \
  python-pip virtualenvwrapper \
  python-tables \
  graphviz \
  gdb \
  p7zip-full \
  rubygems \
  libzmq-dev \
  nodejs npm \
  #python-zmq \
  #
set +h

sudo gem install foreman

# from
# https://github.com/joyent/node/wiki/Installing-Node.js-via-package-manager
sudo add-apt-repository -y ppa:chris-lea/node.js
sudo apt-get update
sudo apt-get install -y nodejs
set +h

(cd src/editor; npm install jquery underscore)

# FIXME mkvirtualenv never automatically works; I have to do it by hand; wtf
workon pomagma || mkvirtualenv --system-site-packages pomagma
set +h
workon pomagma && pip install -r requirements.txt && pip install -e . && make

