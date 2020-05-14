#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#define epsilon 0.0000000001

using namespace std;
const int NotUsed = system("color F0");

//������, ���������� ��������� �� ������
string error_info = "";

void convertStr(string st, vector<double> *targetFunction, int *signIneq){
	//��������������� �������, ������� �� ������ st �������� ������ targetFunction �������������
	//targetFunction[0] = ���������� �����
	//targetFunction[1, ..., n] = ������������ ��� ��������������� x
	//� signIneq �������� ���� ����������� �����������
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
			*signIneq = -1;
		}
		//����� ����� >= ������������ ��������� ����
		if (curStr == ">="){
			number = stod(substrings.at(i + 1));
			targetFunction->at(0) += number;
			*signIneq = 1;
		}
		//����� ����� = ������������ ��������� ����
		if (curStr == "="){
			number = stod(substrings.at(i + 1));
			targetFunction->at(0) += number;
			*signIneq = 0;
		}
	}
}

class system_zlp{
public:	 bool dualTask = false; //������ ������ ��� ������������
		 int amountX;			//���������� ����������
		 int amountInequality;	//���������� ����������
		 bool target;			//0 - �������, 1 - ��������
		 vector<double> *functionTarget; //������� ���� (�������� ������� ��� ����������)
		 vector<vector<double>> *systemValue;//�������� ������������� � �������
		 vector<int> *signsInequality;	//����� �����������-���������� (-1 - ������, 0 - �����, 1 - ������ ���� �����)
		 vector <vector<double>> *simplexTable;//��������-�������
		 vector<int> *basisIndices;//�������� ����������
		 vector<int> *freeIndices; //��������� ����������
		 vector<double> resultValues; //��������  ���� ����������
		 int resolvingColumn;	//����������� ������� �� ������� ����
		 int resolvingRow;		//����������� ������ �� ������� ����

		 void readFromFile(string filename);	//������ ������� ��� �� �����
		 void dualTaskSolution();				//������� ������������ ��� �� ������
		 void showTaskConditions();				//������ ������� ������
		 void findDecision();					//����� ������� ���
		 void printTable(int mode);				//������ ��������-�������(���� � ��� ��� ���� ������� Fi, � ���� � ��� ���)
		 bool ableToEnhance(int rowOpt, bool max);//�������� �� �������������� ��� ������� � ������ rowOpt
		 bool rebuildTable(int rowOpt, bool max);//����������� ��������-������� ��� ����������� ������ rowOpt
		 void showResult();						//������ ������������ ������� ������ �� �����
		 ~system_zlp(){
			 delete functionTarget;
			 delete systemValue;
			 delete signsInequality;
		 }
};

void system_zlp::readFromFile(string filename){
	string st;
	int tmpVal;
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
	signsInequality = new vector<int>(amountInequality);

	getline(inp, st);						//��������� ������� ����
	convertStr(st, functionTarget, &tmpVal);			//��������� � ����-�� � ������
	//��������� ������� ����������� � ��������� � � ������
	for (int i = 0; i < amountInequality; i++)
	{
		getline(inp, st);
		convertStr(st, &(systemValue->at(i)), &signsInequality->at(i));
	}
}

