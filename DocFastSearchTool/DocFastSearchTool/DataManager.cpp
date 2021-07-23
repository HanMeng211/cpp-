#include"DataManager.h"
#include"Sysutil.h"
#include"Config.h"

#pragma comment(lib, "./sqlite/sqlite3.lib")   //引入静态库

SqliteManager::SqliteManager():m_db(nullptr)
{}
SqliteManager::~SqliteManager()
{
	Close();
}

//打开/创建数据库
void SqliteManager::Open(const string &path)
{
	int rc = sqlite3_open(path.c_str(), &m_db);
	if (rc != SQLITE_OK)
	{
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_db));
		ERROR_LOG("Can't open database: %s\n", sqlite3_errmsg(m_db));
		exit(0);
	}
	else
		//fprintf(stderr, "Opened database successfully\n");
		TRACE_LOG( "Opened database successfully\n");
}

//关闭数据库
void SqliteManager::Close()
{
	if(m_db)
	{
		int rc = sqlite3_close(m_db);
		if (rc != SQLITE_OK)
		{
			//fprintf(stderr, "Can't close database: %s\n", sqlite3_errmsg(m_db));
			ERROR_LOG("Can't close database: %s\n", sqlite3_errmsg(m_db));
			exit(0);
		}
		else
			//fprintf(stderr, "Closeed database successfully\n");
			TRACE_LOG("Closeed database successfully\n");
		m_db = nullptr;
	}
}

