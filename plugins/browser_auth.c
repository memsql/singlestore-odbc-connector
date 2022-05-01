/************************************************************************************
   Copyright (C) 2021 SingleStore, Inc.

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
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#endif

#include "browser_auth.h"
#include "vendor/b64.c/b64.h"
#include "vendor/cJSON/cJSON.h"

int getJsonStringField(MADB_Dbc *Dbc, cJSON *json, const char *fieldName, char **dst)
{
  cJSON *field;
  field = cJSON_GetObjectItem(json, fieldName);
  if (!cJSON_IsString(field) || (field->valuestring == NULL))
  {
    char errorBuff[128] = "Failed to get the following field from decoded JWT: ";
    strcat(errorBuff, fieldName);
    MADB_SetError(&Dbc->Error, MADB_ERR_28000, errorBuff, 0);
    return 1;
  }
  if (!(*dst = strdup(field->valuestring)))
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
  }
  return 0;
}

int getJsonIntField(MADB_Dbc *Dbc, cJSON *json, const char *fieldName, unsigned long *dst)
{
  cJSON *field;
  field = cJSON_GetObjectItem(json, fieldName);
  if (!cJSON_IsNumber(field))
  {
    char errorBuff[128] = "Failed to get the following field from decoded JWT: ";
    strcat(errorBuff, fieldName);
    MADB_SetError(&Dbc->Error, MADB_ERR_28000, errorBuff, 0);
    return 1;
  }
  *dst = field->valueint;
  return 0;
}

void BrowserAuthCredentialsFree(BrowserAuthCredentials *bac)
{
  if (bac)
  {
    MADB_FREE(bac->token);
    MADB_FREE(bac->username);
    MADB_FREE(bac->email);
  }
}

#ifdef WIN32
#define SOCKET_ SOCKET
#else
#define SOCKET_ int
#endif

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

// if endpoint is NULL PORTAL_SSO_ENDPOINT is used. Not NULL endpoint values are used for testing
int getOpenBrowserCommand(MADB_Dbc *Dbc, char *returnTo, const char *email, const char *endpoint, MADB_DynString *openBrowserCommand)
{
#ifdef WIN32
  if (MADB_DynstrAppend(openBrowserCommand, "START /B rundll32 url.dll,FileProtocolHandler "))
#elif __APPLE__
  if (MADB_DynstrAppend(openBrowserCommand, "open "))
#else
  if (MADB_DynstrAppend(openBrowserCommand, "xdg-open "))
#endif
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
  }
  if (MADB_DynstrAppend(openBrowserCommand, "\"") ||
      MADB_DynstrAppend(openBrowserCommand, endpoint == NULL ? PORTAL_SSO_ENDPOINT: endpoint) ||
      MADB_DynstrAppend(openBrowserCommand, "?returnTo=") ||
      MADB_DynstrAppend(openBrowserCommand, returnTo) ||
      (email &&
       (MADB_DynstrAppend(openBrowserCommand, "&email=") ||
        MADB_DynstrAppend(openBrowserCommand, email))) ||
      MADB_DynstrAppend(openBrowserCommand, "\""))
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
  }

#ifndef WIN32
  if (MADB_DynstrAppend(openBrowserCommand, " &"))
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
  }
#endif

  return 0;
}

int startLocalHttpServer(MADB_Dbc *Dbc, SOCKET_ *serverSocket, MADB_DynString *serverEndpoint)
{
  char portStr[10];
  struct sockaddr_in serverAddress;
  socklen_t addrLen;

  // Socket setup: creates an endpoint for communication, returns a descriptor
  if (invalidSocketCheck(*serverSocket = socket(AF_INET, SOCK_STREAM, 0)))
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to create socket", 0);
  }

  // Construct local address structure
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = 0; // choose any free port
  serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1;

  // Bind socket to local address
  if (bind(*serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to bind socket for browser authentication", 0);
    goto cleanupSocket;
  }

  // Get information about server address
  addrLen = sizeof(serverAddress);
  if (getsockname(*serverSocket, (struct sockaddr *)&serverAddress, &addrLen)) {
    MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to get information about socket address for browser authentication", 0);
    goto cleanupSocket;
  }
  sprintf(portStr, "%d", ntohs(serverAddress.sin_port));

  // Mark socket to listen for incoming connections
  // 10 is the maximum length to which the queue of pending connections for sockfd may grow
  if (listen(*serverSocket, 10))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to mark socket to listen for incoming connections for browser authentication", 0);
    goto cleanupSocket;
  }

  // Generate server endpoint
  if (MADB_DynstrAppend(serverEndpoint, "http://127.0.0.1:")  || MADB_DynstrAppend(serverEndpoint, portStr))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
    goto cleanupSocket;
  }

  return 0;

cleanupSocket:
  closeSocket(*serverSocket);
  return Dbc->Error.ReturnValue;
}

int parseCredentialsFromJson(MADB_Dbc *Dbc, cJSON *json, const char *jwt, int jwt_len, BrowserAuthCredentials *credentials)
{
  memset(credentials, 0, sizeof(BrowserAuthCredentials));
  // TODO: PLAT-6174 update json parsing
  if (getJsonStringField(Dbc, json, "email", &credentials->email) ||
      getJsonStringField(Dbc, json, "username", &credentials->username) ||
      getJsonIntField(Dbc, json, "exp", &credentials->expiration))
  {
    goto end;
  }

  if (!(credentials->token = strndup(jwt, jwt_len)))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY000, NULL, 0);
    goto end;
  }

end:
  return Dbc->Error.ReturnValue;
}

// parseJWTToCredentials gets login credentials from the JWT `token`.
// `token` consists of three parts separated by dot "."
// `token_len` is either the length of `token` or non-positive integer,
// which means `token` end is indicated by a zero byte.
// <header>.<payload>.<verify signature>
int parseJWTToCredentials(MADB_Dbc *Dbc, const char *token, int token_len, BrowserAuthCredentials *credentials /* out */)
{
  const char *jwtPayloadStart;
  const char *jwtPayloadEnd;
  const char *base64URL;
  char *base64;
  char *jsonStr;
  long base64URLLen;
  long base64Len;
  cJSON *json;
  int i;

  ADJUST_LENGTH(token, token_len);
  if (!token_len) token_len = strlen(token);

  if ((jwtPayloadStart = memchr(token, '.', token_len)) == NULL)
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to parse browser authentication credentials: JWT contains only header", 0);
  }
  jwtPayloadStart++;

  if ((jwtPayloadEnd = memchr(jwtPayloadStart, '.', token_len - (jwtPayloadStart - token))) == NULL)
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to parse browser authentication credentials: JWT contains only header and payload", 0);
  }

  base64URLLen = jwtPayloadEnd - jwtPayloadStart;
  base64Len = ((base64URLLen + 3) / 4) * 4;

  if (!(base64 = MADB_CALLOC(base64Len+1)))
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_HY000, NULL, 0);
  }
  base64URL = jwtPayloadStart;
  for (i = 0; i < base64Len; i++)
  {
    if (i < base64URLLen)
    {
      if (base64URL[i] == '-')
      {
        base64[i] = '+';
      } else if (base64URL[i] == '_')
      {
        base64[i] = '/';
      } else
      {
        base64[i] = base64URL[i];
      }
    } else
    {
      base64[i] = '=';
    }
  }

  if (!(jsonStr = (char *)b64_decode(base64, base64Len)))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to parse base64 string for browser authentication", 0);
    goto cleanup;
  }

  json = cJSON_Parse(jsonStr);
  if (json == NULL || !cJSON_IsObject(json))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to parse JSON string for browser authentication", 0);
    goto cleanup;
  }

  if (parseCredentialsFromJson(Dbc, json, token, token_len, credentials))
  {
    goto cleanup;
  }

