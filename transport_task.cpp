#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <conio.h>
#include <iomanip>

using namespace std;
const int NotUsed = system("color F0");

// �-������
class t_task{
private:
	int amount_provider;				//���������� �����������
	int amount_client;					//���������� ������������
	vector<double> *supplies;			//������ � ������� ����������
	vector<double> *needs;				//����� � ������� �����������
	vector<vector<double>> *cost;		//cost[i][j] - ������� �� �������� ������� ��������� �� ���������� i � ����������� j
	vector<vector<double>> *base_plan;	//������� ����
	int extra_resource;					//-1 - ���� ��� ���. �����������, 1 - ���. ���������, 0 - ������� ����� ������
	double epsilon = 0.01;				//�������� epsilon �� ������ �������������
public:
	t_task(string filename);		//�����������
	bool print_conditions();		//������ ������� �-������
	void print_base_plan();			//������ �������� �������� �����
	void find_first_plan();			//������������ ���������� �������� �����
	void find_optimal_decision();	//���������� ������������ ������� ������������ ������
};

t_task::t_task(string filename){ //���������� ������� ���������� �� ����� filename
	ifstream inp(filename);	//�������� ����� � ��������� �������
	if (inp.is_open())
	{
		//���������� ���������� ����������� � ������������
		inp >> amount_provider;
		inp >> amount_client;
		//�������� ����������� �������� ������
		supplies = new vector<double>(amount_provider, 0);
		needs = new vector<double>(amount_client, 0);
		cost = new vector<vector<double>>(amount_provider, vector<double>(amount_client, 0));
		//���������� ������� �������
		for (int i = 0; i < amount_provider; i++)
			inp >> (*supplies)[i];
		//���������� ������� ������������
		for (int j = 0; j < amount_client; j++)
			inp >> (*needs)[j];
		//���������� ������� ����������
		for (int i = 0; i < amount_provider; i++)
			for (int j = 0; j < amount_client; j++)
				inp >> (*cost)[i][j];
	}
	else
	{
		cout << "������ �������� �����";
		amount_client = amount_provider = 0;
	}
}

bool t_task::print_conditions(){ //������ ������������ ������� ������
	if (amount_provider == 0 || amount_client == 0)
	{
		cout << "������� ������ �����������" << endl;
		return false;
	}
	//������ ������ ������ �������
	cout << "������� ������������ ������:" << endl;
	cout << "������\t\t";
	for (int j = 0; j < amount_client; j++)
		cout << j + 1 << "\t";
	cout << "\t������" << endl << endl;

	//������ ������� ���������� � ������� �������
	for (int i = 0; i < amount_provider; i++)
	{
		cout << i + 1 << "\t\t";
		for (int j = 0; j < amount_client; j++)
			cout << (*cost)[i][j] << "\t";
		cout << "|\t";
		cout << (*supplies)[i] << endl;
	}
	cout << "\t\t";
	for (int i = 0; i < amount_client; i++)
		cout << "-----\t";
	cout << endl;

	//������ ������� ������������
	cout << "�����������\t";
	for (int j = 0; j < amount_client; j++)
		cout << (*needs)[j] << "\t";
	cout << endl;
	cout << "________________________________________________________________________________________________" << endl;
	return true;
}

