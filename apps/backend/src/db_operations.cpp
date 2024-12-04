
#include <stdio.h>
#include <stdint.h>
#include <sqlite/sqlite3.h>
#include <hiddenframe_headers.h>
#include <stdexcept>
#include <string>
#include <iostream>
#include <vector>
#include <tuple>
#include <crow/json.h>
#include <mutex>

using namespace std;

sqlite3 *createDB(const string &filepath)
{
  sqlite3 *db;
  int rc = sqlite3_open_v2(filepath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (rc)
  {
    throw runtime_error("Database not opened correctly - " + string(sqlite3_errmsg(db)));
  }

  const char *usersSql = "CREATE TABLE IF NOT EXISTS Users("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                         "username TEXT NOT NULL,"
                         "token_id TEXT,"
                         "invites_created INTEGER DEFAULT 0,"
                         "password TEXT NOT NULL);";

  const char *invitesSql = "CREATE TABLE IF NOT EXISTS Invites("
                           "id INTEGER PRIMARY KEY NOT NULL,"
                           "is_valid BOOLEAN,"
                           "created_by INTEGER,"
                           "used_by INTEGER,"
                           "FOREIGN KEY (created_by) REFERENCES Users(id) ON DELETE CASCADE,"
                           "FOREIGN KEY (used_by) REFERENCES Users(id) ON DELETE CASCADE);";

  char *errMsg = nullptr;
  sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
  rc = sqlite3_exec(db, usersSql, 0, 0, &errMsg);
  if (rc != SQLITE_OK)
  {
    sqlite3_free(errMsg);
    throw runtime_error("Failed to execute SQL - " + string(sqlite3_errmsg(db)));
  }
  sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
  sqlite3_free(errMsg);

  sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
  rc = sqlite3_exec(db, invitesSql, 0, 0, &errMsg);
  if (rc != SQLITE_OK)
  {
    sqlite3_free(errMsg);
    throw runtime_error("Failed to execute SQL - " + string(sqlite3_errmsg(db)));
  }
  sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
  sqlite3_free(errMsg);

  return db;
}

void closeDB(sqlite3 *db)
{
  if (db)
  {
    sqlite3_db_cacheflush(db);
    sqlite3_close(db);
  }
}

bool usernameExists(sqlite3 *db, const string &username)
{
  sqlite3_stmt *checkStmt;
  const char *checkSql = "SELECT COUNT(*) FROM Users WHERE username = ?;";

  int rc = sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, NULL);
  if (rc != SQLITE_OK)
  {
    if (checkStmt)
      sqlite3_finalize(checkStmt);
    throw runtime_error("Failed to prepare statement for checking duplicate username - " + string(sqlite3_errmsg(db)));
  }

  sqlite3_bind_text(checkStmt, 1, username.c_str(), -1, SQLITE_STATIC);
  rc = sqlite3_step(checkStmt);

  if (rc == SQLITE_ROW)
  {
    int count = sqlite3_column_int(checkStmt, 0);
    sqlite3_finalize(checkStmt);
    return count > 0;
  }
  else
  {
    if (checkStmt)
      sqlite3_finalize(checkStmt);
    throw runtime_error("Error checking for username - " + string(sqlite3_errmsg(db)));
  }
}

void executeNonQuery(sqlite3 *db, const char *sql, const vector<pair<int, string>> &bindings)
{
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
  {
    throw runtime_error("Failed to prepare statement - " + string(sqlite3_errmsg(db)));
  }

  for (const auto &bind : bindings)
  {
    sqlite3_bind_text(stmt, bind.first, bind.second.c_str(), -1, SQLITE_STATIC);
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE)
  {
    sqlite3_finalize(stmt);
    throw runtime_error("Execution failed - " + string(sqlite3_errmsg(db)));
  }
  sqlite3_finalize(stmt);
}

