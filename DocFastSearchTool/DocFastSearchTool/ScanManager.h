#pragma once

#include"DataManager.h"

class ScanManager
{
public:
	static ScanManager& CreateScanManagerObj(const string &path);
public:
	void StartScan(const string &path);           //ɨ��Ŀ¼
	bool DirectionWatch(const string &path);
	void StartWatch(const string &path);
	void ScanDirection(const string &path);
private:
	ScanManager();
private:
	//DataManager m_dm;
	static ScanManager _sm_inst;
	mutex m_mutex;
	condition_variable m_cond;//��������
};