void t_task::find_first_plan(){ //������������ ���������� �������� �����
	if (base_plan != nullptr)
		delete base_plan; 

	//�������� ������ ��������� �����
	base_plan = new vector<vector<double>>(amount_provider, vector<double>(amount_client, 0));

	//������� �������, �������� ���������������� ���������� ������
	vector<double> aa = *supplies;
	vector<double> bb = *needs;

	//������ ����� ������� ���������� �������� �������� aa � bb;
	int not_null_aa = -1;
	for (int i = 0; i < amount_provider; i++)
		if ((*supplies)[i] > 0)
		{
			not_null_aa = i; break;
		}
	int not_null_bb = -1;
	for (int j = 0; j < amount_client; j++)
		if ((*needs)[j] > 0)
		{
			not_null_bb = j; break;
		}

	//���������� ������� �������� �����, ���� ��� aa � bb �� ����� 0
	bool is_degenerated = false;	// ���� ������������� �����
	while (not_null_aa < amount_provider && not_null_bb < amount_client){
		//�������� ������� �� ���������� ������ ������� � ����
		double s = std::fmin(aa[not_null_aa], bb[not_null_bb]);
		//� ���������� ���� ������� � ������� �������� �����
		(*base_plan)[not_null_aa][not_null_bb] = s;
		//�������� �� ������� � ������������ �������������� ����� ��������
		aa[not_null_aa] -= s;
		bb[not_null_bb] -= s;
		//�������� ��������� ������� � ���������
		if (aa[not_null_aa] == 0 && bb[not_null_bb] == 0)		//���� ���������� � ������ � �����������, �� ���� �����������, ��������� epsilon
		{
			if (not_null_aa < amount_provider - 1 || not_null_bb < amount_client - 1)
			{
				is_degenerated = true;
				if (not_null_aa < amount_provider - 1 && (*base_plan)[not_null_aa + 1][not_null_bb] == 0)	//���� ����� �������� epsilon ���� �� ������������ ������
					(*base_plan)[not_null_aa + 1][not_null_bb] = epsilon;
				else
					if (not_null_bb < amount_client - 1 && (*base_plan)[not_null_aa][not_null_bb + 1] == 0)	//���� ����� �������� epsilon ������ �� ������������ ������
						(*base_plan)[not_null_aa][not_null_bb + 1] = epsilon;
					else //����� ��������� epsilon ����� �� ������������ ������
						(*base_plan)[not_null_aa][not_null_bb - 1] = epsilon;
				epsilon /= 10;
			}
			not_null_aa++;
			not_null_bb++;
		}
		else
		{	//����� ���������� ������� ���������� �����
			if (aa[not_null_aa] == 0)
				not_null_aa++;
			if (bb[not_null_bb] == 0)
				not_null_bb++;
		}
	}
	if (is_degenerated == true)
		cout << "������� ���� �����������" << endl;
}

void t_task::print_base_plan(){
	//����� �������� �����
	cout << "________________________________________________________________________________________________" << endl;
	cout << "������� ���� ����� ���:" << endl;
	for (int j = 0; j < amount_client; j++)
		cout << "\tB" << j + 1;
	cout << endl;
	for (int i = 0; i < amount_provider; i++)
	{
		cout << "A" << i + 1 << "\t";
		for (int j = 0; j < amount_client; j++)
			cout << (*base_plan)[i][j] << "\t";
		cout << endl;
	}
	//���������� �������� ������� ���� � ��������
	vector<vector<double>> x_copy = (*base_plan);
	double func_val = 0;
	double extra_val = 0;
	for (int i = 0; i < amount_provider; i++)
		for (int j = 0; j < amount_client; j++)
			if (x_copy[i][j] > 0)
				func_val += (double)round(x_copy[i][j]) * (*cost)[i][j];
	//�������� �� �������������� ��������� ��� �����������
	if (extra_resource == 1){//���. ���������
		for (int j = 0; j < amount_client; j++)
			extra_val += x_copy[amount_provider - 1][j];
		cout << "(��������� A" << amount_provider << " �������� ���������)" << endl;
	}
	else if (extra_resource == -1){//���. �����������
		for (int i = 0; i < amount_provider; i++)
			extra_val += x_copy[i][amount_client - 1];
		cout << "(����������� B" << amount_client << " �������� ���������)" << endl;
	}

	//����� �������� ������� ����
	cout << "������� ���� F = " << func_val << " ���. ��." << endl;
	if (extra_resource == 1)
		cout << "���������� = " << extra_val  << " ��. ���������" << endl;
	if (extra_resource == -1)
		cout << "������� = " << extra_val << " ��. ���������" << endl;
}

