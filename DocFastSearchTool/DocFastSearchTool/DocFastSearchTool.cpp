#include"ScanManager.h"
#include"Sysutil.h"
#include"SysFrame.h"

char *title = "文档快速搜索工具";


int main(int argc, char *argv[])
{
	//const string &path = "C:\\Bit\\Book\\比特科技\\阶段性考试试卷";
	const string &path = "F:\\比特";
	
	//创建扫描对象
	ScanManager &sm = ScanManager::CreateScanManagerObj(path);
	//sm.ScanDirection(path);

	//创建搜索对象
	DataManager &dm = DataManager::GetDataManagerObj();
	string key;
	vector<pair<string,string>> doc_path;
	while(1)
	{
		DrawFrame(title);
		DrawMenu();

		//cout<<"请输入要搜索的关键字:>";
		cin>>key;
		if (key == string("exit"))
			break;

		dm.Search(key, doc_path);

		int init_row = 5; //由界面决定
		int count = 0;  

		string prefix, highlight, suffix;
		for(const auto &e : doc_path)
		{
			string doc_name = e.first;
			string doc_path = e.second;

			DataManager::SplitHighlight(doc_name, key, prefix, highlight, suffix);

			SetCurPos(2, init_row+count++);
			cout<<prefix;
			ColourPrintf(highlight.c_str());
			cout<<suffix;
			SetCurPos(33, init_row+count - 1);
			printf("%-50s", doc_path.c_str());
		}
		doc_path.clear();
		SystemEnd();
		system("pause");
	}
	SystemEnd();
	return 0;
}
