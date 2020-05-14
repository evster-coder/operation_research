#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#define epsilon 0.0000000001

using namespace std;
const int NotUsed = system("color F0");

void convertStr(string st, vector<double> *targetFunction){ 
	//��������������� �������, ������� �� ������ st �������� ������ targetFunction �������������
	//targetFunction[0] = ���������� �����
	//targetFunction[1, ..., n] = ������������ ��� ��������������� x
	int prev = 0;
	int next;
	vector<string> substrings;
	while ((next = st.find(" ", prev)) != string::npos){ //���� � ������ ����� ����� ������
		string tmp = st.substr(prev, next - prev); //�������� ��������� ���������
		substrings.push_back(st.substr(prev, next - prev)); //��������� � � ������
		prev = next + 1; //��������� � ��������� ���������
	}
	substrings.push_back(st.substr(prev, st.length()));

	//����������� ������ ��������� � ������� ������� targetFunction
	char sign = '+'; //���� �����
	double number = 0;//���� �����
	for (int i = 0; i < substrings.size(); i++)
	{
		string curStr = substrings.at(i);
		//���� ��������� - ���� + ��� -, �� ��� �� �� ��� ��������� ����
		//���� ������� ���� ���� � sign
		if (curStr == "-" || curStr == "+")
		{
			if (number != 0)
			{
				targetFunction->at(0) = number;
				number = 0;
			}
			sign = curStr[0];
		}
		//���� ��������� - �����, �� ���������� ���
		if (curStr[0] >= '0' && curStr[0] <= '9' || (curStr[0] == '+' && curStr.length() > 1) || (curStr[0] == '-' && curStr.length() > 1))
		{
			number = stod(curStr);
			if (sign == '-')
				number = -number;
		}
		//���� ��������� ���������� � 'x', �� ������ ������� ������ ����������
		if (curStr[0] == 'x'){
			int indexX = stoi(curStr.substr(1, curStr.length()));
			targetFunction->at(indexX) = number;
			number = 0;
		}
		//����� ����� <= ������������ ��������� ����
		if (curStr == "<="){
			number = stod(substrings.at(i + 1));
			targetFunction->at(0) += number;
		}
	}
}

class system_zlp{
private: int amountX;			//���������� ����������
		 int amountInequality;	//���������� ����������
		 bool target;			//0 - �������, 1 - ��������
		 vector<double> *functionTarget; //������� ���� (�������� ������� ��� ����������)
		 vector<vector<double>> *systemValue;//�������� ������������� � �������
		 vector<vector<double>> *simplexTable;//��������-�������
		 vector<int> *basisIndices;//�������� ����������
		 vector<int> *freeIndices; //��������� ����������
		 vector<double> resultValues; //��������  ���� ����������
		 int resolvingColumn;		//����������� ������� �� ������� ����
		 int resolvingRow;			//����������� ������ �� ������� ����

public:  void readFromFile(string filename);	//��������� ������� ��� �� �����
		 void showTaskConditions();				//������ ������� ������
		 bool ableToEnhance();					//����������� ��������� ������� ����
		 void findDecision();					//����� ������� ���
		 void showResult();						//������ ����������
		 void writeResult(string filename);		//������ ���������� � �����
		 ~system_zlp(){
			 delete functionTarget;
			 delete systemValue;
			 delete simplexTable;
			 delete basisIndices;
			 delete freeIndices;
		 }
};

void system_zlp::readFromFile(string filename){
	string st;
	//��������� ���� ��� ���������� �������� ������
	ifstream inp(filename);
	inp >> amountX;							//��������� ���������� ����������
	inp >> amountInequality;				//��������� ���������� ����������
	inp >> st;								//��������� �������� ���� ������� ������� ����	
	if (st == "max")
		target = true;
	else
		target = false;

	functionTarget = new vector<double>(amountX + 1, 0);
	systemValue = new vector<vector<double>>(amountInequality, vector<double>(amountX + 1, 0));

	getline(inp, st);						//��������� ������� ����
	convertStr(st, functionTarget);			//��������� � ����-�� � ������
	//��������� ������� ����������� � ��������� � � ������
	for (int i = 0; i < amountInequality; i++)
	{
		getline(inp, st);
		convertStr(st, &(systemValue->at(i)));
	}
}

