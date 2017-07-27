#include "rgooroompss.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>

using namespace std;

RGooroomPss::RGooroomPss()
{
   isOpenDB = false;
   dbFile = nullptr;

   configPath = "/etc/gooroom/pss/";
   configName = "pss.conf";
   dbPath = "";

   score = {};
   config = {};

   if ( isConfigExists() )
      configParser("database");
      dbPath = configPath + config["name"];
      if ( !isOpenDB && dbPath != "")
         isOpenDB = connectDatabase();

      if ( isOpenDB )
         setScore();
}

RGooroomPss::~RGooroomPss() {}

RGooroomPss *RGooroomPss::instance = nullptr;

RGooroomPss *RGooroomPss::getInstance()
{
   if (instance == NULL)
      instance = new RGooroomPss();

   return instance;
}

bool RGooroomPss::isConfigExists()
{
   struct stat buffer;

   return (stat((configPath+configName).c_str(), &buffer) == 0);
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

void RGooroomPss::configParser(string section)
{
    ifstream configFile (configPath + configName);
    string line, key, value;
    string token = " =";
    if (configFile.is_open())
    {
        while (getline (configFile, line))
        {
            unsigned long len = line.length();
            if (line.at(0) == '[' && line.at(len-1) == ']')
            {
                if (line.substr(1, line.length()-2).compare(section) == 0)
                {
                    while (getline (configFile, line))
                    {
                        len = line.length();
                        if (line.at(0) == '[' && line.at(len-1) == ']')
                            break;

                        int nCutPos;
                        int nIndex = 0;
                        string *strResult = new string[5];
                        while ((nCutPos = line.find_first_of(token)) != line.npos)
                        {
                            if (nCutPos > 0)
                            {
                                strResult[nIndex++] = line.substr(0, nCutPos);
                            }
                            line = line.substr(nCutPos+1);
                        }

                        if(line.length() > 0)
                        {
                            strResult[nIndex++] = line.substr(0, nCutPos);
                        }
                        key = strResult[0];
                        value = strResult[1];
                        config.insert(StrToStrMap::value_type(key, value));
                    }
                }
            }
        }
        configFile.close();
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
            dataStr = (char*)sqlite3_column_name(statement, i);
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
