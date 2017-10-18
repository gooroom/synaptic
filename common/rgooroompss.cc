#include "rgooroompss.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include "INIReader.h"
#include "sections_trans.h"

using namespace std;

RGooroomPss::RGooroomPss()
{
   isOpenDB = false;
   dbFile = nullptr;

   configPath = "/etc/gooroom/pss/";
   configName = "pss.conf";
   dbPath = "";
   dbName = "";

   if ( isFileExists(configPath + configName) )
   {
      INIReader reader(configPath + configName);
      dbName = reader.Get("database", "name", "UNKNOWN");

      if ( isFileExists(configPath + dbName) && dbName.compare("UNKNOWN") != 0 )
      {
         dbPath = configPath + dbName;
         if ( !isOpenDB && dbPath != "")
            isOpenDB = connectDatabase();

         if ( isOpenDB )
            setScore();
      }
   }
}

RGooroomPss::~RGooroomPss() {}

RGooroomPss *RGooroomPss::instance = nullptr;

RGooroomPss *RGooroomPss::getInstance()
{
   if (instance == NULL)
      instance = new RGooroomPss();

   return instance;
}

bool RGooroomPss::isFileExists(string path)
{
   struct stat buffer;

   return (stat((path).c_str(), &buffer) == 0);
}

bool RGooroomPss::connectDatabase()
{
   if (sqlite3_open(dbPath.c_str(), &dbFile) == SQLITE_OK)
   {
      return true;
   }
   return false;
}

void RGooroomPss::disconnectDatabase()
{
   if (isOpenDB == true)
   {
      sqlite3_close(dbFile);
      isOpenDB = false;
   }
}

void RGooroomPss::setScore()
{
   sqlite3_stmt *statement;

   string query = string("SELECT package_name, cve_score "
      "FROM pss_output "
      "GROUP BY package_name "
      "ORDER BY package_version DESC");

   if (sqlite3_prepare(dbFile, query.c_str(), -1, &statement, 0) == SQLITE_OK)
   {
      // int ctotal = sqlite3_column_count(statement);
      while ( true )
      {
         int res = sqlite3_step(statement);
         if ( res == SQLITE_ROW )
         {
            string key = (char*)sqlite3_column_text(statement, 0);
            string value = (char*)sqlite3_column_text(statement, 1);
            score.insert(map<string, string>::value_type(key, value));
         }

         if ( res == SQLITE_DONE || res==SQLITE_ERROR)
         {
               break;
         }
      }
   }
}

string RGooroomPss::getScore(const char *pkg)
{
   return score[pkg];
}

vector<string>
RGooroomPss::getAllInfo(const char *pkg)
{
   if ( isOpenDB == false )
   {
      isOpenDB = connectDatabase();
   }
   vector<string> dataStrings;
   string dataStr;

   // get from database
   sqlite3_stmt *statement;
   string query = string("SELECT * FROM pss_output WHERE package_name = '") + string(pkg) + "'";
   if (sqlite3_prepare(dbFile, query.c_str(), -1, &statement, 0 ) == SQLITE_OK)
   {
      int ctotal = sqlite3_column_count(statement);
      int res = 0;
      res = sqlite3_step(statement);

      if ( res == SQLITE_ROW )
      {
         for ( int i = 0; i < ctotal; i++ )
         {
            dataStr = (_((char*)sqlite3_column_name(statement, i)));
            dataStrings.push_back(dataStr);
            dataStr = ((char*)sqlite3_column_text(statement, i) ? (char*)sqlite3_column_text(statement, i) : "");
            dataStrings.push_back(dataStr);
         }
      } else {
         dataStr = "No";
         dataStrings.push_back(dataStr);
         dataStr = "Data";
         dataStrings.push_back(dataStr);
      }
   } else {
      dataStr = "No";
      dataStrings.push_back(dataStr);
      dataStr = "Data";
      dataStrings.push_back(dataStr);
   }

   return dataStrings;
}
