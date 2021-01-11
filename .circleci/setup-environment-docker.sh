if [[ $IMAGE_NAME == centos* ]]
then
  yum update
  yum -y install curl
  yum -y install mysql
  yum -y install gcc gcc-c++
  yum -y install make cmake
  yum -y install unixODBC unixODBC-devel
else
  apt update
  apt install -y curl
  apt-get update
  apt-get install -y mysql-client-core-5.5
  apt-get install -y unixodbc odbcinst
  apt-get install -y unixodbc-dev
  apt-get install -y build-essential
  apt-get install -y cmake
  apt-get install --allow-unauthenticated -y --force-yes -m unixodbc-dev odbcinst1debian2 libodbc1
  apt-get clean
fi