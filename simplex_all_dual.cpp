#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#define epsilon 0.0000000001

using namespace std;
const int NotUsed = system("color F0");

//Строка, содержащая сообщение об ошибке
string error_info = "";

void convertStr(string st, vector<double> *targetFunction, int *signIneq){
	//Вспомогательная функция, которая из строки st получает массив targetFunction коэффициентов
	//targetFunction[0] = Свободному члену
	//targetFunction[1, ..., n] = коэффициенту при соответствующем x
	//В signIneq хранится знак ограничения неравенства
	int prev = 0;
	int next;
	vector<string> substrings;
	while ((next = st.find(" ", prev)) != string::npos){ //Пока в строке можно найти пробел
		string tmp = st.substr(prev, next - prev); //Вырезаем очередную подстроку
		substrings.push_back(st.substr(prev, next - prev)); //Добавляем её в список
		prev = next + 1; //Переходим к следующей подстроке
	}
	substrings.push_back(st.substr(prev, st.length()));

	//Преобразуем каждую подстроку в элемент массива targetFunction
	char sign = '+'; //Знак числа
	double number = 0;//Само число
	for (int i = 0; i < substrings.size(); i++)
	{
		string curStr = substrings.at(i);
		//Если подстрока - знак + или -, то или до неё был свободный член
		//либо заносим этот знак в sign
		if (curStr == "-" || curStr == "+")
		{
			if (number != 0)
			{
				targetFunction->at(0) = number;
				number = 0;
			}
			sign = curStr[0];
		}
		//Если подстрока - число, то запоминаем его
		if (curStr[0] >= '0' && curStr[0] <= '9' || (curStr[0] == '+' && curStr.length() > 1) || (curStr[0] == '-' && curStr.length() > 1))
		{
			number = stod(curStr);
			if (sign == '-')
				number = -number;
		}
		//Если подстрока начинается с 'x', то дальше следует индекс переменной
		if (curStr[0] == 'x'){
			int indexX = stoi(curStr.substr(1, curStr.length()));
			targetFunction->at(indexX) = number;
			number = 0;
		}
		//После знака <= записывается свободный член
		if (curStr == "<="){
			number = stod(substrings.at(i + 1));
			targetFunction->at(0) += number;
			*signIneq = -1;
		}
		//После знака >= записывается свободный член
		if (curStr == ">="){
			number = stod(substrings.at(i + 1));
			targetFunction->at(0) += number;
			*signIneq = 1;
		}
		//После знака = записывается свободный член
		if (curStr == "="){
			number = stod(substrings.at(i + 1));
			targetFunction->at(0) += number;
			*signIneq = 0;
		}
	}
}

class system_zlp{
public:	 bool dualTask = false; //Прямая задача или двойственная
		 int amountX;			//Количество переменных
		 int amountInequality;	//Количество неравенств
		 bool target;			//0 - минимум, 1 - максимум
		 vector<double> *functionTarget; //Функция цели (хранятся индексы при переменных)
		 vector<vector<double>> *systemValue;//Значения коэффициентов в системе
		 vector<int> *signsInequality;	//Знаки ограничений-неравенств (-1 - меньше, 0 - равно, 1 - больше либо равно)
		 vector <vector<double>> *simplexTable;//Симплекс-таблица
		 vector<int> *basisIndices;//Базисные переменные
		 vector<int> *freeIndices; //Свободные переменные
		 vector<double> resultValues; //Значения  всех переменных
		 int resolvingColumn;	//Разрешающий столбец на текущем шаге
		 int resolvingRow;		//Разрешающая строка на текущем шаге

		 void readFromFile(string filename);	//Чтение условия ЗЛП из файла
		 void dualTaskSolution();				//Решение двойственной ЗЛП из прямой
		 void showTaskConditions();				//Печать условий задачи
		 void findDecision();					//Поиск решения ЗЛП
		 void printTable(int mode);				//Печать симплекс-таблицы(если в ней ещё есть функция Fi, и если её уже нет)
		 bool ableToEnhance(int rowOpt, bool max);//Возможно ли оптимизировать еще функцию в строке rowOpt
		 bool rebuildTable(int rowOpt, bool max);//Перестройка симплекс-таблицы для оптимизации строки rowOpt
		 void showResult();						//Печать оптимального решения задачи на экран
		 ~system_zlp(){
			 delete functionTarget;
			 delete systemValue;
			 delete signsInequality;
		 }
};