void t_task::find_optimal_decision(){ /*���������� ������������ �������*/
	print_conditions();
	//�������� ������ �� ������������������
	double sum1 = 0, sum2 = 0;
	for (int i = 0; i < amount_provider; i++)
		sum1 += supplies->at(i);
	for (int j = 0; j < amount_client; j++)
		sum2 += needs->at(j);
	//���� ������ ��������� �����������
	//�� ��������� ���������� �����������
	if (sum1 > sum2)
	{
		cout << "������ ������������������. ���� ��������������� �����������" << endl;
		amount_client += 1;
		for (int i = 0; i < amount_provider; i++)
			cost->at(i).push_back(0);
		needs->push_back(sum1 - sum2);
		extra_resource = -1;
	}
	//���� ����������� ��������� ������
	//�� ��������� ���������� ����������
	if (sum1 < sum2)
	{
		cout << "������ ������������������. ���� ��������������� ����������" << endl;
		amount_provider += 1;
		cost->push_back(vector<double>(amount_client, 0));
		supplies->push_back(sum2 - sum1);
		extra_resource = 1;
	}
	//����� ������ ����������������
	if (sum1 == sum2)
		extra_resource = 0;

	// ���������� ���������� ����������� �������� �����
	find_first_plan();
	//������ ���������� �������� �����
	print_base_plan();

	//����� �����������
	int iter_count = 0;
	while (true)
	{
		iter_count++;
		cout << endl << "#######################################################################################" << endl;
		cout << "�������� � " << iter_count << endl;
		cout << "#######################################################################################" << endl;
		bool degenerate = false;	//�������� �� ���� ����������� �� ������ ���������

		vector<double> *pb = new vector<double>(amount_client, DBL_MAX);		//���������� ��� ����������� Ai
		vector<double> *pa = new vector<double>(amount_provider, DBL_MAX);		//���������� ��� ������������ Bj

		//��������������� ������� �����������
		vector<vector<double>> *h = new vector<vector<double>>(amount_provider, vector<double>(amount_client, 0));

		//������������ ��������� �����������
		(*pa)[0] = 0;
		bool all_found = true;
		do{
			all_found = true;
			for (int i = 0; i < amount_provider; i++)
				for (int j = 0; j < amount_client; j++)
					if ((*base_plan)[i][j] != 0)
					{
						if ((*pa)[i] != DBL_MAX && (*pb)[j] == DBL_MAX)
							(*pb)[j] = (*cost)[i][j] + (*pa)[i];
						if ((*pa)[i] == DBL_MAX && (*pb)[j] != DBL_MAX)
							(*pa)[i] = (*pb)[j] - (*cost)[i][j];
					}
			//��������, ��� ��� ���������� ���� �������
			for (int i = 0; i < amount_provider; i++)
				if ((*pa)[i] == DBL_MAX)
				{
					all_found = false; break;
				}
			if (all_found)
			{
				for (int j = 0; j < amount_client; j++)
					if ((*pb)[j] == DBL_MAX)
					{
						all_found = false; break;
					}
			}
		} while (all_found == false);
		//������������ ������� �����������
		for (int i = 0; i < amount_provider; i++)
			for (int j = 0; j < amount_client; j++)
			{
				(*h)[i][j] = (*cost)[i][j] + (*pa)[i] - (*pb)[j];
			}


		//�������� ����� �� ������������� � ���������� ������������ �������������� ��������
		bool able_to_enhance = false;
		double start_el = 0;
		pair<int, int> start_index;
		for (int i = 0; i < amount_provider; i++)
			for (int j = 0; j < amount_client; j++)
				if ((*h)[i][j] < 0)		//���� �� � ������� H ������������� ��������
				{
					able_to_enhance = true;
					if ((*h)[i][j] < start_el)	//����� ������������ �������������� ��������
					{
						start_el = (*h)[i][j];
						start_index = make_pair(i, j);
					}
				}
		//���� ���� ������ ��������, �� �� �����������
		if (able_to_enhance == false)
		{
			cout << "������� ���� �� ���������� �������� ���������� ��������, ������ �� �����������" << endl;
			cout << "________________________________________________________________________________________________" << endl;
			cout << "________________________________________________________________________________________________" << endl;
			break;
		}

		//����� ������ ���� �� �������� �������
		vector<vector<double>> x_copy = *(base_plan);
		x_copy[start_index.first][start_index.second] = DBL_MAX;
		//���� �������� ������� ������������, � ������:
		//����� ���������� ���������� ����� � �������� ������� � ��� ���������� ����
		//� ������ ������������.
		//1. ����������� ������, � ������� ����� 1 �������� ������
		//2. ����������� �������, � ������� ����� 1 �������� ������
		//3. ��������� �.1, ���� ����� ���-�� ����������
		//4. ���������� ������ �������� ����. ��� ��� ������ ������ �� ��������� ������
		//5. ����� ����� �������� ������ � ��� �� ������, ����� � ������� ��������� - ��������� �������� � ��
		//   ���� �� �������� � �������� 
		//(�.�. ��������, �.�. ��������� "����������� � �������������� ������ �������� �������" ������ - 2011, ���. 114)
		int count = 0;
		bool cutMade = false;
		while (true)
		{
			//���� ������, ��� ����� 1 ��������� �������  � �������� ��
			cutMade = false;
			for (int i = 0; i < amount_provider; i++)
			{
				count = 0;
				for (int j = 0; j < amount_client; j++)
					if (x_copy[i][j] != 0)
						count++;
				if (count == 1)
				{
					for (int j = 0; j < amount_client; j++)
						x_copy[i][j] = 0;
					cutMade = true;
				}
			}
			//���� �������, ��� ����� 1 ��������� ������� � �������� ��
			for (int j = 0; j < amount_client; j++)
			{
				count = 0;
				for (int i = 0; i < amount_provider; i++)
					if (x_copy[i][j] != 0)
						count++;
				if (count == 1)
				{
					for (int i = 0; i < amount_provider; i++)
						x_copy[i][j] = 0;
					cutMade = true;
				}
			}

			//���� �� ����� ������ ��� ������� �� ���� ��������, �� ��������� �����
			if (cutMade == false)
				break;
		}

		pair<int, int> curr_index = start_index;
		vector< pair<int, int>> cycle;			//�������� ������� �����
		cycle.push_back(start_index);

		//����� �����: ������� ������ ��������� ������� �� ������, ����� ��������� �� ������� � ��� ���� �� �������� � ��������
		while (true)
		{
			for (int j = 0; j < amount_client; j++)
				if (x_copy[curr_index.first][j] != 0 && j != curr_index.second)
				{
					curr_index = make_pair(curr_index.first, j);
					break;
				}
			if (curr_index == start_index)
				break;
			cycle.push_back(curr_index);

			for (int i = 0; i < amount_provider; i++)
				if (x_copy[i][curr_index.second] != 0 && i != curr_index.first)
				{
					curr_index = make_pair(i, curr_index.second);
					break;
				}
			if (curr_index == start_index)
				break;
			cycle.push_back(curr_index);
		}

		//���� ����������� ������ ������� (�� �������)
		pair<int, int> min_index = cycle[1];
		double min_el = (*base_plan)[cycle[1].first][cycle[1].second];
		for (unsigned i = 3; i < cycle.size(); i += 2)
			if ((*base_plan)[cycle[i].first][cycle[i].second] < min_el)
			{
				min_el = (*base_plan)[cycle[i].first][cycle[i].second];
				min_index = cycle[i];
			}

		//����������� �� ���� �������� ��������� ����� � ��������� �� ���� ������ ��������� �����
		for (unsigned i = 0; i < cycle.size(); i += 2)
		{
			(*base_plan)[cycle[i].first][cycle[i].second] += min_el;
			(*base_plan)[cycle[i+1].first][cycle[i+1].second] -= min_el;
		}

		//�������� ����� �� �������������
		int amount_not_null = 0;
		//������� ��������� ��������� � ������� �����
		for (int i = 0; i < amount_provider; i++)
			for (int j = 0; j < amount_client; j++)
				if ((*base_plan)[i][j] > 0)
					amount_not_null++;

		//���� not_null > m + n - 1, �� ���� �����������
		bool already_found = false;
		//��������� epsilon � ������� ����
		if (amount_not_null != amount_client + amount_provider - 1)
		{
			degenerate = true;	//������� � ������������� �����
			for (unsigned int i = 0; i < cycle.size(); i++)
			{
				if ((*base_plan)[cycle[i].first][cycle[i].second] == 0)
					if (already_found == true)
					{
						(*base_plan)[cycle[i].first][cycle[i].second] = epsilon;
						epsilon /= 10;
					}
					else
						already_found = true;
			}
		}
		delete pa, pb, h;
		if (degenerate)
			cout << "������� ���� �� ������� �������� �����������" << endl;
		//������ �������� �����
		print_base_plan();
	}

	//������ ������������ ������� ������������ ������
	cout << endl;
	cout << "����������� �������:" << endl;
	//��������� �� ���������� ������, ����� ���������� �� �������
	for (int i = 0; i < amount_provider; i++)
		for (int j = 0; j < amount_client; j++)
			(*base_plan)[i][j] = (double)(round((*base_plan)[i][j]));
	print_base_plan();
} 

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Rus");
	t_task *new_task = new t_task("input.txt");
	new_task->find_optimal_decision();
	return 0;
}