void system_zlp::dualTaskSolution(){
	//������� ������������ ������
	dualTask = true;

	//���� ����� ���������������, �� �������� �����������-����������� � ���� <=
	if (target == true)
	{
		for (int i = 0; i < amountInequality; i++)
			//���� ������� ���� >= 
			if ((*signsInequality)[i] == 1)
			{
				(*signsInequality)[i] = -1;
				for (int j = 0; j < amountX + 1; j++)
					if ((*systemValue)[i][j] != 0)
						(*systemValue)[i][j] *= -1;
			}
	}
	else
	//����� �������� ����������� � ���� >=
	{
		for (int i = 0; i < amountInequality; i++)
			//���� ������� ���� <=
			if ((*signsInequality)[i] == -1)
			{
				(*signsInequality)[i] = 1;
				for (int j = 0; j < amountX + 1; j++)
					if ((*systemValue)[i][j] != 0)
						(*systemValue)[i][j] *= -1;
			}
	}

	//���� � ������ ������ - ����� ���������������, �� � ������������ - �������������� � ��������
	if (target == true)
		target = false;
	else
		target = true;

	//������������ ����� ������� ����������� � ������� L
	vector<vector<double>> *newSystemValue = new vector<vector<double>>(amountX, vector<double>(amountInequality + 1, 0));
	vector<int> *newSignsInequality = new vector<int>(amountX);
	vector<double> *newFunctionTarget = new vector<double>(amountInequality + 1);
	
	for (int i = 0; i < amountX; i++)
		for (int j = 0; j < amountInequality + 1; j++)
			(*newSystemValue)[i][j] = 0;

	//������������ � ������� ���� ������������ ������
	//��������� �� ���������� ������� � ������������ ������ ������
	for (int i = 0; i < amountInequality; i++)
		(*newFunctionTarget)[i + 1] = (*systemValue)[i][0];
	(*newFunctionTarget)[0] = (*functionTarget)[0];

	//������� ������������� ����������� ���������� ����������������� ��������
	for (int i = 0; i < amountX; i++)
	{
		(*newSystemValue)[i][0] = (*functionTarget)[i + 1];
		for (int j = 1; j < amountInequality + 1; j++)
			(*newSystemValue)[i][j] = (*systemValue)[j - 1][i + 1];

		//����� ���������� ���������� ���������� �� ���������������
		if (target == true) //���� ����� ��������������� ���� � ������������ ������, �� ����� <=
		{
			for (int i = 0; i < amountX; i++)
				newSignsInequality->at(i) = -1;
		}
		else
			//���� ���� �������������� ���� � ������������ ������, �� ����� >=
			for (int i = 0; i < amountX; i++)
				newSignsInequality->at(i) = 1;
	}

	//����� ���������� � ������ ������ ��������� � ������ ����������� � ������������
	//����� ����������� � ������ ��������� � ������ ���������� � ������������
	int tmp = amountX;
	amountX = amountInequality;
	amountInequality = tmp;

	functionTarget = newFunctionTarget;
	systemValue = newSystemValue;
	signsInequality = newSignsInequality;
	resultValues.clear();
	//����� ������ ������������ ������ ������� ��������
	
	showTaskConditions();
	findDecision();
}

void system_zlp::showTaskConditions(){
	//������� ������� ���������� � �������
	char variable; //���������� ��, ����� � ��� ���������� (x ��� y)
	if (dualTask)
		variable = 'y';
	else
		variable = 'x';

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
		if (!dualTask)
			cout << "F = ";
		else
			cout << "L = ";
		if (functionTarget->at(0) != 0)
			cout << functionTarget->at(0) << "    +    ";
		for (int i = 1; i < amountX + 1; i++)
		{
			if (functionTarget->at(i) != 0)
			{
				if (i != amountX)
				{
					cout << functionTarget->at(i) << " * " << variable << i << "    +    ";
				}
				else
				{
					cout << functionTarget->at(i) << " * " << variable << i << " ,";
				}
			}
		}

		//������ ������� �����������
		cout << endl << "��� ������������:" << endl;
		for (int i = 0; i < amountInequality; i++)
		{
			for (int j = 1; j < amountX; j++)
			{
				cout << systemValue->at(i).at(j) << " * " << variable << j << "   +\t";
			}
			cout << systemValue->at(i).at(amountX) << " * " << variable << amountX;
			
			if (signsInequality->at(i) == -1)
				cout << " <= ";
			if (signsInequality->at(i) == 0)
				cout << " =  ";
			if (signsInequality->at(i) == 1)
				cout << " >= ";
			cout << systemValue->at(i).at(0) << ";" << endl;
		}
		cout << endl;
		//����� �������������� ����������� �� ����������������� xi
		for (int i = 0; i < amountX; i++)
		{
			cout << variable << i + 1 << " >= 0;\t";
		}
		cout << endl << "________________________________________________________________________" << endl;
	}
}

bool system_zlp::ableToEnhance(int rowOpt, bool max){
	//������� ���������, ���� �� ����������� ��� ��������(���������������, ��������������) ������� � ������ rowOpt
	int tableSizeCol = simplexTable->at(0).size();
	//���� ����� �������� ������������
	//�� ���� ������������� ����-�� � ������� ���� � ��������-�������
	//�.�. �� ������� '-' �� ������
	if (max == 1)
	{
		for (int i = 1; i < tableSizeCol; i++)
			if ((*simplexTable)[rowOpt][i] < -epsilon)
				return true;
	}
	//����� ���� ������������� ����-�� � ������� ���� � ��������-�������
	else
	{
		for (int i = 1; i < tableSizeCol; i++)
			if ((*simplexTable)[rowOpt][i] > epsilon)
			{
				return true;
			}
	}
	return false;
}

