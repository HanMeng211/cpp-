#pragma once

#include"Common.h"

//��װsqlite
class SqliteManager
{
public:
	SqliteManager();
	~SqliteManager();
public:
	void Open(const string &path);     //��/�������ݿ�
	void Close();                      //�ر����ݿ�
	void ExecuteSql(const string &sql);//ִ��sql
	void GetResultTable(const string &sql, char **&ppRet, int &row, int &col);
private:
	sqlite3 *m_db;
};

//�Զ���ȡ�����  RAII 
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


//��װ���ݹ�����
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
							   string &prefix, string &highlight, string &suffix);    //��ʼ�����ݿ�
public:
	void InsertDoc(const string &path, const string &doc);      //�����ļ�
	void DeleteDoc(const string &path, const string &doc);      //ɾ���ļ�
	void GetDocs(const string &path, multiset<string> &docs);   //��ȡ�ļ�
private:
	DataManager();
	~DataManager();
private:
	SqliteManager m_dbmgr;
	static DataManager dm_inst;
};