cleanup:
  MADB_FREE(base64);
  MADB_FREE(jsonStr);
  cJSON_Delete(json);

  return Dbc->Error.ReturnValue;
}

// parseRequest parses HTTP request sent from the browser to
// the local HTTP server started by the driver and retrieves credentials from it.
//
// Request contains status line, zero or more header lines terminated by "\r\n" each.
// Then one line with "\r\n" and request body.
// <status>\r\n
// <header>\r\n
// ...
// <header>\r\n
// \r\n
// <body>
//
// Body of the request should contain JWT.
// JWT consists of three parts separated by dot "."
// <header>.<payload>.<verify signature>
//
// Second part of the JWT contains base64URL encoded JSON
// This JSON should contain all fields of BrowserAuthCredentials
int parseRequest(MADB_Dbc *Dbc, const char *request, BrowserAuthCredentials *credentials /*out*/)
{
  const char *bodyStart;
  if ((bodyStart = strstr(request, "\r\n\r\n")) == NULL)
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to parse browser authentication credentials: http request doesn't contain body", 0);
  }
  bodyStart += 4;

  return parseJWTToCredentials(Dbc, bodyStart, SQL_NTS, credentials);
}

// tryGetFullRequestLength takes request prefix and if it contains all headers
// then it retrieves Content-Length header and computes full length of the request
// If Content-Length header is not presented - it assumes that the body is empty
#define CONTENT_LENGTH "Content-Length: "
long tryGetFullRequestLength(const char *request)
{
  char *bodyStart;
  char *contentLengthHeader;
  long headersLen;
  long contentLen;

  if (!(bodyStart = strstr(request, "\r\n\r\n")))
  {
    // request doesn't contain all headers
    return -1;
  }
  bodyStart += 4;
  headersLen = bodyStart - request;

  if (!(contentLengthHeader = strstr(request, CONTENT_LENGTH)))
  {
    // No Content-Length header
    // Assuming that the body is empty
    return headersLen;
  }
  contentLengthHeader += strlen(CONTENT_LENGTH);
  contentLen = strtol(contentLengthHeader, NULL, 10);

  return headersLen + contentLen;
}