void system_zlp::showTaskConditions(){
	//������� ������� ���������� � �������
	if (amountX > 0 && amountInequality > 0) //���� �������� ������ ��� �������
	{
		cout << "���������� ���������� = " << amountX << " ���������� = " << amountInequality;
		cout << "\n________________________________________________________________________\n";
		cout << "�����  ";
		//���� ����� �������� ������������, �� target = true, ���� �����������, �� false
		if (target == true)
			cout << "�������� ";
		else
			cout << "������� ";

		//������ ������� ����
		cout << "F = ";
		if (functionTarget->at(0) != 0)
			cout << functionTarget->at(0) << "    +    ";
		for (int i = 1; i < amountX + 1; i++)
		{
			if (functionTarget->at(i) != 0)
			{
				if (i != amountX)
					cout << functionTarget->at(i) << " * x" << i << "    +    ";
				else
					cout << functionTarget->at(i) << " * x" << i << " ,";
			}
		}

		//������ ������� �����������
		cout << endl << "��� ������������:" << endl;
		for (int i = 0; i < amountInequality; i++)
		{
			for (int j = 1; j < amountX; j++)
			{
					cout << systemValue->at(i).at(j) << " * x" << j << "   +\t";
			}
			cout << systemValue->at(i).at(amountX) << " * x" << amountX;
			cout << " <= " << systemValue->at(i).at(0) << ";" << endl;
		}
		cout << endl;
		//����� �������������� ����������� �� ����������������� xi
		for (int i = 0; i < amountX; i++)
			cout << "x" << i + 1 << " >= 0;\t";
		cout << endl << "________________________________________________________________________" << endl;
	}
}

bool system_zlp::ableToEnhance(){
	//������� ���������, ���� �� ����������� ��� ��������(���������������, ��������������) ������� ����
	int size = amountX + 1;
	//���� ����� �������� ������������
	//�� ���� ������������� ����-�� � ������� ���� � ��������-�������
	//�.�. �� ������� '-' �� ������
	if (target == 1)
	{
		for (int i = 1; i < size; i++)
			if ((*simplexTable)[0][i] < - epsilon)
				return true;
	}
	//����� ���� ������������� ����-�� � ������� ���� � ��������-�������
	else
	{
		for (int i = 1; i < size; i++)
			if ((*simplexTable)[0][i] > epsilon)
				return true;
	}
	return false;
}