void createNewAdmin(const string &username, const string &password)
{
  sqlite3 *db = createDB("./database/userdatabase.db");

  if (username.empty() || password.empty())
  {
    closeDB(db);
    throw runtime_error("Password and/or username cannot be empty");
  }

  if (usernameExists(db, username))
  {
    closeDB(db);
    throw runtime_error("User already exists");
  }

  const char *sql = "INSERT INTO Users (username, password) VALUES (?, ?);";
  vector<pair<int, string>> bindings{{1, username}, {2, password}};

  sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
  executeNonQuery(db, sql, bindings);
  sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

  closeDB(db);
}

void createNewUser(const string &username, const string &password, const int64_t inviteID)
{
  sqlite3 *db = createDB("./database/userdatabase.db");

  if (username.empty() || password.empty())
  {
    closeDB(db);
    throw runtime_error("Password and/or username cannot be empty");
  }

  if (usernameExists(db, username))
  {
    closeDB(db);
    throw runtime_error("User already exists");
  }

  sqlite3_stmt *stmt;
  const char *inviteSql = "SELECT is_valid FROM Invites WHERE id = ?;";
  int rc = sqlite3_prepare_v2(db, inviteSql, -1, &stmt, NULL);
  if (rc != SQLITE_OK || !stmt)
  {
    if (stmt)
      sqlite3_finalize(stmt);
    closeDB(db);
    throw runtime_error("Failed to prepare statement for invite check - " + string(sqlite3_errmsg(db)));
  }

  sqlite3_bind_int64(stmt, 1, inviteID);
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW)
  {
    if (sqlite3_column_int(stmt, 0) == 0)
    {
      sqlite3_finalize(stmt);
      closeDB(db);
      throw runtime_error("Invite ID is not valid.");
    }
  }
  else
  {
    sqlite3_finalize(stmt);
    closeDB(db);
    throw runtime_error("No such invite ID exists.");
  }
  sqlite3_finalize(stmt);

  const char *userSql = "INSERT INTO Users (username, password) VALUES (?, ?);";
  vector<pair<int, string>> userBindings{{1, username}, {2, password}};

  sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
  executeNonQuery(db, userSql, userBindings);
  int newUserID = sqlite3_last_insert_rowid(db);

  const char *updateSql = "UPDATE Invites SET is_valid = 0, used_by = ? WHERE id = ?;";
  vector<pair<int, string>> updateBindings{{1, to_string(newUserID)}, {2, to_string(inviteID)}};
  executeNonQuery(db, updateSql, updateBindings);
  sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

  closeDB(db);
}

int64_t createInvite(const string &username,int64_t id)
{
  sqlite3 *db = createDB("./database/userdatabase.db");
  if (!db)
  {
    throw runtime_error("Database not opened correctly - " + string(sqlite3_errmsg(db)));
  }

  sqlite3_stmt *stmt;
  const char *sql0 = "SELECT id, invites_created FROM Users WHERE username = ?;";
  int rc = sqlite3_prepare_v2(db, sql0, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
  {
    if (stmt)
      sqlite3_finalize(stmt);
    closeDB(db);
    throw runtime_error("Failed to prepare statement - " + string(sqlite3_errmsg(db)));
  }

  sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW)
  {
    int userID = sqlite3_column_int(stmt, 0);
    int invitesCreated = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    if (invitesCreated >= 5)
    {
      closeDB(db);
      return -1; // No invites remaining
    }
    const char *inviteSql = "INSERT INTO Invites (id, is_valid, created_by, used_by) VALUES (?, 1, ?, NULL);";
    vector<pair<int, string>> inviteBindings{{1,to_string(id)},{2, to_string(userID)}};

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    executeNonQuery(db, inviteSql, inviteBindings);

    const char *updateSql = "UPDATE Users SET invites_created = invites_created + 1 WHERE id = ?;";
    vector<pair<int, string>> updateBindings{{1, to_string(userID)}};
    executeNonQuery(db, updateSql, updateBindings);
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

    int64_t inviteID = sqlite3_last_insert_rowid(db);
    closeDB(db);
    return inviteID;
  }
  else
  {
    sqlite3_finalize(stmt);
    closeDB(db);
    throw runtime_error("User not found.");
  }
}