int makeSocketNonBlocking(SOCKET_ socket)
{
#ifdef WIN32
  u_long mode = 1;  // 1 to enable non-blocking socket
  if (ioctlsocket(socket, FIONBIO, &mode))
  {
    return 1;
  }
#else
  int flags;
  if ((flags = fcntl(socket, F_GETFL)) == -1)
  {
    return 1;
  }
  if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
  {
    return 1;
  }
#endif

  return 0;
}

int isBlockingError()
{
#ifdef WIN32
  return WSAGetLastError() == WSAEWOULDBLOCK;
#else
  return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

void sleepMilliseconds(int milliseconds)
{
#ifdef WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds*1000);
#endif
}

#define BUFFER_SIZE 2048
#define HTTP_204 "HTTP/1.1 204 No Content\r\nAccess-Control-Allow-Origin: *\r\n\r\n"
#define HTTP_400 "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\n\r\n"
#define HTTP_500 "HTTP/1.1 500 Internal Server Error\r\nAccess-Control-Allow-Origin: *\r\n\r\n"
int readRequest(MADB_Dbc *Dbc, SOCKET_ serverSocket, int requestReadTimeoutSec, BrowserAuthCredentials *credentials)
{
  int size_recv;
  char buff[BUFFER_SIZE];
  SOCKET_ clientSocket;
  MADB_DynString request;
  long fullRequestLength = -1;
  time_t startTime;

  startTime = time(NULL);

  if (MADB_InitDynamicString(&request, "", BUFFER_SIZE, BUFFER_SIZE))
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
  }

  // Change serverSocket to be non blocking
  // This is needed in order to don't wait for request on the accept call
  // If request is not send, accept will return an error that can be checked using isBlockingError function
  makeSocketNonBlocking(serverSocket);
  while (invalidSocketCheck(clientSocket = accept(serverSocket, NULL, NULL)))
  {
    if (isBlockingError())
    {
      if (time(NULL) - startTime > requestReadTimeoutSec)
      {
        MADB_SetError(&Dbc->Error, MADB_ERR_HYT00, "Browser authentication response timeout expired", 0);
        send(clientSocket, HTTP_400, sizeof(HTTP_400), 0);
        goto cleanupRequest;
      }

      sleepMilliseconds(10);
    } else
    {
      MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to accept the connection for browser authentication", 0);
      goto cleanupRequest;
    }
  }

  // Change clientSocket to be non blocking
  // This is needed in order to don't wait for data on the recv call
  // If data is not, send recv will return an error that can be checked using isBlockingError function
  makeSocketNonBlocking(clientSocket);
  while(fullRequestLength == -1 || request.length < fullRequestLength)
  {
    memset(buff, 0 , BUFFER_SIZE);
    size_recv = recv(clientSocket, buff, BUFFER_SIZE-1, 0);
    if (size_recv < 0)
    {
      if (isBlockingError())
      {
        if (time(NULL) - startTime > requestReadTimeoutSec)
        {
          MADB_SetError(&Dbc->Error, MADB_ERR_HYT00, "Browser authentication response timeout expired", 0);
          send(clientSocket, HTTP_400, sizeof(HTTP_400), 0);
          goto cleanupSocket;
        }

        sleepMilliseconds(10);
        continue;
      } else
      {
        MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to read data from socket for browser authentication", 0);
        send(clientSocket, HTTP_400, sizeof(HTTP_400), 0);
        goto cleanupSocket;
      }
    }

    if (MADB_DynstrAppend(&request, buff))
    {
      MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
      send(clientSocket, HTTP_500, sizeof(HTTP_500), 0);
      goto cleanupSocket;
    }

    if (fullRequestLength == -1)
    {
      fullRequestLength = tryGetFullRequestLength(request.str);
    }
  }

  if (parseRequest(Dbc, request.str, credentials))
  {
    send(clientSocket, HTTP_400, sizeof(HTTP_400), 0);
    goto cleanupSocket;
  }

  send(clientSocket, HTTP_204, sizeof(HTTP_204), 0);

cleanupSocket:
  closeSocket(clientSocket);
cleanupRequest:
  MADB_DynstrFree(&request);
  return Dbc->Error.ReturnValue;
}

