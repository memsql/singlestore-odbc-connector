# ************************************************************************************
#   Copyright (c) 2021 SingleStore, Inc.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public
#   License along with this library; if not see <http://www.gnu.org/licenses>
#   or write to the Free Software Foundation, Inc.,
#   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
# *************************************************************************************/

if [[ $IMAGE_NAME == centos* ]]
then
  sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-Linux-*
  sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.epel.cloud|g' /etc/yum.repos.d/CentOS-Linux-*
  yum -y update
  yum -y install gcc-c++ make gcc openssl-devel unixODBC unixODBC-devel wget bind-utils
  yum -y install libglib2.0 libsecret-1-dev  # for CentOS 7
  yum -y install glib2-devel libsecret-devel
  wget https://github.com/Kitware/CMake/releases/download/v3.20.3/cmake-3.20.3-linux-x86_64.tar.gz
  cd /usr || exit
  tar --strip-components=1 -xzf /root/project/cmake-3.20.3-linux-x86_64.tar.gz
  yum -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-$(rpm --eval '%{centos_ver}').noarch.rpm
  yum -y install dnf-plugins-core
  yum config-manager --set-enabled powertools
  yum -y install xdg-utils w3m curl
  # install python and pip
  yum -y install wget make gcc openssl-devel bzip2-devel
  wget https://www.python.org/ftp/python/3.7.9/Python-3.7.9.tgz
  tar xzf Python-3.7.9.tgz
  ./Python-3.7.9/configure --enable-optimizations
  make altinstall
  ln -sfn /usr/local/bin/python3.7 /usr/bin/python3
  ln -sfn /usr/local/bin/pip3.7 /usr/bin/pip3
  pip3 install urllib3==1.26.16
else
  apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 648ACFD622F3D138  # Debian 10/buster archive signing key
  apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 0E98404D386FA1D9  # Debian 11/bullseye archive signing key
  apt-key adv --recv-keys --keyserver keyserver.ubuntu.com DCC9EFBF77E11517  # Debian 10/buster release key
  echo "deb http://deb.debian.org/debian buster main contrib non-free" >> /etc/apt/sources.list.d/tutu.list
  apt-get update
  apt-get install -y cmake make gcc libssl-dev dnsutils
  apt-get install -y unixodbc-dev 'libglib2.0-0' libglib2.0-dev libsecret-1-dev xdg-utils w3m curl
  # install python and pip
  apt-get install -y python3 python3-pip
  apt-get install -y libkrb5-dev
fi

echo 'export BROWSER=$(which w3m)' >> $BASH_ENV