void system_zlp::readFromFile(string filename){
	string st;
	int tmpVal;
	//Открываем файл для считывания исходных данных
	ifstream inp(filename);
	inp >> amountX;							//Считываем количество переменных
	inp >> amountInequality;				//Считываем количество неравенств
	inp >> st;								//Считываем максимум либо минимум функции цели	
	if (st == "max")
		target = true;
	else
		target = false;

	functionTarget = new vector<double>(amountX + 1, 0);
	systemValue = new vector<vector<double>>(amountInequality, vector<double>(amountX + 1, 0));
	signsInequality = new vector<int>(amountInequality);

	getline(inp, st);						//Считываем функцию цели
	convertStr(st, functionTarget, &tmpVal);			//Переводим её коэф-ты в массив
	//Считываем систему ограничений и добавляем её в массив
	for (int i = 0; i < amountInequality; i++)
	{
		getline(inp, st);
		convertStr(st, &(systemValue->at(i)), &signsInequality->at(i));
	}
}

void system_zlp::dualTaskSolution(){
	//Решение двойственной задачи
	dualTask = true;

	//Если нужно максимизировать, то приводим неравенства-ограничения в виду <=
	if (target == true)
	{
		for (int i = 0; i < amountInequality; i++)
			//Если имеется знак >= 
			if ((*signsInequality)[i] == 1)
			{
				(*signsInequality)[i] = -1;
				for (int j = 0; j < amountX + 1; j++)
					if ((*systemValue)[i][j] != 0)
						(*systemValue)[i][j] *= -1;
			}
	}
	else
	//Иначе приводим ограничения к виду >=
	{
		for (int i = 0; i < amountInequality; i++)
			//Если имеется знак <=
			if ((*signsInequality)[i] == -1)
			{
				(*signsInequality)[i] = 1;
				for (int j = 0; j < amountX + 1; j++)
					if ((*systemValue)[i][j] != 0)
						(*systemValue)[i][j] *= -1;
			}
	}

	//Если у прямой задачи - нужно максимизировать, то у двойственной - минимизировать и наоборот
	if (target == true)
		target = false;
	else
		target = true;

	//Формирование новой системы ограничений и функции L
	vector<vector<double>> *newSystemValue = new vector<vector<double>>(amountX, vector<double>(amountInequality + 1, 0));
	vector<int> *newSignsInequality = new vector<int>(amountX);
	vector<double> *newFunctionTarget = new vector<double>(amountInequality + 1);
	
	for (int i = 0; i < amountX; i++)
		for (int j = 0; j < amountInequality + 1; j++)
			(*newSystemValue)[i][j] = 0;

	//Коэффициенты в функции цели двойственной задачи
	//совпадают со свободными членами в ограничениях прямой задачи
	for (int i = 0; i < amountInequality; i++)
		(*newFunctionTarget)[i + 1] = (*systemValue)[i][0];
	(*newFunctionTarget)[0] = (*functionTarget)[0];

	//Матрица коэффициентов ограничений получается транспонированием исходной
	for (int i = 0; i < amountX; i++)
	{
		(*newSystemValue)[i][0] = (*functionTarget)[i + 1];
		for (int j = 1; j < amountInequality + 1; j++)
			(*newSystemValue)[i][j] = (*systemValue)[j - 1][i + 1];

		//Знаки полученных неравенств заменяются на противоположные
		if (target == true) //Если нужно максимизировать цель в двойственной задаче, то знаки <=
		{
			for (int i = 0; i < amountX; i++)
				newSignsInequality->at(i) = -1;
		}
		else
			//Если надо минимизировать цель в двойственной задачи, то знаки >=
			for (int i = 0; i < amountX; i++)
				newSignsInequality->at(i) = 1;
	}

	//Число неравенств в прямой задаче совпадает с числом неизвестных в двойственной
	//Число неизвестных в прямой совпадает с числом неравенств в двойственной
	int tmp = amountX;
	amountX = amountInequality;
	amountInequality = tmp;

	functionTarget = newFunctionTarget;
	systemValue = newSystemValue;
	signsInequality = newSignsInequality;
	resultValues.clear();
	//Затем решаем двойственную задачу обычным способом
	
	showTaskConditions();
	findDecision();
}

