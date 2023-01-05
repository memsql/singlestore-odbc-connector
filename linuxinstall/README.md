Creating a Data Source with SingleStore ODBC Driver

ODBC driver offers the flexibility to handle data using any character set through
its Unicode-enabled driver, or the maximum raw speed for a more limited range of character
sets through its ANSI driver.
Both kinds of drivers are provided in the same download package.
To install Unicode driver, use libssodbcw.so in the father steps,
otherwise use the libssodbca.so.

1. Install UnixODBC and OpenSSL

The following command would install the UnixODBC package.

(Centos)
```
sudo yum update
sudo yum install unixODBC
sudo yum install openssl-devel
```
(Debian)
```
sudo apt-get update
sudo apt-get install unixodbc
sudo apt-get install libssl-dev
```

If you plan to compile an application from source against SingleStore ODBC Driver and UnixODBC,
then you also need the development header files that define the ODBC API function prototypes,
ODBC data types, etc.
To install UnixODBC development files you can use the following command.

(Centos)
```
sudo yum update
sudo yum install unixODBC-devel
```
(Debian)
```
sudo apt-get update
sudo apt-get install unixodbc-dev
```

2. Configuring SingleStore ODBC Driver as a UnixODBC Driver on Linux

Create a template file similar to the following,
with a name like SingleStore_odbc_driver_template.ini.

```
[SingleStore ODBC ANSI|Unicode Driver]
Description = SingleStore ODBC Driver
Driver = <your path>/libssodbc[a|w].so
```

Install it to the system's global /etc/odbcinst.ini file with the following command.

```
sudo odbcinst -i -d -f SingleStore_odbc_driver_template.ini
```

3. Configuring a DSN with UnixODBC on Linux

Create a template file similar to the following,
with a name like SingleStore_odbc_data_source_template.ini.

```
[SingleStore-server]
Description=SingleStore server
Driver=SingleStore ODBC ANSI|Unicode Driver
SERVER=<your server>
USER=<your user>
PASSWORD=<your password>
DATABASE=<your database>
PORT=<your port>
```

Install it to the system's global /etc/odbc.ini file with the following command.

```
sudo odbcinst -i -s -l -f SingleStore_odbc_data_source_template.ini
```

4. Verifying a DSN Configuration with UnixODBC on Linux

Verify that a DSN is properly configured with UnixODBC on Linux by using the isql utility.

```
isql SingleStore-server
```
