#https://linuxiac.com/how-to-install-nextcloud-with-docker-compose/

FROM debian:bookworm-backports

MAINTAINER salsi.it

RUN DEBIAN_FRONTEND=noninteractive apt update && apt-get upgrade -y 
RUN DEBIAN_FRONTEND=noninteractive apt install -y  build-essential \
g++ \
zlib1g-dev \
zlib1g \
libbz2-dev \
graphviz-dev \
libicu-dev \
cmake \
libssl-dev \
autoconf \
libgraphicsmagick++1-dev \
libpq-dev \
libpango1.0-dev \
liblzma-dev \
libcurl4-openssl-dev \
libsqlite3-dev \
git \
libboost-all-dev
nattended-upgrades \
apt-listchanges

COPY wt /usr/local/src/wt
COPY pocket /usr/local/src/pocket
COPY cppcommons /usr/local/src/cppcommons

RUN mv /usr/local/src/wt/pocket-wt/wt-4.8.3.tar.gz /usr/local/src
WORKDIR /usr/local/src
RUN tar -zxvf wt-4.8.3.tar.gz 
WORKDIR wt-4.8.3
RUN mkdir build
WORKDIR build
RUN cmake \
-D MULTI_THREADED=ON \
-D RUNDIR=/var/www/wt \
-D WEBUSER=www-data \
-D WEBGROUP=www-data \
-D BOOST_ROOT=/usr \
-D BOOST_LIBRARYDIR=/usr/lib/boost \
-D BOOST_INCLUDEDIR=/usr/include/boost \
-D SHARED_LIBS=ON \
-D CONNECTOR_FCGI=OFF \
-D CONNECTOR_HTTP=ON \
-D USERLIB_PREFIX=/usr/lib \
-D Boost_USE_STATIC_LIBS=OFF \
-D Boost_USE_STATIC_RUNTIME=OFF \
-D CONFIGDIR=/etc/wt \
..
RUN make -j16
RUN make install
RUN mkdir /usr/local/src/wt/pocket-wt/build
WORKDIR /usr/local/src/wt/pocket-wt/build
RUN cmake ..
RUN make -j16
RUN cp /usr/local/src/wt/pocket-wt/build/pocket-wt /var/www/wt/pocket-wt
RUN ldconfig

RUN mkdir -p /var/www/wt
RUN mkdir -p /var/www/wt/file_folder
RUN mv /usr/local/src/wt/pocket-wt/resources /var/www/wt/resources/

RUN echo "#!/bin/bash\n\n/var/www/wt/pocket-wt --docroot /var/www/wt/ --http-address 0.0.0.0 --http-port 8082 --resources-dir=/var/www/wt/html/resources/" > /usr/local/bin/pocket.sh
RUN chmod +x /usr/local/bin/pocket.sh

RUN DEBIAN_FRONTEND=noninteractive apt-get clean && rm -rf /var/lib/apt/lists/*
RUN rm -fr /usr/local/src/*

RUN chown -R www-data:www-data /var/www
USER www-data

WORKDIR /var/www/wt

#ENTRYPOINT ["/var/www/wt/pocket-wt" "--docroot" "/var/www/wt/" "--http-address" "0.0.0.0" "--http-port" "8082" "--resources-dir=/var/www/wt/html/resources/"]
CMD ["/usr/local/bin/pocket.sh"]