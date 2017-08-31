#ifndef _RGOOROOMPSS_H
#define _RGOOROOMPSS_H

#include <map>
#include <sqlite3.h>
#include <string>
#include <vector>

using namespace std;

typedef map<string, string> StrToStrMap;

class RGooroomPss {
 private:
   RGooroomPss();
   RGooroomPss(const RGooroomPss& other);
   ~RGooroomPss();

   static RGooroomPss *instance;

   bool isOpenDB;
   sqlite3 *dbFile;

   string configPath;
   string configName;
   string dbPath;
   string dbName;

   StrToStrMap score;
   StrToStrMap config;

   bool isFileExists(string path);

   bool connectDatabase();
   void disconnectDatabase();

   void setScore();
 public:
   static RGooroomPss *getInstance();
   bool getIsOpenDB() { return isOpenDB; };
   string getScore(const char *pkg);
   vector<string> getAllInfo(const char *pkg);
};
#endif
