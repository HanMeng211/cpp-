#pragma once

#include"Common.h"

//封装sqlite
class SqliteManager
{
public:
	SqliteManager();
	~SqliteManager();
public:
	void Open(const string &path);     //打开/创建数据库
	void Close();                      //关闭数据库
	void ExecuteSql(const string &sql);//执行sql
	void GetResultTable(const string &sql, char **&ppRet, int &row, int &col);
private:
	sqlite3 *m_db;
};

//自动获取结果表  RAII 
class AutoGetResultTable
{
public:
	AutoGetResultTable(SqliteManager *db, const string &sql, 
					   char **&ppRet, int &row, int &col);
	~AutoGetResultTable();
public:
	AutoGetResultTable(const AutoGetResultTable &) = delete;
	AutoGetResultTable& operator=(const AutoGetResultTable &)=delete;
private:
	SqliteManager *m_db;
	char **m_ppRet;
};


//封装数据管理类
class DataManager
{
public:
	static DataManager& GetDataManagerObj();
public:
	void InitSqlite();
public:
	void Search(const string &key, vector<pair<string,string>> &doc_path);
public:
	static void SplitHighlight(const string &str, const string &key,
							   string &prefix, string &highlight, string &suffix);    //初始化数据库
public:
	void InsertDoc(const string &path, const string &doc);      //插入文件
	void DeleteDoc(const string &path, const string &doc);      //删除文件
	void GetDocs(const string &path, multiset<string> &docs);   //获取文件
private:
	DataManager();
	~DataManager();
private:
	SqliteManager m_dbmgr;
	static DataManager dm_inst;
};

