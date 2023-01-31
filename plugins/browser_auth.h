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
  int64_t expiration;
} BrowserAuthCredentials;

#define READ_TIMEOUT_USER_SEC (5*60)
#define READ_TIMEOUT_TEST_SEC 10

#define PORTAL_SSO_ENDPOINT "https://portal.singlestore.com/engine-sso"
#define LOCAL_TEST_ENDPOINT "http://127.0.0.1:18087"

int BrowserAuth(MADB_Dbc *Dbc, const char *email, BrowserAuthCredentials *credentials /*out*/, int testFlags);
void BrowserAuthCredentialsFree(BrowserAuthCredentials *credentials);
int parseJWTToCredentials(MADB_Dbc *Dbc, const char *token, int token_len, BrowserAuthCredentials *bac /*out*/);

#if defined(_WIN32)
// Win secure key storage
// https://docs.microsoft.com/en-us/windows/win32/api/wincred/
#include <wincred.h>
#define SECURE_JWT_STORAGE_KEY L"SingleStore ODBC Driver JWT storage"
#elif defined(__APPLE__)
// Mac secure key storage
// https://developer.apple.com/documentation/security/keychain_services/keychain_items?language=objc
#include <Security/Security.h>
#include <Security/SecKeyChain.h>
#include <Security/SecKeyChainItem.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBase.h>

#define SECURE_JWT_STORAGE_KEY "SingleStore ODBC Driver JWT storage"
#else
// Linux secure key storage
// https://gnome.pages.gitlab.gnome.org/libsecret/
#include <glib.h>
#include <gmodule.h>
#include <libsecret/secret.h>
#define SECURE_JWT_STORAGE_KEY "SingleStore ODBC Driver JWT storage"
const SecretSchema *jwt_cache_get_schema(void);
#define JWT_CACHE_SCHEMA jwt_cache_get_schema ()

struct libsecret_functions 
{
SecretCollection* (*secret_collection_for_alias_sync) (SecretService *service,
                                                      const gchar *alias,
                                                      SecretCollectionFlags flags,
                                                      GCancellable *cancellable,
                                                      GError **error);
gint (*secret_service_unlock_sync) (SecretService *service,
                                    GList *objects,
                                    GCancellable *cancellable,
                                    GList **unlocked,
                                    GError **error);
SecretService* (*secret_service_get_sync) (SecretServiceFlags flags,
                                           GCancellable *cancellable,
                                           GError **error);
gboolean (*secret_collection_get_locked) (SecretCollection *self);
gchar* (*secret_password_lookup_sync)(const SecretSchema *schema,
                                      GCancellable *cancellable,
                                      GError **error,
                                      ...) G_GNUC_NULL_TERMINATED;
void (*secret_password_free) (gchar *password);
gboolean (*secret_password_store_sync) (const SecretSchema *schema,
                                        const gchar *collection,
                                        const gchar *label,
                                        const gchar *password,
                                        GCancellable *cancellable,
                                        GError **error,
                                        ...) G_GNUC_NULL_TERMINATED;
};

struct libglib_functions 
{
  void (*g_error_free) (GError *error);
  GList* (*g_list_append) (GList *list, gpointer data);
};
#endif

int PutCachedCredentials(MADB_Dbc *Dbc, BrowserAuthCredentials *bac);
int GetCachedCredentials(MADB_Dbc *Dbc, const char *username, BrowserAuthCredentials *bac /*out*/);
int GetCredentialsBrowserSSO(MADB_Dbc *Dbc, MADB_Dsn *Dsn /*out*/, const char* email, my_bool readCached);

#define BROWSER_AUTH_FLAG_TEST_FIRST_CALL (1 << 1)
#define BROWSER_AUTH_FLAG_TEST_SECOND_CALL (1 << 2)
#define BROWSER_AUTH_FLAG_TEST_ENDPOINT (1 << 3)
#define BROWSER_AUTH_FLAG_TEST_SHORT_TIMEOUT (1 << 4)

#endif //_browser_auth_h_
