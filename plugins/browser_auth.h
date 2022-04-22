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

#ifndef _browser_auth_h_
#define _browser_auth_h_

#include "ma_odbc.h"

typedef struct BrowserCredentials {
  char *token;
  char *username;
  char *email;
  // expiration is a numeric value representing the number of seconds from
  // 1970-01-01T00:00:00Z UTC until the expiration UTC date/time,
  // ignoring leap seconds.
  int expiration;
} BrowserAuthCredentials;

#define READ_TIMEOUT_SEC (5*60)
#define PORTAL_SSO_ENDPOINT "https://portal.singlestore.com/engine-sso"

int BrowserAuthInternal(MADB_Dbc *Dbc, char *email, char *endpoint, int requestReadTimeoutSec, BrowserAuthCredentials *credentials /*out*/);
void BrowserAuthCredentialsFree(BrowserAuthCredentials *credentials);
int BrowserAuth(MADB_Dbc *Dbc, char *email, BrowserAuthCredentials *credentials /*out*/)
{
  BrowserAuthInternal(Dbc, email, PORTAL_SSO_ENDPOINT, READ_TIMEOUT_SEC, credentials);
}

#endif//_browser_auth_h_