bool system_zlp::rebuildTable(int rowOpt, bool max){
	//������� ��������� �������� ��������� ��������-�������
	//� ������������ ������ row � ����������� ������������ ��� �����������
	resolvingColumn = 0;	//����������� �������
	resolvingRow = 0;		//����������� ������
	
	//����������� ��������-�������
	int tableSizeStr = simplexTable->size();
	int tableSizeCol = simplexTable->at(rowOpt).size();

	//�������� ������� ����������������� xi (��� �������� ����������)
	//� �� ���������������
	for (int i = 1 + rowOpt; i < tableSizeStr; i++)
	{
		if (simplexTable->at(i).at(0) < 0)
		{
			error_info = "�.�. ������� �����������";
			return false;
		}
		if (fabs(simplexTable->at(i).at(0)) < epsilon)
		{
			simplexTable->at(i).at(0) = 0.00001;
		}
	}

	//����� ������������ �������
	double valueTmp = 0;
	if (max == true) //���� ���� ��������, �� �������� ������������ ������������� �������
	{
		for (int j = 1; j < tableSizeCol; j++)
			if (valueTmp >(*simplexTable)[rowOpt][j])
			{
				resolvingColumn = j;
				valueTmp = (*simplexTable)[rowOpt][j];
			}
	}
	else //����� ������������ �������������
	{
		for (int j = 1; j < tableSizeCol; j++)
			if (valueTmp < (*simplexTable)[rowOpt][j])
			{
				resolvingColumn = j;
				valueTmp = (*simplexTable)[rowOpt][j];
			}
	}

	//����� ����������� ������
	valueTmp = INFINITY;
	for (int i = rowOpt + 1; i < tableSizeStr; i++)
	{
		//������� ����������� ������������� ��������� b[i] / c[i][resolvingColumn]
		if ((*simplexTable)[i][resolvingColumn] > 0)
		{
			double curVal = (*simplexTable)[i][0] / (*simplexTable)[i][resolvingColumn];
			if (curVal < valueTmp){
				valueTmp = curVal;
				resolvingRow = i;
			}
		}
	}
	//���� ��� ��������� �� ������������� ����������� ��������(���� �� ���� ������������ � �������), �� ������� ���
	if (valueTmp == INFINITY)
	{
		error_info = "�.�. ������� ������������ ";
		if (target)
			error_info += "������";
		else
			error_info += "�����";
		return false;
	}

	//������� �������� ����������(� ������� � ������ resolvingRow) �� ���������(� ������� resolvingColumn)
	int tmpIndex = freeIndices->at(resolvingColumn - 1);
	freeIndices->at(resolvingColumn - 1) = basisIndices->at(resolvingRow - 1 - rowOpt);
	basisIndices->at(resolvingRow - 1 - rowOpt) = tmpIndex;

	//������������ ����� ��������-�������
	vector<vector<double>> *newSimplexTable = new vector<vector<double>>(tableSizeStr, vector<double>(tableSizeCol, 0));

	//������������ �������� �� ����� ������������ ��������
	//a'[r][c] = 1 / a[r][c]
	(*newSimplexTable)[resolvingRow][resolvingColumn] = 1 / (*simplexTable)[resolvingRow][resolvingColumn];

	//������������ �������� �� ����� ����������� ������
	//a'[r][j] = a[r][j] / a[r][k] (j = 0,1,...,amountX + 1; j != c)
	for (int j = 0; j < tableSizeCol; j++)
		if (j != resolvingColumn)
			(*newSimplexTable)[resolvingRow][j] =
			(*simplexTable)[resolvingRow][j] / (*simplexTable)[resolvingRow][resolvingColumn];

	//������������ �������� �� ����� ������������ �������
	//a'[i][c] = - a[i][c] / a[r][c] (i = 0,1,...,amountInequality + 1; i != r)
	for (int i = 0; i < tableSizeStr; i++)
		if (i != resolvingRow)
			(*newSimplexTable)[i][resolvingColumn] =
			(-(*simplexTable)[i][resolvingColumn]) / (*simplexTable)[resolvingRow][resolvingColumn];

	//������������ �������� ���� ��������� �������
	//a'[i][j] = a[i][j] + a[r][j] * a'[i][c] (i = 0,1,2,..., amountInequality + 1; i != r; j = 0,1,2,..., amountX + 1; j != c)
	for (int i = 0; i < tableSizeStr; i++)
	{
		for (int j = 0; j < tableSizeCol; j++)
		{
			if (i != resolvingRow && j != resolvingColumn)
				(*newSimplexTable)[i][j] =
				(*simplexTable)[i][j] + (*simplexTable)[resolvingRow][j] * (*newSimplexTable)[i][resolvingColumn];
		}
	}
	//������ ��� ��������-������� �������
	(*simplexTable) = (*newSimplexTable);
	delete newSimplexTable;
	return true;
}