void makeTestCreds(BrowserAuthCredentials *credentials /*out*/)
{
  credentials->username = strdup("test_jwt_user");
  credentials->email = strdup("test@singlestore.com");
  credentials->expiration = 2514359920;
  credentials->token = strdup(getenv("MEMSQL_JWT"));
}

// BrowserAuth opens browser to perform the auth workflow using SSO.
// Return 0 if the credentials have been filled successfully.
// testFlags are used to assert and control certain aspects in tests 
int BrowserAuth(MADB_Dbc *Dbc, const char *email, BrowserAuthCredentials *credentials /*out*/, int testFlags)
{
  MDBUG_C_ENTER(Dbc, "BrowserAuth");
  int requestReadTimeoutSec = testFlags & BROWSER_AUTH_FLAG_TEST_SHORT_TIMEOUT ? READ_TIMEOUT_TEST_SEC : READ_TIMEOUT_USER_SEC;
  char* endpoint = testFlags & BROWSER_AUTH_FLAG_TEST_ENDPOINT ? LOCAL_TEST_ENDPOINT : PORTAL_SSO_ENDPOINT;
  if (testFlags & BROWSER_AUTH_FLAG_TEST_FIRST_CALL)
  {
    makeTestCreds(credentials);
    MDBUG_C_RETURN(Dbc, 0, &Dbc->Error);
  }
  if (testFlags & BROWSER_AUTH_FLAG_TEST_SECOND_CALL)
  {
    assert(0 && "BrowserAuth must not be called in the second connection with BROWSER_SSO");
    MDBUG_C_RETURN(Dbc, 0, &Dbc->Error);
  }
  MADB_DynString serverEndpoint;
  MADB_DynString openBrowserCommand;
  SOCKET_ serverSocket;

  if (MADB_InitDynamicString(&serverEndpoint, "", BUFFER_SIZE, BUFFER_SIZE))
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
  }
  if (MADB_InitDynamicString(&openBrowserCommand, "", BUFFER_SIZE, BUFFER_SIZE))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
    goto cleanupServerEndpoint;
  }

  if (startLocalHttpServer(Dbc, &serverSocket, &serverEndpoint))
  {
    goto cleanupOpenBrowserCommand;
  }
  if (getOpenBrowserCommand(Dbc, serverEndpoint.str, email, endpoint, &openBrowserCommand))
  {
    goto cleanupServer;
  }
  if (system(openBrowserCommand.str))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_S1000, "Failed to open browser", 0);
    goto cleanupServer;
  }
  if (readRequest(Dbc, serverSocket, requestReadTimeoutSec, credentials))
  {
    goto cleanupServer;
  }

cleanupServer:
  closeSocket(serverSocket);
cleanupOpenBrowserCommand:
  MADB_DynstrFree(&openBrowserCommand);
cleanupServerEndpoint:
  MADB_DynstrFree(&serverEndpoint);
  MDBUG_C_RETURN(Dbc, Dbc->Error.ReturnValue, &Dbc->Error);
}
#undef BUFFER_SIZE