//执行sql
void SqliteManager::ExecuteSql(const string &sql)
{

	char *zErrMsg = 0;
	int rc = sqlite3_exec(m_db, sql.c_str(), 0, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		ERROR_LOG("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		//fprintf(stdout, "Operation done successfully\n");
		TRACE_LOG("Operation done successfully\n");
	}
}

//获取结果表
void SqliteManager::GetResultTable(const string &sql, char **&ppRet, int &row, int &col)
{
	char *zErrMsg = 0;
	int rc = sqlite3_get_table(m_db, sql.c_str(), &ppRet, &row, &col, &zErrMsg);
	if(rc != SQLITE_OK)
	{
		 //fprintf(stderr, "get result table error: %s\n", sqlite3_errmsg(m_db));
		ERROR_LOG("get result table error: %s\n", sqlite3_errmsg(m_db));
		 exit(0);
	}
	else
		//fprintf(stderr, "Get result table successfully\n");
		TRACE_LOG("Get result table successfully\n");
}


//自动获取结果表
AutoGetResultTable::AutoGetResultTable(SqliteManager *db, const string &sql, 
					   char **&ppRet, int &row, int &col) : m_db(db)
{
	m_db->GetResultTable(sql, ppRet, row, col);
	m_ppRet = ppRet;
}
AutoGetResultTable::~AutoGetResultTable()
{
	if(m_ppRet)
		sqlite3_free_table(m_ppRet);
}




//数据管理类

DataManager DataManager::dm_inst;

DataManager& DataManager::GetDataManagerObj()
{
	
	return dm_inst;
}

DataManager::DataManager()
{
	//创建数据库
	m_dbmgr.Open(DOC_DB);
	//创建表
	InitSqlite();
}
DataManager::~DataManager()
{
	m_dbmgr.Close();
}

//初始化数据库  创建表
void DataManager::InitSqlite()
{
	char sql[SQL_BUFFER_SIZE] = {0}; 
	sprintf(sql, "create table if not exists %s (id INTEGER PRIMARY KEY autoincrement, doc_name text, doc_path text, doc_pinyin text, doc_initials text)",
			DOC_TABLE);
	m_dbmgr.ExecuteSql(sql);
}

//插入文件
void DataManager::InsertDoc(const string &path, const string &doc)
{
	char sql[SQL_BUFFER_SIZE] = {0};

	string doc_pinyin = ChineseConvertPinYinAllSpell(doc);
	string doc_initials = ChineseConvertPinYinInitials(doc);
	//ID  文件名  路径  拼音  首字母
	sprintf(sql, "insert into %s values(null, '%s', '%s', '%s', '%s')",
			DOC_TABLE, doc.c_str(), path.c_str(), doc_pinyin.c_str(), doc_initials.c_str());
	m_dbmgr.ExecuteSql(sql);
}

//删除文件
void DataManager::DeleteDoc(const string &path, const string &doc)
{
	char sql[SQL_BUFFER_SIZE] = {0};
	sprintf(sql, "delete from %s where doc_path='%s' and doc_name='%s'",
			DOC_TABLE, path.c_str(), doc.c_str());
	m_dbmgr.ExecuteSql(sql);


	//目录的级联删除
	memset(sql, 0,SQL_BUFFER_SIZE);
	string doc_path = path;
	doc_path += "\\";
	doc_path += doc;
	sprintf(sql, "delete from %s where doc_path like '%s%%'",
			DOC_TABLE, doc_path.c_str());     //以路径匹配，匹配%s目录下的文件
	m_dbmgr.ExecuteSql(sql);
}

//获取文件
void DataManager::GetDocs(const string &path, multiset<string> &docs)
{
	char sql[SQL_BUFFER_SIZE] = {0};
	//sprintf(sql, "select doc_name from %s where doc_path='%s'",
	//	DOC_TABLE, path.c_str());

	sprintf(sql, "select doc_name from %s where doc_path='%s'",
	        DOC_TABLE, path.c_str());

	int row=0, col=0;
	char **ppRet = 0;
	//m_dbmgr.GetResultTable(sql, ppRet, row, col);
	AutoGetResultTable at(&m_dbmgr, sql, ppRet, row, col);

	for(int i=1; i<=row; ++i)
		docs.insert(ppRet[i]);

	//释放结果表
	//sqlite3_free_table(ppRet);
}

//搜索
void DataManager::Search(const string &key, vector<pair<string,string>> &doc_path)
{
	char sql[SQL_BUFFER_SIZE] = {0};                      
	
	//将文件名转换为拼音
	string key_pinyin = ChineseConvertPinYinAllSpell(key);
	//将文件名转换为首字母
	string key_initials = ChineseConvertPinYinInitials(key);
	
	//%%s%                                                 //名字模糊匹配                 //拼音模糊搜素                    //首字母模糊搜素
	sprintf(sql, "select doc_name, doc_path from %s where doc_name like '%%%s%%' or doc_pinyin like '%%%s%%' or doc_initials like '%%%s%%'",
			DOC_TABLE, key.c_str(), key_pinyin.c_str(), key_initials.c_str());
	
	int row=0, col=0;
	char **ppRet = nullptr;
	//m_dbmgr.GetResultTable(sql, ppRet, row, col);
	AutoGetResultTable at(&m_dbmgr, sql, ppRet, row, col);

	doc_path.clear();

	for(int i=1; i<=row; ++i)
		doc_path.push_back(make_pair(ppRet[i*col], ppRet[i*col+1]));

	//释放表结果
	//sqlite3_free_table(ppRet);
}

void DataManager::SplitHighlight(const string &str, const string &key,
							   string &prefix, string &highlight, string &suffix)

{
	string strlower(str), keylower(key);
	transform(str.begin(), str.end(), strlower.begin(), tolower);
	transform(key.begin(), key.end(), keylower.begin(), tolower);

	//1 原始字符串中能够查询到key
	size_t pos = strlower.find(keylower);
	if(pos != string::npos)
	{
		prefix = strlower.substr(0, pos);
		highlight = strlower.substr(pos, keylower.size());
		suffix = strlower.substr(pos+keylower.size(), string::npos);
		return;
	}

	//2 全拼搜索
	string str_pinyin = ChineseConvertPinYinAllSpell(strlower);
	string key_pinyin = ChineseConvertPinYinAllSpell(keylower);
	pos = str_pinyin.find(key_pinyin);
	if(pos != string::npos)
	{
		int str_index = 0;
		int pinyin_index = 0;
		int highlight_index = 0;
		int highlight_len = 0;

		bool first_set = true;
		while(str_index < strlower.size())
		{
			if(pinyin_index >= pos && first_set)
			{
				highlight_index = str_index;
				first_set = false;
			}
			if(pinyin_index >= pos+key_pinyin.size())
			{
				highlight_len = str_index - highlight_index;
				break;
			}

			if(strlower[str_index]>=0  && strlower[str_index]<=127)
			{
				++str_index;
				++pinyin_index;
			}
			else
			{
				string substr(strlower, str_index, 2);
				string sub_pinyin = ChineseConvertPinYinAllSpell(substr);
				str_index += 2;
				pinyin_index += sub_pinyin.size();
			}
		}

		prefix = str.substr(0, highlight_index);
		highlight = str.substr(highlight_index, highlight_len);
		suffix = str.substr(highlight_index+highlight_len, string::npos);
		return;
	}

	//3 首字母搜索
	string str_initials = ChineseConvertPinYinInitials(strlower);
	string key_initials = ChineseConvertPinYinInitials(keylower);
	pos = str_initials.find(key_initials);
	if(pos != string::npos)
	{
		int str_index = 0;
		int initials_index = 0;
		int highlight_index = 0;
		int highlight_len = 0;
		
		bool first_set = true;
		while(str_index < strlower.size())
		{
			if(initials_index >= pos && first_set)
			{
				highlight_index = str_index;
				first_set = false;
			}
			if(initials_index >= pos + key_initials.size())
			{
				highlight_len = str_index - highlight_index;
				break;
			}
			if(strlower[str_index]>=0 && strlower[str_index]<=127)
			{
				++str_index;
				++initials_index;
			}
			else
			{
				str_index += 2;
				++initials_index;
			}
		}
		prefix = str.substr(0, highlight_index);
		highlight = str.substr(highlight_index, highlight_len);
		suffix = str.substr(highlight_index+highlight_len, string::npos);
		return;
	}

	//未能搜索到
	prefix = str;
	highlight.clear();
	suffix.clear();
}