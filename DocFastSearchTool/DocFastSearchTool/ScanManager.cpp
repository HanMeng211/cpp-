#include"ScanManager.h"
#include"Sysutil.h"

ScanManager ScanManager::_sm_inst;

ScanManager::ScanManager()
{}

//开始扫描
void ScanManager::StartScan(const string &path)
{

	while(1)
	{
		unique_lock<mutex> lock(m_mutex);
		m_cond.wait(lock);  //阻塞等待
		ScanDirection(path);
	}
}

extern unsigned long g_file_count;
extern unsigned long g_scan_count;

bool ScanManager::DirectionWatch(const string &path)
{
	 //1通过文件个数
	 GetFileCount(path);
	 //2通过文件名
	 return g_scan_count != g_file_count;
}

void ScanManager::StartWatch(const string &path)
{
	g_file_count = GetFileCount(path);
	while(1)
	{
		g_scan_count = 0;
		//休眠时间
		this_thread::sleep_for(std::chrono::seconds(3));
		bool isChange = DirectionWatch(path);
		if(isChange)
		{
			m_cond.notify_one(); //通知扫描线程
			g_file_count = g_scan_count;
		}
	}
}

ScanManager& ScanManager::CreateScanManagerObj(const string &path)
{
	//创建扫描线程
	thread scan_th(&StartScan, &_sm_inst, path);
	scan_th.detach();

	//创建监控线程
	thread watch_th(&StartWatch, &_sm_inst, path);
	watch_th.detach();

	return _sm_inst;
}


void ScanManager::ScanDirection(const string &path)
{
	//扫描本地文件
	vector<string> local_files;
	vector<string> local_dirs;
	DirectionList(path, local_files, local_dirs);
	multiset<string> local_set;
	local_set.insert(local_files.begin(), local_files.end());
	local_set.insert(local_dirs.begin(), local_dirs.end());

	//扫描数据库文件
	multiset<string> db_set;
	DataManager &m_dm = DataManager::GetDataManagerObj();
	m_dm.GetDocs(path, db_set);


	//同步数据库
	auto local_it = local_set.begin();
	auto db_it = db_set.begin();
	while(local_it!=local_set.end() && db_it!=db_set.end())
	{
		
		if(*local_it < *db_it)
		{
			//本地有文件，数据库没有，插入数据
			m_dm.InsertDoc(path, *local_it);
			++local_it;
		}
		else if(*local_it > *db_it)
		{
			//本地没有文件，数据库有，删除数据
			m_dm.DeleteDoc(path, *db_it);
			++db_it;
		}
		else
		{
			++local_it;
			++db_it;
		}
	}

	while(local_it != local_set.end())
	{
		m_dm.InsertDoc(path, *local_it);
		++local_it;
	}
	while(db_it != db_set.end())
	{
		m_dm.DeleteDoc(path, *db_it);
		++db_it;
	}

	//扫描子目录
	for(const auto &dir : local_dirs)
	{
		string dir_path = path;
		dir_path += "\\";
		dir_path += dir;
		ScanDirection(dir_path);
	}
}