#if !defined(_WIN32) && !defined(__APPLE__)
// Linux secure key storage
const SecretSchema *jwt_cache_get_schema(void)
{
  static const SecretSchema the_schema = {
    SECURE_JWT_STORAGE_KEY, SECRET_SCHEMA_NONE,
    {
      {"email", SECRET_SCHEMA_ATTRIBUTE_STRING},
      {"NULL", 0},
    }
  };
  return &the_schema;
}
#endif

int GetCachedCredentials(MADB_Dbc *Dbc, const char *email, BrowserAuthCredentials *bac /*out*/)
{
  MDBUG_C_ENTER(Dbc, "GetCachedCredentials");
#if defined(_WIN32)
// Win secure key storage
  CREDENTIALW* pCredential = NULL;
  int res = CredReadW(SECURE_JWT_STORAGE_KEY, CRED_TYPE_GENERIC, 0 /*flags*/, &pCredential);
  if (!res)
  {
    unsigned long lastError = GetLastError();
    if (lastError != ERROR_NOT_FOUND)
    {
      MDBUG_C_PRINT(Dbc, "Got error in CredReadW. Error code is: %lu", lastError);
      MADB_SetError(&Dbc->Error, MADB_ERR_HY000, "Failed to read JWT for login from Windows Credential Manager", 0);
    }
    goto endwin;
  }
  MDBUG_C_PRINT(Dbc, "%s SUCCESS", "CredReadW");
  parseJWTToCredentials(Dbc, pCredential->CredentialBlob, pCredential->CredentialBlobSize, bac);
endwin:
  if(pCredential) CredFree(pCredential);
  MDBUG_C_RETURN(Dbc, Dbc->Error.ReturnValue, &Dbc->Error);
#elif defined(__APPLE__)
// Mac secure key storage
 void *password_data = NULL;
 uint32_t password_len;
 char *email_str = email != NULL ? email : "";
 OSStatus status = SecKeychainFindGenericPassword(
  NULL /* default keychain */,
  strlen(SECURE_JWT_STORAGE_KEY),
  SECURE_JWT_STORAGE_KEY,
  strlen(email_str),
  email_str,
  &password_len,
  &password_data,
  NULL);
if (status == errSecSuccess)
{
  parseJWTToCredentials(Dbc, password_data, password_len, bac);
}
else if (status != errSecItemNotFound)
{
  // TODO: refactor PLAT-6194
  char error_msg[128];
  int error_len = snprintf(error_msg, 127, "Error reading token from Mac OS keychain: code is %d", status);
  error_msg[MIN(127, error_len)] = '\0';
  MADB_SetError(&Dbc->Error, MADB_ERR_HY000, error_msg, 0);
}
MDBUG_C_RETURN(Dbc, Dbc->Error.ReturnValue, &Dbc->Error);
#else
// Linux secure key storage
  gchar *password = NULL;
  GError *err = NULL;

  SecretService * service = secret_service_get_sync (0, NULL, &err);
  SecretCollection *defaultCollection = secret_collection_for_alias_sync(
    service, SECRET_COLLECTION_DEFAULT, 0 /*SecretCollectionFlags*/, NULL, &err);
  if (err != NULL)
  {
    // gnome-keyring secret service cannot be started or no collection is available
    goto gerror;
  }
  gboolean is_locked = secret_collection_get_locked(defaultCollection);
  // TODO: PLAT-6176 try to unlock with timeout so that secret_password_lookup_sync doesn't hang
  if (email)
    password = secret_password_lookup_sync(
      JWT_CACHE_SCHEMA, NULL, &err, "email", email, NULL);
  else
    password = secret_password_lookup_sync(
      JWT_CACHE_SCHEMA, NULL, &err, NULL);

  if (err != NULL)
  {
    // password lookup error
    goto gerror;
  }
  if (password)
  {
    parseJWTToCredentials(Dbc, (char*)password, SQL_NTS, bac);
    secret_password_free(password);
  }
  MDBUG_C_RETURN(Dbc, 0, &Dbc->Error);
gerror:
  MADB_SetError(&Dbc->Error, MADB_ERR_HY000, err->message, 0);
  g_error_free(err);
  MDBUG_C_RETURN(Dbc, Dbc->Error.ReturnValue, &Dbc->Error);
#endif
}