void system_zlp::findDecision(){
	//������� ���� ����������� ������� ���

	//����������� ������������� ������ ��������� ����������
	freeIndices = new vector<int>(amountX, 0);
	for (int i = 0; i < amountX; i++)
		freeIndices->at(i) = i + 1;

	//����������� ������������� ������ �������� ����������
	basisIndices = new vector<int>(amountInequality, 0);
	for (int i = 0; i < amountInequality; i++)
		basisIndices->at(i) = i + 1 + amountX;

	//������ � �������������� ���������  ��������-�������
	simplexTable = new vector<vector<double>>(amountInequality + 1, vector<double>(amountX + 1, 0));

	//������ ������ �������� ������� -- ������� ����
	//� ����������� � ������ ����-�� ��� xi �� ������ '-'
	simplexTable->at(0) = *functionTarget;
	for (int i = 0; i < amountX; i++)
	{
		if (simplexTable->at(0).at(i + 1) != 0)
			simplexTable->at(0).at(i + 1) *= -1;
	}
	//��� ��������� ������ �������� ������� ���������
	//����������� ������� �� ��������� ������� (��� �������� bi �����)
	for (int i = 0; i < amountInequality; i++)
	{
		simplexTable->at(i + 1) = systemValue->at(i);
	}

	bool imposible = false;			//������ ��������� ������
	//���������, ���� �������� �������� ������� ����
	while (ableToEnhance() == true)
	{
		resolvingColumn = 0;	//����������� �������
		resolvingRow = 0;		//����������� ������

		//�������� ������� ����������������� xi (��� �������� ����������)
		//� �� ���������������
		for (int i = 0; i < amountInequality; i++)
		{
			if (simplexTable->at(i + 1).at(0) < 0)
			{
				imposible = true;
				break;
			}
			if (fabs(simplexTable->at(i + 1).at(0)) < epsilon) 
			{
				simplexTable->at(i + 1).at(0) = 0.00001;
			}
		}
		//���� �������� ���������� ������������
		if (imposible)
			break;

		//����� ������������ �������
		double valueTmp = 0;
		if (target == true) //���� ���� ��������, �� �������� ������������ ������������� �������
		{
			for (int i = 0; i < amountX; i++)
				if (valueTmp > (*simplexTable)[0][i + 1])
				{
					resolvingColumn = i + 1;
					valueTmp = (*simplexTable)[0][i + 1];
				}
		}
		else //����� ������������ �������������
		{
			for (int i = 0; i < amountX; i++)
				if (valueTmp < (*simplexTable)[0][i + 1])
				{
					resolvingColumn = i + 1;
					valueTmp = (*simplexTable)[0][i + 1];
				}
		}

		//����� ����������� ������
		valueTmp = INFINITY;
		for (int i = 0; i < amountInequality; i++)
		{
			//������� ����������� ������������� ��������� b[i] / c[i][resolvingColumn]
			if ((*simplexTable)[i + 1][resolvingColumn] > 0)
			{
				double curVal = (*simplexTable)[i + 1][0] / (*simplexTable)[i + 1][resolvingColumn];
				if (curVal < valueTmp){
					valueTmp = curVal;
					resolvingRow = i + 1;
				}
			}
		}
		//���� ��� ��������� �� ������������� ����������� ��������(���� �� ���� ������������ � �������), �� ������� ���
		if (valueTmp == INFINITY)
		{
			imposible = true;
			break;
		}

		//������� �������� ����������(� ������� � ������ resolvingRow) �� ���������(� ������� resolvingColumn)
		int tmpIndex = freeIndices->at(resolvingColumn - 1);
		freeIndices->at(resolvingColumn - 1) = basisIndices->at(resolvingRow - 1);
		basisIndices->at(resolvingRow - 1) = tmpIndex;

		//������������ ����� ��������-�������
		vector<vector<double>> *newSimplexTable = new vector<vector<double>>(amountInequality + 1, vector<double>(amountX + 1, 0));

		//������������ �������� �� ����� ������������ ��������
		//a'[r][c] = 1 / a[r][c]
		newSimplexTable->at(resolvingRow).at(resolvingColumn) = 1 / (*simplexTable)[resolvingRow][resolvingColumn];

		//������������ �������� �� ����� ����������� ������
		//a'[r][j] = a[r][j] / a[r][k] (j = 0,1,...,amountX + 1; j != c)
		for (int j = 0; j < amountX + 1; j++)
			if (j != resolvingColumn)
				newSimplexTable->at(resolvingRow).at(j) = 
				(*simplexTable)[resolvingRow][j] / (*simplexTable)[resolvingRow][resolvingColumn];

		//������������ �������� �� ����� ������������ �������
		//a'[i][c] = - a[i][c] / a[r][c] (i = 0,1,...,amountInequality + 1; i != r)
		for (int i = 0; i < amountInequality + 1; i++)
			if (i != resolvingRow)
				newSimplexTable->at(i).at(resolvingColumn) =
				(-(*simplexTable)[i][resolvingColumn]) / (*simplexTable)[resolvingRow][resolvingColumn];

		//������������ �������� ���� ��������� �������
		//a'[i][j] = a[i][j] + a[r][j] * a'[i][c] (i = 0,1,2,..., amountInequality + 1; i != r; j = 0,1,2,..., amountX + 1; j != c)
		for (int i = 0; i < amountInequality + 1; i++)
		{
			for (int j = 0; j < amountX + 1; j++)
			{
				if (i != resolvingRow && j != resolvingColumn)
					newSimplexTable->at(i).at(j) =
					(*simplexTable)[i][j] + (*simplexTable)[resolvingRow][j] * (*newSimplexTable)[i][resolvingColumn];
			}
		}
		//������ ��� ��������-������� �������
		(*simplexTable) = (*newSimplexTable);
		for (int i = 0; i < amountInequality + 1; i++)
		{
			for (int j = 0; j < amountX + 1; j++)
				cout << (*simplexTable)[i][j] << "\t";
			cout << endl;
		}
		cout << endl << endl;
		delete newSimplexTable;
	}
	if (imposible == true) //���� ���� ����� �� ������ � ������
	{
		cout << "��� �������" << endl;
	}
	else
	{
		//� simplexTable[0][0] ����� �������� ����������� �������� ������� ����,
		//��� ��������� ��������� ����������, ������ ����
		//��� ���������� ��������� ������� ��������� ������ ����� �� ��������-�������
		//�� �������, � ������� �������� ������ �������� ����������
		for (int i = 0; i < amountX + amountInequality; i++){
			auto it = find(basisIndices->begin(), basisIndices->end(), i + 1);
			if (it == basisIndices->end())
			{
				resultValues.push_back(0);
			}
			else
			{
				resultValues.push_back((*simplexTable)[it - basisIndices->begin() + 1][0]);
			}
		}
	}
}