void system_zlp::findDecision(){
	//������� ���� ����������� ������� ���
	bool success = true; //������� ������ ��� ���������� �������

	//��������, ����� ��� bi >= 0
	//� ��������� ������ - ��������� �� 1 ���� �������� ������ ������ �����������
	for (int i = 0; i < amountInequality; i++)
		if ((*systemValue)[i][0] < 0)
		{
			(*signsInequality)[i] *= -1;
			for (int j = 0; j < amountX + 1; j++)
				if ((*systemValue)[i][j] != 0)
					(*systemValue)[i][j] = -(*systemValue)[i][j];
		}

	//������� ��������-�������
	int tableSizeStr = 0;
	int tableSizeCol = 0;

	//����������� ����������� �������� ��� �������
	int isArtificial = 0;		// ����� �� ������ ������ � �������������� ������ �������������� ������
	int amountArtificial = 0;	// ���������� ���������� ����������, ������� �� ������ � ����������� �����
	for (int i = 0; i < signsInequality->size(); i++)
	{
		//���� >=, �� ���������� ���������� ����� �� ������ -
		//������ ����� ������� ������������� ����������
		if (signsInequality->at(i) == 1)
			amountArtificial++;
		//���� = ��� >=, �� ����� ������� ������������� ����������
		if (signsInequality->at(i) > -1 && isArtificial == 0)
			isArtificial = 1;
	}

	//���� ���� ������������� �����
	if (isArtificial)
	{
		//�� ������� ��������� ������� ������� fi, ������� �������� ������ ���������� �������������� ������
		//� ������ ����� ��������� ������� ������� �������

		//������� ��������-�������
		tableSizeStr = amountInequality + 2;
		tableSizeCol = amountX + 1 + amountArtificial;

		//��������� ������ ��� ��������-�������, ������� ��������� � �������� ����������
		simplexTable = new vector<vector<double>>(tableSizeStr , vector<double>(tableSizeCol, 0));
		freeIndices = new vector<int>(amountX + amountArtificial, 0);
		basisIndices = new vector<int>(amountInequality, 0);
	}

	else
	{
		//����� ��������� ������� ������� ������� �� �������� ��������

		//������� ��������-�������
		tableSizeStr = amountInequality + 1;
		tableSizeCol = amountX + 1;

		//��������� ������ ��� ��������-�������, ������� ��������� � �������� ����������
		simplexTable = new vector<vector<double>>(tableSizeStr, vector<double>(tableSizeCol, 0));
		freeIndices = new vector<int>(amountX, 0);
		basisIndices = new vector<int>(amountInequality, 0);
	}
	//����������� �������� ��������� ���������� � ������� �������
	for (int i = 0; i < amountX; i++)
		(*freeIndices)[i] = i + 1;


	//������ ������ ����������, ���������� ������������� �������� ����������
	int currX = amountX + 1;	//������� ����� ���������� x
	int currKsi = -1;			//������� ����� ���������� ��� (��� �������� - � ������������� ������� ����������)

	int currBasis = 0;			//����� � ������� ��������
	int currFree = amountX;		//����� � ������� ��������

	//�������� �� ������� �����������
	for (int i = 0; i < amountInequality; i++)
	{
		//���� � �� ���� <= , ��
		//� ������ �������� ���������� ��������� ����������, ������� � ������ ���� � ���� ������
		if ((*signsInequality)[i] == -1)
		{
			(*basisIndices)[currBasis++] = currX++;
		}
		
		//���� � �� ���� =, ��
		//� ������ �������� ���������� ��������� ���������� �� �������������� ������
		if ((*signsInequality)[i] == 0)
		{
			(*basisIndices)[currBasis++] = currKsi--;
		}

		//���� � �� ���� >=, ��
		//� ������ �������� ���������� ��������� ���������� �� �������������� ������
		//� � ������ ��������� - ���������� ��� ���� ������, ��������� � � ������� �����������
		if ((*signsInequality)[i] == 1)
		{
			(*simplexTable)[i + 2][currFree + 1] = -1;
			(*freeIndices)[currFree++] = currX++;
			(*basisIndices)[currBasis++] = currKsi--;

		}
	}
	
	//������ ������ �������� ������� - ������� ����
	//� ����������� � ������ ����-�� ��� xi �� ������ '-'
	(*simplexTable)[0][0] = (*functionTarget)[0];
	for (int i = 0; i < amountX; i++)
	{
		(*simplexTable)[0][i + 1] = (*functionTarget)[i + 1];
		if (simplexTable->at(0).at(i + 1) != 0)
			simplexTable->at(0).at(i + 1) *= -1;
	}

	//��� ��������� ������ �������� ������� ���������
	//����������� ������� �� ��������� �������
	for (int i = 0; i < amountInequality; i++)
	{ 
		for (int j = 0; j < amountX + 1; j++) 
			(*simplexTable)[i + 1 + isArtificial][j] = (*systemValue)[i][j];
	}

	if (isArtificial)	//���� ���� ������������ �����
	{
		//���������� ������� Fi
		//���� ����������� ����� � ��������� ����������� ���
		vector<double>* Fi = new vector<double>(tableSizeCol, 0);
		for (int i = 0; i < amountInequality; i++)
		{
			if ((*basisIndices)[i] < 0)
				for (int j = 0; j < tableSizeCol; j++)
					(*Fi)[j] += (*simplexTable)[i + 2][j];
		}
		//�������� ��� ������������ ������� Fi � 1-�� ������ ��������-�������
		(*simplexTable)[1] = *Fi;



		//��������� ����������� ������� Fi 
		//(��� ������ ���� ����� 0 � ��� ������������ ��� ��������� ���������� ������ ���� ����� 0)
		success = true; //������� ������ ��� ���������� �������
		while (ableToEnhance(1, false) == true) //���� ����� �������� �������
		{
			success = rebuildTable(1, false);
			if (success == false)
			{
				break;
			}
			//������� �� ��������� ���������� �������������� ������
			for (int j = 0; j < simplexTable->at(0).size() - 1; j++)
			{
				if (freeIndices->at(j) < 0)
				{
					for (int i = 0; i < simplexTable->size(); i++)
					{
						simplexTable->at(i).erase(simplexTable->at(i).begin() + j + 1);
					}
					freeIndices->erase(freeIndices->begin() + j);
					j--;
				}
			}
		}
		//��������, ��� Fi = 0 ��� ���� ������������� ������ 0
		if ((*simplexTable)[1][0] < -epsilon)
		{
			error_info = "�.�. ������� ������� �����������";
			success = false;
		}
		for (int j = 1; j < simplexTable->at(1).size(); j++)
			if (abs((*simplexTable)[1][j]) > epsilon)
			{
				error_info = "�.�. ������� ������� �����������";
				success = false;
				break;
			}
		
		//���� Fi = 0 � ����-�� � ��� �������, �� ������ �� ����������� ������� ���� ���� ����� �������
		//������ ����� ������� ������� Fi � ��������� ����������� ������� ����
		if (success == true)
			simplexTable->erase(simplexTable->begin() + 1);
	}

	//���� ������� �������������� ������� Fi = 0
	if (success == true)
	{
		//��������� ����������� ��������-�������
		tableSizeStr = simplexTable->size();
		tableSizeCol = simplexTable->at(0).size();

		//����� ��������
		int iterationCount = 1;

		//������������ ���������� �������� �����
		for (int i = 0; i < currX - 1; i++)
			resultValues.push_back(0);
		for (int i = 0; i < tableSizeStr - 1; i++)
			resultValues[basisIndices->at(i) - 1] = (*simplexTable)[i + 1][0];
		cout << "_________________________________________________________________________________________________" << endl;
		cout << "�������� 0" << ":" << endl;
		cout << "������� ����:" << endl;
		showResult();

		//��������� ����������� ������� ����
		while (ableToEnhance(0, target))//���� ����� �������� ������� ����
		{ 
			//���������� ����� ��������-�������
			success = rebuildTable(0, target);
			if (success == false) //���� �������� ������ ��� ������������, �� ��������� ����
				break;
			//������������ �������� ����� �� ��������
			resultValues.clear();
			for (int i = 0; i < currX - 1; i++)
				resultValues.push_back(0);
			for (int i = 0; i < tableSizeStr - 1; i++)
				resultValues[basisIndices->at(i) - 1] = (*simplexTable)[i + 1][0];
			cout << "_________________________________________________________________________________________________" << endl;
			cout << "�������� " << iterationCount++ << ":" << endl;
			//cout << "��������-������� �� ���� ��������:" << endl;
			//printTable(0); //�������� ������� �� ���� ��������

			//������ �������� �����:
			cout << "������� ����:" << endl;
			showResult();
		}
	}
	if (success == false)	resultValues.clear();
	//������ ��������� �������
	cout << "_________________________________________________________________________________________________" << endl;
	cout << "�������� �������: " << endl;
	if (success == true)
		printTable(0);
	showResult();
}

