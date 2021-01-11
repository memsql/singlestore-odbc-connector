if [[ $IMAGE_NAME == centos* ]]
then
  yum update
  yum -y install git cmake make gcc openssl-devel unixODBC unixODBC-devel
else
  apt-get update
  apt-get install -y cmake make gcc libssl-dev unixodbc-dev
#  apt update
#  apt install -y curl
#  apt-get update
#  apt-get install -y mysql-client-core-5.5
#  apt-get install -y unixodbc odbcinst
#  apt-get install -y unixodbc-dev
#  apt-get install -y build-essential
#  apt-get install -y cmake
#  apt-get install -y libssl-dev
#  apt-get install --allow-unauthenticated -y --force-yes -m unixodbc-dev odbcinst1debian2 libodbc1
#  apt-get clean
fi