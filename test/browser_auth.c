/************************************************************************************
   Copyright (C) 2013,2018 MariaDB Corporation AB
                 2021 SingleStore, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not see <http://www.gnu.org/licenses>
   or write to the Free Software Foundation, Inc.,
   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
*************************************************************************************/

#include "ma_odbc.h"
#include "plugins/browser_auth.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#endif

#ifdef WIN32
#define SOCKET_ SOCKET
#else
#define SOCKET_ int
#endif

#define BUFFER_SIZE 2048
#define HTTP_204 "HTTP/1.1 204 No Content\r\nAccess-Control-Allow-Origin: *\r\n\r\n"
#define TOKEN "header.ewogICJlbWFpbCI6ICJ0ZXN0LWVtYWlsQGdtYWlsLmNvbSIsCiAgInVzZXJuYW1lIjogInRlc3QtdXNlciIsCiAgImV4cCI6IDE5MTYyMzkwMjIKfQ==.signature"
/* This token body represents the following JSON:
{
  "email": "test-email@gmail.com",
  "username": "test-user",
  "exp": 1916239022
}
*/

void Assert(int check, char *message)
{
  if (!check)
  {
    printf("%s\n", message);
    exit(1);
  }
}

int invalidSocketCheck(SOCKET_ s)
{
#ifdef WIN32
  return s == INVALID_SOCKET;
#else
  return s < 0;
#endif
}

int closeSocket(SOCKET_ s)
{
#ifdef WIN32
  closesocket(s);
#else
  close(s);
#endif
}

SOCKET_ startMockPortal()
{
  struct sockaddr_in serverAddress;
  SOCKET_ serverSocket;
  int res;

  // Socket setup: creates an endpoint for communication, returns a descriptor
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  Assert(!invalidSocketCheck(serverSocket), "Failed to create socket");

  // Construct local address structure
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(18087);
  serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1;

  // Bind socket to local address
  res = bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
  Assert(!res, "Failed to bind socket");

  // Mark socket to listen for incoming connections
  res = listen(serverSocket, 10);
  Assert(!res, "Failed to mark socket to listen for incoming connections");

  return serverSocket;
}

void sendJWT(char *url)
{
  char command[BUFFER_SIZE];

  memset(command ,0 , BUFFER_SIZE);
  strcat(command, "curl -X POST -d \"");
  strcat(command, TOKEN);
  strcat(command, "\" ");
  strcat(command, url);

  system(command);
}

#ifdef WIN32
DWORD WINAPI
#else
void *
#endif
handle(void *serverSocketVoid)
{
  SOCKET_ clientSocket, *serverSocket;
  char buff[BUFFER_SIZE];
  char *umlStart, *umlEnd;
  int size_recv;

  serverSocket = (SOCKET_*)serverSocketVoid;

  // Accept socket
  clientSocket = accept(*serverSocket, NULL, NULL);
  Assert(!invalidSocketCheck(clientSocket), "Failed to accept the connection");

  // Read the result
  memset(buff ,0 , BUFFER_SIZE);
  size_recv = recv(clientSocket, buff, BUFFER_SIZE-1, 0);
  Assert(size_recv >= 0, "Failed to read the response");

  // Parse port from the request
  umlStart = strstr(buff, "returnTo=");
  Assert(umlStart != NULL, "Wrong request");
  umlStart += strlen("returnTo=");
  umlEnd = strstr(umlStart, "&");
  Assert(umlEnd != NULL, "Wrong request");
  *umlEnd = 0;

  // Answer and close socket
  send(clientSocket, HTTP_204, sizeof(HTTP_204), 0);
  closeSocket(clientSocket);

  // JWT to the driver
  sendJWT(umlStart);
}

int main(int argc, char **argv)
{
  SQLHANDLE henv, hdbc;
  BrowserAuthCredentials creds;
  SOCKET_ serverSocket;
  int res;

#ifdef WIN32
  HANDLE thread;
#else
  pthread_t thread;
#endif
  SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
  SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC2, SQL_IS_INTEGER);
  SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

  serverSocket = startMockPortal();
#ifdef WIN32
  thread = CreateThread(NULL, 0, handle, &serverSocket, 0, NULL);
#else
  pthread_create(&thread, NULL, handle, &serverSocket);
#endif

  res = BrowserAuth(hdbc, "test-email@gmail.com", &creds, BROWSER_AUTH_FLAG_TEST_SHORT_TIMEOUT + BROWSER_AUTH_FLAG_TEST_ENDPOINT);
  Assert(!res, "Browser authentication failed");
#ifdef WIN32
  WaitForSingleObject(thread, INFINITE);
#else
  pthread_join(thread, NULL);
#endif
  closeSocket(serverSocket);

  Assert(!strcmp(creds.email, "test-email@gmail.com"), "Wrong email");
  Assert(!strcmp(creds.token, TOKEN), "Wrong token");
  Assert(!strcmp(creds.username, "test-user"), "Wrong username");
  Assert(creds.expiration == 1916239022, "Wrong exp");

  BrowserAuthCredentialsFree(&creds);

  // Test that BrowserAuth fails when server is not responding
  res = BrowserAuth(hdbc, "test-email@gmail.com", &creds, BROWSER_AUTH_FLAG_TEST_SHORT_TIMEOUT + BROWSER_AUTH_FLAG_TEST_ENDPOINT);
  Assert(res, "Browser authentication expected to fail");

  SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
  SQLFreeHandle(SQL_HANDLE_ENV, henv);

  return 0;
}
