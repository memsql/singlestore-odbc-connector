if [ "$MEMSQL_PASSWORD" = "" ]
then
  mysql -h 127.0.0.1 -u root -P 5506 -e "create database test;"
else
  mysql -h 127.0.0.1 -u root -P 5506 --password="$MEMSQL_PASSWORD" -e "create database test;"
fi