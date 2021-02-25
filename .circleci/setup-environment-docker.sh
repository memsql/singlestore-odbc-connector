if [[ $IMAGE_NAME == centos* ]]
then
  yum update
  yum -y install cmake make gcc openssl-devel unixODBC unixODBC-devel
else
  apt-get update
  apt-get install -y cmake make gcc libssl-dev unixodbc odbcinst unixodbc-dev
fi