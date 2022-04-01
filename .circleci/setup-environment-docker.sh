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
  yum -y install libglib2.0 libsecret-1-dev
  wget https://github.com/Kitware/CMake/releases/download/v3.20.3/cmake-3.20.3-linux-x86_64.tar.gz
  cd /usr || exit
  tar --strip-components=1 -xzf /root/project/cmake-3.20.3-linux-x86_64.tar.gz
else
  apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 648ACFD622F3D138  # Debian 10/buster archive signing key
  apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 0E98404D386FA1D9  # Debian 11/bullseye archive signing key
  apt-key adv --recv-keys --keyserver keyserver.ubuntu.com DCC9EFBF77E11517  # Debian 10/buster release key
  apt-get update
  apt-get install -y cmake make gcc libssl-dev dnsutils
  echo "deb http://deb.debian.org/debian buster main contrib non-free" >> /etc/apt/sources.list.d/tutu.list
  apt-get update
  apt-get install -y unixodbc-dev=2.3.6-0.1 libglib2.0 libsecret-1-dev
fi
