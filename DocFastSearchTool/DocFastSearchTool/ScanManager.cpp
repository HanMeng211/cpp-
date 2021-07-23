#include"ScanManager.h"
#include"Sysutil.h"

ScanManager ScanManager::_sm_inst;

ScanManager::ScanManager()
{}

//��ʼɨ��
void ScanManager::StartScan(const string &path)
{

	while(1)
	{
		unique_lock<mutex> lock(m_mutex);
		m_cond.wait(lock);  //�����ȴ�
		ScanDirection(path);
	}
}

extern unsigned long g_file_count;
extern unsigned long g_scan_count;

bool ScanManager::DirectionWatch(const string &path)
{
	 //1ͨ���ļ�����
	 GetFileCount(path);
	 //2ͨ���ļ���
	 return g_scan_count != g_file_count;
}

void ScanManager::StartWatch(const string &path)
{
	g_file_count = GetFileCount(path);
	while(1)
	{
		g_scan_count = 0;
		//����ʱ��
		this_thread::sleep_for(std::chrono::seconds(3));
		bool isChange = DirectionWatch(path);
		if(isChange)
		{
			m_cond.notify_one(); //֪ͨɨ���߳�
			g_file_count = g_scan_count;
		}
	}
}

ScanManager& ScanManager::CreateScanManagerObj(const string &path)
{
	//����ɨ���߳�
	thread scan_th(&StartScan, &_sm_inst, path);
	scan_th.detach();

	//��������߳�
	thread watch_th(&StartWatch, &_sm_inst, path);
	watch_th.detach();

	return _sm_inst;
}


void ScanManager::ScanDirection(const string &path)
{
	//ɨ�豾���ļ�
	vector<string> local_files;
	vector<string> local_dirs;
	DirectionList(path, local_files, local_dirs);
	multiset<string> local_set;
	local_set.insert(local_files.begin(), local_files.end());
	local_set.insert(local_dirs.begin(), local_dirs.end());

	//ɨ�����ݿ��ļ�
	multiset<string> db_set;
	DataManager &m_dm = DataManager::GetDataManagerObj();
	m_dm.GetDocs(path, db_set);


	//ͬ�����ݿ�
	auto local_it = local_set.begin();
	auto db_it = db_set.begin();
	while(local_it!=local_set.end() && db_it!=db_set.end())
	{
		
		if(*local_it < *db_it)
		{
			//�������ļ������ݿ�û�У���������
			m_dm.InsertDoc(path, *local_it);
			++local_it;
		}
		else if(*local_it > *db_it)
		{
			//����û���ļ������ݿ��У�ɾ������
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

	//ɨ����Ŀ¼
	for(const auto &dir : local_dirs)
	{
		string dir_path = path;
		dir_path += "\\";
		dir_path += dir;
		ScanDirection(dir_path);
	}
}