void system_zlp::showTaskConditions(){
	//Функция выводит информацию о системе
	char variable; //Определяет то, какие у нас переменные (x или y)
	if (dualTask)
		variable = 'y';
	else
		variable = 'x';

	if (amountX > 0 && amountInequality > 0) //Если исходные данные уже введены
	{
		cout << "Количество переменных = " << amountX << " Неравенств = " << amountInequality;
		cout << "\n________________________________________________________________________\n";
		cout << "Найти  ";
		//Если целью является максимизация, то target = true, если минимизация, то false
		if (target == true)
			cout << "максимум ";
		else
			cout << "минимум ";

		//Печать функции цели
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

		//Печать системы ограничений
		cout << endl << "при ограничениях:" << endl;
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
		//Вывод дополнительных ограничений на неотрицательность xi
		for (int i = 0; i < amountX; i++)
		{
			cout << variable << i + 1 << " >= 0;\t";
		}
		cout << endl << "________________________________________________________________________" << endl;
	}
}

bool system_zlp::ableToEnhance(int rowOpt, bool max){
	//Функция проверяет, есть ли возможность ещё улучшить(максимизировать, минимизировать) функцию в строке rowOpt
	int tableSizeCol = simplexTable->at(0).size();
	//Если целью является максимизация
	//то ищем отрицательные коэф-ты в функции цели в симплекс-таблице
	//т.к. мы вынесли '-' за скобки
	if (max == 1)
	{
		for (int i = 1; i < tableSizeCol; i++)
			if ((*simplexTable)[rowOpt][i] < -epsilon)
				return true;
	}
	//иначе ищем положительные коэф-ты в функции цели в симплекс-таблице
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
	//Функция позволяет получить следующую симплекс-таблицу
	//с оптимизацией строки row в направлении максимизации или минимизации
	resolvingColumn = 0;	//Разрешающий столбец
	resolvingRow = 0;		//Разрешающая строка
	
	//Размерность симплекс-таблицы
	int tableSizeStr = simplexTable->size();
	int tableSizeCol = simplexTable->at(rowOpt).size();

	//Проверка условия неотрицательности xi (для базисных переменных)
	//и их невырожденности
	for (int i = 1 + rowOpt; i < tableSizeStr; i++)
	{
		if (simplexTable->at(i).at(0) < 0)
		{
			error_info = "т.к. система несовместна";
			return false;
		}
		if (fabs(simplexTable->at(i).at(0)) < epsilon)
		{
			simplexTable->at(i).at(0) = 0.00001;
		}
	}

	//Поиск разрешающего столбца
	double valueTmp = 0;
	if (max == true) //Если ищем максимум, то выбираем максимальный отрицательный элемент
	{
		for (int j = 1; j < tableSizeCol; j++)
			if (valueTmp >(*simplexTable)[rowOpt][j])
			{
				resolvingColumn = j;
				valueTmp = (*simplexTable)[rowOpt][j];
			}
	}
	else //иначе максимальный положительный
	{
		for (int j = 1; j < tableSizeCol; j++)
			if (valueTmp < (*simplexTable)[rowOpt][j])
			{
				resolvingColumn = j;
				valueTmp = (*simplexTable)[rowOpt][j];
			}
	}

	//Поиск разрешающей строки
	valueTmp = INFINITY;
	for (int i = rowOpt + 1; i < tableSizeStr; i++)
	{
		//Находим минимальное положительное отношение b[i] / c[i][resolvingColumn]
		if ((*simplexTable)[i][resolvingColumn] > 0)
		{
			double curVal = (*simplexTable)[i][0] / (*simplexTable)[i][resolvingColumn];
			if (curVal < valueTmp){
				valueTmp = curVal;
				resolvingRow = i;
			}
		}
	}
	//Если все отношения не удовлетворяют необходимым условиям(хотя бы одно положительно и конечно), то решений нет
	if (valueTmp == INFINITY)
	{
		error_info = "т.к. функция неограничена ";
		if (target)
			error_info += "сверху";
		else
			error_info += "снизу";
		return false;
	}

	//Заменям базисную переменную(в таблице в строке resolvingRow) со свободной(в столбце resolvingColumn)
	int tmpIndex = freeIndices->at(resolvingColumn - 1);
	freeIndices->at(resolvingColumn - 1) = basisIndices->at(resolvingRow - 1 - rowOpt);
	basisIndices->at(resolvingRow - 1 - rowOpt) = tmpIndex;

	//Формирование новой симплекс-таблицы
	vector<vector<double>> *newSimplexTable = new vector<vector<double>>(tableSizeStr, vector<double>(tableSizeCol, 0));

	//Формирование значения на месте разрешающего элемента
	//a'[r][c] = 1 / a[r][c]
	(*newSimplexTable)[resolvingRow][resolvingColumn] = 1 / (*simplexTable)[resolvingRow][resolvingColumn];

	//формирование значений на месте разрешающей строки
	//a'[r][j] = a[r][j] / a[r][k] (j = 0,1,...,amountX + 1; j != c)
	for (int j = 0; j < tableSizeCol; j++)
		if (j != resolvingColumn)
			(*newSimplexTable)[resolvingRow][j] =
			(*simplexTable)[resolvingRow][j] / (*simplexTable)[resolvingRow][resolvingColumn];

	//формирование значений на месте разрешающего столбца
	//a'[i][c] = - a[i][c] / a[r][c] (i = 0,1,...,amountInequality + 1; i != r)
	for (int i = 0; i < tableSizeStr; i++)
		if (i != resolvingRow)
			(*newSimplexTable)[i][resolvingColumn] =
			(-(*simplexTable)[i][resolvingColumn]) / (*simplexTable)[resolvingRow][resolvingColumn];

	//формирование значений всей остальной таблицы
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
	//Делаем эту симплекс-таблицу текущей
	(*simplexTable) = (*newSimplexTable);
	delete newSimplexTable;
	return true;
}