void system_zlp::showResult(){
	//����� ���������� �� �����
	if (resultValues.size() > 0)
	{
		if (target == true)
			cout << "Max  ";
		else
			cout << "Min  ";
		//�������� ������� ���� �������� � simplexTable[0][0]
		if (!dualTask)
			cout << "F = ";
		else
			cout << "L = ";
		cout << (*simplexTable)[0][0] << endl;
		cout << endl;
		//����� �������� ����������� ���������� � ���������� ����������
		if (!dualTask)
		{
			cout << "X = ( ";
			for (int i = 0; i < amountX; i++)
				cout << "x" << i + 1 << "  ";
			cout << ") = (  ";
		}
		else
		{
			cout << "Y = ( ";
			for (int i = 0; i < amountX; i++)
				cout << "y" << i + 1 << "  ";
			cout << ") = (  ";
		}
		for (int i = 0; i < amountX; i++)
			cout << resultValues[i] << "  ";
		cout << ")" << endl;

		//������� ������� ����� ���������� ����������
		int amountExtra = 0;
		for (int i = 0; i < amountInequality; i++)
			if (signsInequality->at(i) != 0)
				amountExtra++;
		//����� ���������� ����������
		cout << "Z = ( ";
		for (int j = 0; j < amountExtra; j++)
			cout << "z" << j + 1 << " ";
		cout << ") = (  ";
		for (int j = 0; j < amountExtra; j++)
			cout << resultValues[j + amountX] << "  ";
		cout << ")" << endl;
	}
	else
	{
		//������� ��� � ����� ������� ���������� �������
		cout << "������� ��� " << error_info << endl;

	}
}

