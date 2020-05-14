#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#define INF 2147483647

using namespace std;

string string_cr(int amount, string str){ //������� ��������� str amount ���
	string result = "";
	for (int i = 0; i < amount; i++)
		result += str;
	return result;
}
char return_letter(int num){
	return (char)65 + num;
}
int return_number(char s){
	return (int)s - 64;
}

class Detail{			//��������� ������ ������
public:

	vector<int> times;				//����� ������� ��������� ������
	vector<int> ways;				//��������������� ������� ������

	void setTime(int time){
		times.push_back(time);
	}
	void delWayTime(){
		times.erase(times.begin());
		ways.erase(ways.begin());
	}
	void setWay(int tool){
		ways.push_back(tool);
	}
};

class Tool{			//��������� ������ ������
public:
	int currDetail = -1;			//����� ������� ������, �������������� �� ������
	int workTime;					//����� ������ ������
	unsigned int downTime;			//����� ������� ������
	string GantGraphic;				//������, ���������� ������ �����
	vector<string> *queue_graphics;	//������� �������� (� ���� ������ ��� ������)
	vector<int> queue_details;		//������ �������, ����������� � ������� �� ������ ������
	Tool(){
		workTime = 0;
		downTime = 0;
		GantGraphic = "";
		queue_graphics = new vector<string>(1, "");
	}
	~Tool(){
		delete queue_graphics;
	}
};

class Manufacture{		//��������� ������ �����
private:
	int ruleNumber;					//����� ������� �� ������ ������ ��� ���������
	int amount_details;				//���������� �������
	int amount_tools;				//���������� �������
	int totalTime;					//����� ����� ������
	vector<Detail> *details;		//������ �� �������
	vector<Tool> *tools;			//������ �� �������
	int ** rest_time;				//������������ ������� �� ������ �� �������
	vector<int> *done_time;			//�����, ��� ������ ������, ������������ ����� ���������������� �����
public:
	void showInfo();				//����� ������� ����� � �������� ��� ���� �������
	void showInfo(string filename);	//���������� ������� ����� � �������� � ����
	int nextDetail(vector<int> *queue_to_machine);//����� ��������� ������ ��� ��������� � ������������ � ��������
	void doWork();					//������������� ���� ������� � ������������ � �� ���������
	vector<int> getDownTime();		//���������� ����� ������� ���� �������
	int getTotalTime();				//���������� ����� ����� ������

	Manufacture(string fileName, int ruleNumber){//������������� ������� � ������� (������ ����������� �� ����� fileName)
												//�������� ��������� �������� ��������� �������
		//� ����� � ������ filename ������ �������� ������ � ��������� �������:
		//���������� �������, ���������� �������, ������� ��������������� ���������, ������� ���� ������� ���������
		this->ruleNumber = ruleNumber;
		ifstream fileW(fileName);
		int cur_val;

		//���������� ������ ���������������� �������� � ��������� ������ �� ��������� ������� �� �����
		if (fileW.is_open())
		{
			fileW >> amount_details;
			fileW >> amount_tools;
			details = new vector<Detail>(amount_details);
			tools = new vector<Tool>(amount_tools);
			char s;
			for (int i = 0; i < amount_details; i++)
				for (int j = 0; j < amount_tools; j++)
				{
					fileW >> s;
					details->at(i).setWay(return_number(s));
				}
			for (int i = 0; i < amount_details; i++)
				for (int j = 0; j < amount_tools; j++)
				{
					fileW >> cur_val;
					details->at(i).setTime(cur_val);
				}
		}
		else
		{
			amount_details = amount_tools = 0;
			details = new vector<Detail>();
			tools = new vector<Tool>();
		}

		//������������ ��������� �������� �� ������
		for (int i = 0; i < amount_details; i++){
			tools->at(details->at(i).ways.at(0) - 1).queue_details.push_back(i);
		}

		//�������� ������� ��� �������� ������� ������������ � ������� ���������������� �����:
		rest_time = new int*[amount_details];
		done_time = new vector<int>(amount_details);
		for (int i = 0; i < amount_details; i++)
			rest_time[i] = new int[amount_tools];
		for (int i = 0; i < amount_details; i++)
		{

			for (int j = 0; j < amount_tools; j++)
				rest_time[i][j] = 0;
		}

		//�������������� ���������� ������ �� ������ � ������������ �������� ���������
		for (int i = 0; i < amount_tools; i++)
		{
			if (tools->at(i).currDetail == -1)
			 {
				int curr_det = nextDetail(&tools->at(i).queue_details);
				tools->at(i).currDetail = curr_det;
				tools->at(i).queue_details.erase(remove(tools->at(i).queue_details.begin(), tools->at(i).queue_details.end(), curr_det), tools->at(i).queue_details.end());
			 }
		}
	}
	~ Manufacture(){
		delete details;
		delete tools;
		delete done_time;
		for (int i = 0; i < amount_details; i++)
			delete rest_time[i];
		delete[]rest_time;
	}
};
void Manufacture::showInfo(){ //���������� ������� ����� ��� ������ ������
	cout << endl;
	cout << "������� �����:" << endl;
	for (int i = 0; i < amount_tools; i++)
		cout << "������ " << return_letter(i) << ": " << tools->at(i).GantGraphic << endl;
	cout << endl;
	cout << "������� �������� ��� �������: " << endl;
	for (int i = 0; i < amount_tools; i++)
	{
		int size = tools->at(i).queue_graphics->size();
		cout << "������ " << return_letter(i);
		for (int num_queue = 0; num_queue < size; num_queue++)
			if (num_queue > 0)
				cout << "\t: " << tools->at(i).queue_graphics->at(num_queue) << endl;
			else
				cout << ": " << tools->at(i).queue_graphics->at(num_queue) << endl;
	}
	cout << endl << endl;
	cout << "����� ������������:" << endl;
	cout << "\t";
	for (int i = 0; i < amount_tools; i++)
	{
		cout << return_letter(i) << "\t";
	}
	cout << endl << endl;
	for (int i = 0; i < amount_details; i++)
	{
		cout << i + 1 << "|\t";
		for (int j = 0; j < amount_tools; j++)
			cout << rest_time[i][j] << "\t";
		cout << endl;
	}
}
void Manufacture::showInfo(string filename){
	ofstream out(filename);
	out << "������� �����:" << endl;
	for (int i = 0; i < amount_tools; i++)
		out << "������ " << return_letter(i) << ": " << tools->at(i).GantGraphic << endl;
	out << endl << endl;
	out << "������� �������� ��� �������: " << endl;
	for (int i = 0; i < amount_tools; i++)
	{
		int size = tools->at(i).queue_graphics->size();
		out << "������ " << return_letter(i);
		for (int num_queue = 0; num_queue < size; num_queue++)
			if (num_queue > 0)
				out << "\t: " << tools->at(i).queue_graphics->at(num_queue) << endl;
			else
				out << ": " << tools->at(i).queue_graphics->at(num_queue) << endl;
	}
	out << endl << endl;
	out << "����� ����� ������: " << totalTime << endl;
	out << "����� ������� �������:" << endl;
	vector<int> downTime = getDownTime();
	for (int i = 0; i < downTime.size(); i++)
		out << "�� ������ " << return_letter(i) << " = " << downTime[i] << endl;
	out << endl << endl;
	out << "����� ������������:" << endl;
	out << "\t";
	for (int i = 0; i < amount_tools; i++)
	{
		out << return_letter(i) << "\t";
	}
	out << endl << endl;
	for (int i = 0; i < amount_details; i++)
	{
		out << i + 1 << "|\t";
		for (int j = 0; j < amount_tools; j++)
			out << rest_time[i][j] << "\t";
		out << endl;
	}
}
int Manufacture::getTotalTime(){ /*���������� ����� ����� ������*/
	return totalTime;
}
vector<int> Manufacture::getDownTime(){ /*���������� ����� ������� ���� �������*/
	vector<int> downtime;
	for (int i = 0; i < amount_tools; i++)
		downtime.push_back(tools->at(i).downTime);
	return downtime;
}
int Manufacture::nextDetail(vector<int> *queue_to_machine){/*��������� ��������� ������ � ������� � ������������ � ������� �������*/
	int decision;
	int max_num, max_time;
	if (queue_to_machine->size() > 0)
	{
		switch (ruleNumber)
		{
		case 1: 
			//������� FIFO
			decision = queue_to_machine->front();
			break;
		case 2:
			//������� �� ��������� ����������� ����� ����������������� �����
			max_num = queue_to_machine->front();
			max_time = done_time->at(queue_to_machine->front());
			for (auto it = queue_to_machine->begin(); it != queue_to_machine->end(); it++)
			{
				int sum = done_time->at(*it);
				for (int i = 0; i < amount_tools; i++)
					sum += rest_time[*it][i];
				if (sum > max_time)
				{
					max_time = sum;
					max_num = *it;
				}
			}
			decision = max_num;
			break;
		default:
			decision = queue_to_machine->back();
			break;
		}
	}
	else
		decision = -1;
	return decision;
}
void Manufacture::doWork(){
	totalTime = 0;
	int st = 1;														//����� ���������
	while (true){

		//���������� ������������ �������, ����������� �� ������� ���������
		int minTime = INF;
		for (auto it = tools->begin(); it != tools->end(); it++)
		{
			if (it->currDetail != -1)
			{
				if (details->at(it->currDetail).times[0] < minTime)
				{
					minTime = details->at(it->currDetail).times[0];
				}
			}
		}
		//���� ����������� ����� �����������, �� ��� ������ ��� ��������� ����������
		if (minTime == INF)
			break;

		totalTime += minTime;										 //��������� � ������ ������� ������ ����� ���������

		//���������� ������� ������������ ������� � ����� ���������������� �����, ������� ��� ��������� �� ��������� ���������
		for (int i = 0; i < amount_tools; i++)
			for (auto it = tools->at(i).queue_details.begin(); it != tools->at(i).queue_details.end(); it++)
			{
				if (details->at(*it).ways.size() != amount_tools)
					rest_time[*it][i] += minTime;
			}
		
		
		//���������� ���������� � ��������� ��� ������� �� �������� ����� � �������� ��������
		for (int i = 0; i < amount_tools; i++)
		{
			int number_detail = tools->at(i).currDetail;			 //����� ������, �������������� �� ��������� ������

			int sizeOfQueue = tools->at(i).queue_details.size();			 //���������� ������� � ������� �� ���������
			int amountQueueGraphics = tools->at(i).queue_graphics->size();   //������� �������� ��� ��������
			//�������� �������� ��� �������(�������� � ��) ��-��
			//� �������, ���� ������� ��� ������� �� �������
			for (int k = 0; k < sizeOfQueue - amountQueueGraphics; k++)
			{
				tools->at(i).queue_graphics->push_back(string_cr(totalTime - minTime, "* "));
			}
			amountQueueGraphics = tools->at(i).queue_graphics->size();
			for (int symb = 0; symb < minTime; symb++)
				if (number_detail != -1)
				{
					tools->at(i).GantGraphic += to_string(number_detail + 1) + " ";
					//���� ���� ������ � ������� �� ���� ������
					for (int count_queue = 0; count_queue < amountQueueGraphics; count_queue++)
					{
						if (count_queue >= sizeOfQueue)
							tools->at(i).queue_graphics->at(count_queue) += "* ";
						else
							tools->at(i).queue_graphics->at(count_queue) += to_string(tools->at(i).queue_details.at(count_queue) + 1) + " ";
					}
				}
				else
					//���� ������� ������ �� �������������� �� ������ i,
					//�� ������� ��� ���������� � ������ �����
					//� ������������� ����� ������� ��� i-�� ������
				{
					tools->at(i).downTime += 1;
					tools->at(i).GantGraphic += "* ";
					//���� ���� ������ � ������� �� ���� ������
					int size = tools->at(i).queue_graphics->size();
					for (int count_queue = 0; count_queue < size; count_queue++)
					{
						tools->at(i).queue_graphics->at(count_queue) += ("* ");
					}
				}
		}
			

		for (int i = 0; i < amount_tools; i++)							 //��� ������� ������, �� ������� �������������� ������
		{
			if (tools->at(i).currDetail != -1)
			{
				Detail *curr_det = &details->at(tools->at(i).currDetail);
				curr_det->times[0] -= minTime;							 //�������� �� ������� ��������� ������ �� ���� ������
																		 //����� �������� ���������
				done_time->at(tools->at(i).currDetail) += minTime;
				if (curr_det->times[0] == 0)							 //���� ������ ��������� ���������� �� ���� ������ �� ���� ���������
				{
					curr_det->delWayTime();								 //�� ������� �� � ����������� �������� ���� ������
					if (curr_det->ways.size() > 0)						 //���� ��������������� ������� ������ ��� �� ��� ��������
																		 //�� ������� ��� ������ � ������� ���������
																		 //������, ���������� � � ��������
					tools->at(curr_det->ways[0] - 1).queue_details.push_back(tools->at(i).currDetail);
					tools->at(i).currDetail = -1;
				}
			}
		}


		for (int i = 0; i < amount_tools; i++)							 //��� ������� ������, �� ������� ����������� ��������� ������
			if (tools->at(i).currDetail == -1)							 
			{
				int curr_det = nextDetail(&tools->at(i).queue_details);		 //��������� ������, ������� �� �� ��������������
				tools->at(i).currDetail = curr_det;
				tools->at(i).queue_details.erase(remove(tools->at(i).queue_details.begin(), tools->at(i).queue_details.end(), curr_det),tools->at(i).queue_details.end());

			}
		st += 1;														 //� ��������� � ���������� ���������
	}
	showInfo();														 //������� ������� ����� ��� ����� ������� ���������
	showInfo("outputRule" + to_string(ruleNumber) + ".txt");
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Rus");
	//��������� ������� ������ �� ������� ������������ 1
	cout << "����� � �������� ������������ 1: " << endl;
	Manufacture fact1("input.txt", 1);
	fact1.doWork();															//��������� ��� ������ �� ������� � ������� ������� ����� � ��������
	cout << "����� ����� �����: " << fact1.getTotalTime() << endl;				//����� ����� ���������
	cout << "����� ������� �������:" << endl;
	vector<int> downTime = fact1.getDownTime();
	for (int i = 0; i < downTime.size(); i++)
		cout << "�� ������ " << return_letter(i) << " = " << downTime[i] << endl;		//����� ������� ������� �� �������

	cout << endl <<  "##############################################################################################" << endl << endl;

	//����� ��������� ������ �� ������� ������������ 2
	cout << "����� � �������� ������������ 2: " << endl;
	Manufacture fact2("input.txt", 2);
	fact2.doWork();															//��������� ��� ������ �� ������� � ������� ������� ����� � ��������
	cout << "����� ����� �����: " << fact2.getTotalTime() << endl;				//����� ����� ���������
	cout << "����� ������� �������:" << endl;
	downTime = fact2.getDownTime();
	for (int i = 0; i < downTime.size(); i++)
		cout << "�� ������ " << return_letter(i) << " = " << downTime[i] << endl;		//����� ������� ������� �� �������
	return 0;
}