void system_zlp::findDecision(){
	//Функция ищет оптимальное решение ЗЛП
	bool success = true; //Наличие ошибки при вычислении таблицы

	//Проверка, чтобы все bi >= 0
	//В противном случае - умножение на 1 всех индексов данной строки ограничений
	for (int i = 0; i < amountInequality; i++)
		if ((*systemValue)[i][0] < 0)
		{
			(*signsInequality)[i] *= -1;
			for (int j = 0; j < amountX + 1; j++)
				if ((*systemValue)[i][j] != 0)
					(*systemValue)[i][j] = -(*systemValue)[i][j];
		}

	//размеры симплекс-таблицы
	int tableSizeStr = 0;
	int tableSizeCol = 0;

	//Определение изначальных размеров для таблицы
	int isArtificial = 0;		// Нужно ли решать задачу с использованием метода искусственного базиса
	int amountArtificial = 0;	// Количество добавочных переменных, которые не войдут в изначальный базис
	for (int i = 0; i < signsInequality->size(); i++)
	{
		//Если >=, то добавочная переменная будет со знаком -
		//Значит нужно вводить искусственные переменные
		if (signsInequality->at(i) == 1)
			amountArtificial++;
		//Если = или >=, то нужно вводить искусственные переменные
		if (signsInequality->at(i) > -1 && isArtificial == 0)
			isArtificial = 1;
	}

	//Если есть искусственный базис
	if (isArtificial)
	{
		//То сначала вычисляем минимум функции fi, которая является суммой переменных искусственного базиса
		//и только затем вычисляем оптимум целевой функции

		//Размеры симплекс-таблицы
		tableSizeStr = amountInequality + 2;
		tableSizeCol = amountX + 1 + amountArtificial;

		//Выделение памяти под симплекс-таблицу, индексы свободных и базисных переменных
		simplexTable = new vector<vector<double>>(tableSizeStr , vector<double>(tableSizeCol, 0));
		freeIndices = new vector<int>(amountX + amountArtificial, 0);
		basisIndices = new vector<int>(amountInequality, 0);
	}

	else
	{
		//Иначе вычисляем оптимум целевой функции по обычному сценарию

		//Размеры симплекс-таблицы
		tableSizeStr = amountInequality + 1;
		tableSizeCol = amountX + 1;

		//Выделение памяти под симплекс-таблицу, индексы свободных и базисных переменных
		simplexTable = new vector<vector<double>>(tableSizeStr, vector<double>(tableSizeCol, 0));
		freeIndices = new vector<int>(amountX, 0);
		basisIndices = new vector<int>(amountInequality, 0);
	}
	//Соотнесение индексов свободных переменных с номером столбца
	for (int i = 0; i < amountX; i++)
		(*freeIndices)[i] = i + 1;


	//Анализ знаков неравенств, добавление искусственных базисных переменных
	int currX = amountX + 1;	//текущий номер переменных x
	int currKsi = -1;			//текущий номер переменных кси (для различий - в отрицательном порядке нумеруются)

	int currBasis = 0;			//номер в массиве индексов
	int currFree = amountX;		//номер в массиве индексов

	//Проходим по каждому неравенству
	for (int i = 0; i < amountInequality; i++)
	{
		//Если в нём знак <= , то
		//в список базисных переменных добавляем добавочную, которая и должна быть в этой строке
		if ((*signsInequality)[i] == -1)
		{
			(*basisIndices)[currBasis++] = currX++;
		}
		
		//Если в нём знак =, то
		//в список базисных переменных добавляем переменную из искусственного базиса
		if ((*signsInequality)[i] == 0)
		{
			(*basisIndices)[currBasis++] = currKsi--;
		}

		//Если в нём знак >=, то
		//в список базисных переменных добавляем переменную из искусственного базиса
		//а в список свободных - добавочную для этой строки, добавляем её в систему ограничений
		if ((*signsInequality)[i] == 1)
		{
			(*simplexTable)[i + 2][currFree + 1] = -1;
			(*freeIndices)[currFree++] = currX++;
			(*basisIndices)[currBasis++] = currKsi--;

		}
	}
	
	//Первая строка симлпекс таблицы - Функция цели
	//с вынесенными в скобки коэф-ми при xi со знаком '-'
	(*simplexTable)[0][0] = (*functionTarget)[0];
	for (int i = 0; i < amountX; i++)
	{
		(*simplexTable)[0][i + 1] = (*functionTarget)[i + 1];
		if (simplexTable->at(0).at(i + 1) != 0)
			simplexTable->at(0).at(i + 1) *= -1;
	}

	//Все остальные строки симплекс таблицы повторяют
	//аналогичные строчки из уравнений системы
	for (int i = 0; i < amountInequality; i++)
	{ 
		for (int j = 0; j < amountX + 1; j++) 
			(*simplexTable)[i + 1 + isArtificial][j] = (*systemValue)[i][j];
	}

	if (isArtificial)	//Если есть искуственный базис
	{
		//Заполнение функции Fi
		//путём складывания строк с базисными переменными Кси
		vector<double>* Fi = new vector<double>(tableSizeCol, 0);
		for (int i = 0; i < amountInequality; i++)
		{
			if ((*basisIndices)[i] < 0)
				for (int j = 0; j < tableSizeCol; j++)
					(*Fi)[j] += (*simplexTable)[i + 2][j];
		}
		//Копируем эти коэффициенты функции Fi в 1-ую строку симплекс-таблицы
		(*simplexTable)[1] = *Fi;



		//Выполняем минимизацию функции Fi 
		//(Она должна быть равна 0 и все коэффициенты при свободных переменных должны быть равны 0)
		success = true; //Наличие ошибки при вычислении таблицы
		while (ableToEnhance(1, false) == true) //Пока можно улучшить таблицу
		{
			success = rebuildTable(1, false);
			if (success == false)
			{
				break;
			}
			//удаляем из свободных переменные искусственного базиса
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
		//Проверка, что Fi = 0 при всех коэффициентах равных 0
		if ((*simplexTable)[1][0] < -epsilon)
		{
			error_info = "т.к. система условий несовместна";
			success = false;
		}
		for (int j = 1; j < simplexTable->at(1).size(); j++)
			if (abs((*simplexTable)[1][j]) > epsilon)
			{
				error_info = "т.к. система условий несовместна";
				success = false;
				break;
			}
		
		//Если Fi = 0 и коэф-ты в ней нулевые, то задача на оптимизацию функции цели тоже имеет решение
		//значит можно удалять строчку Fi и выполнять оптимизацию функции цели
		if (success == true)
			simplexTable->erase(simplexTable->begin() + 1);
	}

	//Если удалось минимизировать функцию Fi = 0
	if (success == true)
	{
		//Обновляем размерность симплекс-таблицы
		tableSizeStr = simplexTable->size();
		tableSizeCol = simplexTable->at(0).size();

		//номер итерации
		int iterationCount = 1;

		//Формирование начального опорного плана
		for (int i = 0; i < currX - 1; i++)
			resultValues.push_back(0);
		for (int i = 0; i < tableSizeStr - 1; i++)
			resultValues[basisIndices->at(i) - 1] = (*simplexTable)[i + 1][0];
		cout << "_________________________________________________________________________________________________" << endl;
		cout << "Итерация 0" << ":" << endl;
		cout << "Опорный план:" << endl;
		showResult();

		//Выполняем оптимизацию функции цели
		while (ableToEnhance(0, target))//Пока можно улучшить функцию цели
		{ 
			//Построение новой симплекс-таблицы
			success = rebuildTable(0, target);
			if (success == false) //Если возникла ошибка при перестроении, то прерываем цикл
				break;
			//Формирование опорного плана на итерации
			resultValues.clear();
			for (int i = 0; i < currX - 1; i++)
				resultValues.push_back(0);
			for (int i = 0; i < tableSizeStr - 1; i++)
				resultValues[basisIndices->at(i) - 1] = (*simplexTable)[i + 1][0];
			cout << "_________________________________________________________________________________________________" << endl;
			cout << "Итерация " << iterationCount++ << ":" << endl;
			//cout << "Симплекс-таблица на этой итерации:" << endl;
			//printTable(0); //печатаем таблицу на этой итерации

			//Печать опорного плана:
			cout << "Опорный план:" << endl;
			showResult();
		}
	}
	if (success == false)	resultValues.clear();
	//Печать итогового решения
	cout << "_________________________________________________________________________________________________" << endl;
	cout << "ИТОГОВОЕ РЕШЕНИЕ: " << endl;
	if (success == true)
		printTable(0);
	showResult();
}

void system_zlp::showResult(){
	//Вывод результата на экран
	if (resultValues.size() > 0)
	{
		if (target == true)
			cout << "Max  ";
		else
			cout << "Min  ";
		//Значение функции цели хранится в simplexTable[0][0]
		if (!dualTask)
			cout << "F = ";
		else
			cout << "L = ";
		cout << (*simplexTable)[0][0] << endl;
		cout << endl;
		//Вывод значения изначальных переменных и добавочных переменных
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

		//Подсчёт сколько всего добавочных переменных
		int amountExtra = 0;
		for (int i = 0; i < amountInequality; i++)
			if (signsInequality->at(i) != 0)
				amountExtra++;
		//вывод добавочных переменных
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
		//Решения нет и вывод причины отсутствия решения
		cout << "Решения нет " << error_info << endl;

	}
}

void system_zlp::printTable(int mode){
	//Печать симплекс-таблицы
	char variable;
	if (dualTask)
		variable = 'y';
	else
		variable = 'x';

	int tableSizeStr = simplexTable->size();		//размеры
	int tableSizeCol = simplexTable->at(0).size();	//симплекс-таблицы

	cout << endl;
	//Вывод наименований свободных переменных
	cout << "\t\tСв.";
	for (int i = 0; i < tableSizeCol - 1; i++)
		if ((*freeIndices)[i] > 0)
		{
			cout << "\t\t" << variable << freeIndices->at(i);
		}
		else
			cout << "\t\tE" << abs(freeIndices->at(i));
	cout << endl << endl;

	int indent = 0;
	//Вывод значений таблицы
	for (int i = 0; i < tableSizeStr; i++)
	{
		//Вывод наименования строки
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

		//Вывод содержимого строки
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
	//Создаем и инициализируем изначальную систему
	system_zlp mySystem;
	mySystem.readFromFile("input.txt");
	//Дублируем изначальные условия на экран
	mySystem.showTaskConditions();
	//Решаем прямую ЗЛП
	cout << "Решение" << endl;
	mySystem.findDecision();
	cout << endl << endl << "##################################################################################################################" << endl;
	//Решаем двойственную ЗЛП
	cout << endl << "Решение двойственной задачи:" << endl;
	mySystem.dualTaskSolution();
	return 0;
}