void system_zlp::showResult(){
	//����� ���������� �� �����
	cout << "���������:" << endl;
	if (resultValues.size() > 0)
	{
		if (target == true)
			cout << "Max  ";
		else
			cout << "Min  ";
		//����������� �������� ������� ���� �������� � simplexTable[0][0]
		cout << "F = " << (*simplexTable)[0][0] << endl;
		cout << endl;
		//����� �������� ����������� ���������� � ���������� ����������
		cout << "X = ( ";
		for (int i = 0; i < amountX; i++)
			cout << "x" << i + 1 << "  ";
		cout << ") = (  ";
		for (int i = 0; i < amountX; i++)
			cout << resultValues[i] << "  ";
		cout << ")" << endl;

		cout << "Z = ( ";
		for (int j = 0; j < amountInequality; j++)
			cout << "z" << j + 1 << " ";
		cout << ") = (  ";
		for (int j = 0; j < amountInequality; j++)
			cout << resultValues[j + amountX] << "  ";
		cout << ")" << endl;
		
		//�������� �� �������������� �������.
		bool singleDecision = true;
		for (int j = 0; j < amountX; j++)
			if ((*simplexTable)[0][j + 1] == 0)
			{
				singleDecision = false;
				break;
			}
		if (!singleDecision)
			cout << "������� �� �����������, �.�. � �������� ����� ������ �� ��� ����������" << endl;
	}
	else
	{
		cout << "������� ���" << endl;
		
		//����������� �������������� ������� (��� ��������� ����� �������������)
		if (resolvingRow == 0){
			if (target == true)
				cout << "�.�. ������� ������������ ������" << endl;
			else
				cout << "�.�. ������� ������������  �����" << endl;
		}
	}
}
void system_zlp::writeResult(string filename){
	//������ ���������� � ���� filename
	ofstream out(filename);
	out << "���������:" << endl;
	//���� ��� ������ ��� �������:
	if (resultValues.size() > 0)
	{
		if (target == true)
			out << "Max  ";
		else
			out << "Min  ";
		//����������� �������� ������� ���� �������� � simplexTable[0][0]
		out << "F = " << (*simplexTable)[0][0] << endl;
		out << endl;
		//����� �������� ����������� ���������� � ���������� ����������
		out << "X = ( ";
		for (int i = 0; i < amountX; i++)
			out << "x" << i + 1 << "  ";
		out << ") = (  ";
		for (int i = 0; i < amountX; i++)
			out << resultValues[i] << "  ";
		out << ")" << endl;

		out << "Z = ( ";
		for (int j = 0; j < amountInequality; j++)
			out << "z" << j + 1 << " ";
		out << ") = (  ";
		for (int j = 0; j < amountInequality; j++)
			out << resultValues[j + amountX] << "  ";
		out << ")" << endl;

		//�������� �� �������������� �������.
		bool singleDecision = true;
		for (int j = 0; j < amountX; j++)
			if ((*simplexTable)[0][j + 1] == 0)
			{
				singleDecision = false;
				break;
			}
		if (!singleDecision)
			out << "������� �� �����������, �.�. � �������� ����� ������ �� ��� ����������" << endl;
	}
	else
		out << "������� ���" << endl;
	out.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Rus");
	//������� � �������������� ����������� �������
	system_zlp mySystem;
	mySystem.readFromFile("input.txt");
	//��������� ����������� ������� �� �����
	mySystem.showTaskConditions();
	//������ ������
	mySystem.findDecision();
	//�������� ��������� �� ����� � � ����
	mySystem.showResult();
	mySystem.writeResult("output.txt");
	return 0;
}