vector<crow::json::wvalue> listInvites(const int &userId)
{
  sqlite3 *db = createDB("./database/userdatabase.db");
  if (!db)
  {
    throw runtime_error("Database not opened correctly - " + string(sqlite3_errmsg(db)));
  }

  sqlite3_stmt *stmt;
  const char *sql = "SELECT id, is_valid, created_by, used_by FROM Invites WHERE created_by = ?;";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
  {
    closeDB(db);
    throw runtime_error("Failed to prepare statement - " + string(sqlite3_errmsg(db)));
  }

  sqlite3_bind_int(stmt, 1, userId);
  vector<crow::json::wvalue> invites;

  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    crow::json::wvalue invite;
    invite["id"] = sqlite3_column_int64(stmt, 0);
    invite["is_valid"] = sqlite3_column_int(stmt, 1);
    invite["created_by"] = sqlite3_column_int(stmt, 2);
    invite["used_by"] = sqlite3_column_int(stmt, 3);

    invites.push_back(invite);
  }
  sqlite3_finalize(stmt);

  closeDB(db);
  return invites;
}

bool changePassword(const string &username, const string &newPassword)
{
  sqlite3 *db = createDB("./database/userdatabase.db");
  const char *sql = "UPDATE Users SET password = ?, token_id = NULL WHERE username = ?;";
  vector<pair<int, string>> bindings{{1, newPassword}, {2, username}};

  sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
  executeNonQuery(db, sql, bindings);
  sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

  return true;
}

bool authenticateUser(const string &username, const string &password)
{
  sqlite3 *db = createDB("./database/userdatabase.db");

  sqlite3_stmt *stmt;
  const char *sql = "SELECT password FROM Users WHERE username = ?;";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
  {
    if (stmt)
      sqlite3_finalize(stmt);
    closeDB(db);
    throw runtime_error("Failed to prepare statement - " + string(sqlite3_errmsg(db)));
  }

  sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);

  if (rc == SQLITE_ROW)
  {
    const unsigned char *dbPassword = sqlite3_column_text(stmt, 0);
    bool isAuthenticated = password == reinterpret_cast<const char *>(dbPassword);
    sqlite3_finalize(stmt);
    return isAuthenticated;
  }
  sqlite3_finalize(stmt);
  closeDB(db);
  return false;
}

pair<int, string> verifyTokenWithDb(const string &tokenId)
{
  sqlite3 *db = createDB("./database/userdatabase.db");

  sqlite3_stmt *stmt;
  const char *sql = "SELECT id, username FROM Users WHERE token_id = ?;";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
  {
    if (stmt)
      sqlite3_finalize(stmt);
    closeDB(db);
    throw runtime_error("Failed to prepare statement - " + string(sqlite3_errmsg(db)));
  }

  sqlite3_bind_text(stmt, 1, tokenId.c_str(), -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);

  if (rc == SQLITE_ROW)
  {
    int userID = sqlite3_column_int(stmt, 0);
    const unsigned char *username = sqlite3_column_text(stmt, 1);
    string usernameStr(reinterpret_cast<const char *>(username));
    sqlite3_finalize(stmt);
    return make_pair(userID, usernameStr);
  }
  sqlite3_finalize(stmt);
  closeDB(db);
  throw runtime_error("Token not found.");
}

void saveToken(const string &username, const string &tokenId)
{
  sqlite3 *db = createDB("./database/userdatabase.db");

  const char *sql = "UPDATE Users SET token_id = ? WHERE username = ?;";
  vector<pair<int, string>> bindings{{1, tokenId}, {2, username}};

  sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
  executeNonQuery(db, sql, bindings);
  sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

  closeDB(db);
}