void system_zlp::printTable(int mode){
	//������ ��������-�������
	char variable;
	if (dualTask)
		variable = 'y';
	else
		variable = 'x';

	int tableSizeStr = simplexTable->size();		//�������
	int tableSizeCol = simplexTable->at(0).size();	//��������-�������

	cout << endl;
	//����� ������������ ��������� ����������
	cout << "\t\t��.";
	for (int i = 0; i < tableSizeCol - 1; i++)
		if ((*freeIndices)[i] > 0)
		{
			cout << "\t\t" << variable << freeIndices->at(i);
		}
		else
			cout << "\t\tE" << abs(freeIndices->at(i));
	cout << endl << endl;

	int indent = 0;
	//����� �������� �������
	for (int i = 0; i < tableSizeStr; i++)
	{
		//����� ������������ ������
		if (i == 0)
		{
			if (!dualTask)
				cout << "F\t\t";
			else
				cout << "L\t\t";
			indent = 1;
		}
		else if (i == 1 && mode == 1)
		{
			cout << "Fi\t\t";
			indent = 2;
		}
		else if (basisIndices->at(i - indent) > 0)
		{
			cout << variable << basisIndices->at(i - indent) << "\t\t";
		}
		else
			cout << "E" << abs(basisIndices->at(i - indent)) << "\t\t";

		//����� ����������� ������
		for (int j = 0; j < tableSizeCol; j++)
		{
			if (abs((*simplexTable)[i][j]) < epsilon)
				cout << setprecision(4) << "0" << "\t\t";
			else
				cout << setprecision(4) <<  simplexTable->at(i).at(j) << "\t\t";
		}
		cout << endl;
	}
	cout << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Rus");
	//������� � �������������� ����������� �������
	system_zlp mySystem;
	mySystem.readFromFile("input.txt");
	//��������� ����������� ������� �� �����
	mySystem.showTaskConditions();
	//������ ������ ���
	cout << "�������" << endl;
	mySystem.findDecision();
	cout << endl << endl << "##################################################################################################################" << endl;
	//������ ������������ ���
	cout << endl << "������� ������������ ������:" << endl;
	mySystem.dualTaskSolution();
	return 0;
}