int PutCachedCredentials(MADB_Dbc *Dbc, BrowserAuthCredentials *bac)
{
  MDBUG_C_ENTER(Dbc, "PutCachedCredentials");
#if defined(_WIN32)
// Win secure key storage
  CREDENTIALW* winCred = NULL;
  wchar_t* userName = NULL;
  if ( !(winCred = (CREDENTIALW*)malloc(sizeof(CREDENTIALW))) )
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
    goto endwin;
  }
  winCred->Flags = 0;
  winCred->Type = CRED_TYPE_GENERIC;
  winCred->TargetName = SECURE_JWT_STORAGE_KEY;
  winCred->TargetAlias = NULL;
  winCred->Comment = NULL;
  winCred->CredentialBlobSize = strlen(bac->token);
  winCred->CredentialBlob = bac->token;
  winCred->Persist = CRED_PERSIST_LOCAL_MACHINE;
  winCred->AttributeCount = 0;
  winCred->Attributes = NULL;
  if ( !(userName = (wchar_t*)malloc(sizeof(wchar_t) * (strlen(bac->username) + 1))) )
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
  }
  swprintf(userName, strlen(bac->username) + 1, L"%hs", bac->username);
  winCred->UserName = userName;

  int res = CredWriteW(winCred, 0 /*flags*/);
  if (!res)
  {
    unsigned long lastError = GetLastError();
    MDBUG_C_PRINT(Dbc, "Got error in CredWriteW. Error code is: %lu", lastError);
    MADB_SetError(&Dbc->Error, MADB_ERR_HY000, "Failed to write to Windows Credential Manager", 0);
    goto endwin;
  }
  MDBUG_C_PRINT(Dbc, "%s SUCCESS", "CredWriteW");
endwin:
  MADB_FREE(userName);
  MADB_FREE(winCred);
  MDBUG_C_RETURN(Dbc, Dbc->Error.ReturnValue, &Dbc->Error);
#elif defined(__APPLE__)
// Mac secure key storage
SecKeychainItemRef item = NULL;
OSStatus status = SecKeychainFindGenericPassword(
  NULL /* default keychain */,
  strlen(SECURE_JWT_STORAGE_KEY),
  SECURE_JWT_STORAGE_KEY,
  strlen(bac->email),
  bac->email,
  NULL /* unused output parameter */,
  NULL /* unused output parameter */,
  &item);

if (status == errSecSuccess)
{
  status = SecKeychainItemModifyContent(item, NULL, strlen(bac->token), bac->token);
}
else
{
  status = SecKeychainAddGenericPassword(
    NULL /* default keychain */,
    strlen(SECURE_JWT_STORAGE_KEY),
    SECURE_JWT_STORAGE_KEY,
    strlen(bac->email),
    bac->email,
    strlen(bac->token),
    bac->token,
    NULL /* unused output parameter */);
}
if (status != errSecSuccess)
{
  // TODO: refactor PLAT-6194
  char error_msg[128];
  int error_len = snprintf(error_msg, 127, "Error writing token to Mac OS keychain: code is %d", status);
  error_msg[MIN(127, error_len)] = '\0';
  MADB_SetError(&Dbc->Error, MADB_ERR_HY000, error_msg, 0);
}
if (item)
{
  CFRelease(item);
}
MDBUG_C_RETURN(Dbc, Dbc->Error.ReturnValue, &Dbc->Error);
#else
// Linux secure key storage
  GError *error = NULL;
  secret_password_store_sync(JWT_CACHE_SCHEMA, SECRET_COLLECTION_DEFAULT,
                            SECURE_JWT_STORAGE_KEY, bac->token, NULL, &error,
                            "email", bac->email,
                            NULL);
  if (error != NULL)
  {
    MDBUG_C_PRINT(Dbc, "%s FAILED", "secret_password_store_sync");
    MADB_SetError(&Dbc->Error, error->code, error->message, 0);
    g_error_free(error);
    MDBUG_C_RETURN(Dbc, Dbc->Error.ReturnValue, &Dbc->Error);
  }
  else 
  {
    MDBUG_C_PRINT(Dbc, "%s SUCCESS", "secret_password_store_sync");
    MDBUG_C_RETURN(Dbc, 0, &Dbc->Error);
  }
#endif
}
