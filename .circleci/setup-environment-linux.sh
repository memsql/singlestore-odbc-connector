if [[ $IMAGE_NAME == centos* ]]
then
  yum update
  yum -y install curl
  yum -y install mysql
  yum -y install git
  yum -y install gcc gcc-c++
  yum -y install make cmake
  yum -y install unixODBC unixODBC-devel
else
  sudo apt update
  sudo apt install -y curl
  sudo apt-get update
  sudo apt-get install -y mysql-client-core-5.5
  sudo apt-get install unixodbc odbcinst
  sudo apt-get install unixodbc-dev
  sudo apt-get install --allow-unauthenticated -y --force-yes -m unixodbc-dev odbcinst1debian2 libodbc1
  sudo apt-get clean
fi