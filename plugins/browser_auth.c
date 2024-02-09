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
#include <dlfcn.h>
#endif

#include "browser_auth.h"
#include "vendor/b64.c/b64.h"
#include "vendor/cJSON/cJSON.h"



void BrowserAuthCredentialsFree(BrowserAuthCredentials *bac)
{
	return;
}

int parseJWTToCredentials(MADB_Dbc *Dbc, const char *token, int token_len, BrowserAuthCredentials *credentials /* out */)
{
	return 0;
}

int BrowserAuth(MADB_Dbc *Dbc, const char *email, BrowserAuthCredentials *credentials /*out*/, int testFlags)
{
  return 0;
}
#undef BUFFER_SIZE

int GetCachedCredentials(MADB_Dbc *Dbc, const char *email, BrowserAuthCredentials *bac /*out*/)
{
  return 0;
}

int GetCredentialsBrowserSSO(MADB_Dbc *Dbc, MADB_Dsn *Dsn, const char *email, my_bool readCached)
{
	return 0;
}
