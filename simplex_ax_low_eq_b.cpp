#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#define epsilon 0.0000000001

using namespace std;
const int NotUsed = system("color F0");

void convertStr(string st, vector<double> *targetFunction){ 
	//Вспомогательная функция, которая из строки st получает массив targetFunction коэффициентов
	//targetFunction[0] = Свободному члену
	//targetFunction[1, ..., n] = коэффициенту при соответствующем x
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
		}
	}
}

class system_zlp{
private: int amountX;			//Количество переменных
		 int amountInequality;	//Количество неравенств
		 bool target;			//0 - минимум, 1 - максимум
		 vector<double> *functionTarget; //Функция цели (хранятся индексы при переменных)
		 vector<vector<double>> *systemValue;//Значения коэффициентов в системе
		 vector<vector<double>> *simplexTable;//Симплекс-таблица
		 vector<int> *basisIndices;//Базисные переменные
		 vector<int> *freeIndices; //Свободные переменные
		 vector<double> resultValues; //Значения  всех переменных
		 int resolvingColumn;		//Разрешающий столбец на текущем шаге
		 int resolvingRow;			//Разрешающая строка на текущем шаге

public:  void readFromFile(string filename);	//Прочитать условия ЗЛП из файла
		 void showTaskConditions();				//Печать условий задачи
		 bool ableToEnhance();					//Возможность улучшения функции цели
		 void findDecision();					//Поиск решения ЗЛП
		 void showResult();						//Печать результата
		 void writeResult(string filename);		//Запись результата в файла
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
	//Открываем файл для считывание исходных данных
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

	getline(inp, st);						//Считываем функцию цели
	convertStr(st, functionTarget);			//Переводим её коэф-ты в массив
	//Считываем систему ограничений и добавляем её в массив
	for (int i = 0; i < amountInequality; i++)
	{
		getline(inp, st);
		convertStr(st, &(systemValue->at(i)));
	}
}

void system_zlp::showTaskConditions(){
	//Функция выводит информацию о системе
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

		//Печать системы ограничений
		cout << endl << "при ограничениях:" << endl;
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
		//Вывод дополнительных ограничений на неотрицательность xi
		for (int i = 0; i < amountX; i++)
			cout << "x" << i + 1 << " >= 0;\t";
		cout << endl << "________________________________________________________________________" << endl;
	}
}

bool system_zlp::ableToEnhance(){
	//Функция проверяет, есть ли возможность ещё улучшить(максимизировать, минимизировать) функцию цели
	int size = amountX + 1;
	//Если целью является максимизация
	//то ищем отрицательные коэф-ты в функции цели в симплекс-таблице
	//т.к. мы вынесли '-' за скобки
	if (target == 1)
	{
		for (int i = 1; i < size; i++)
			if ((*simplexTable)[0][i] < - epsilon)
				return true;
	}
	//иначе ищем положительные коэф-ты в функции цели в симплекс-таблице
	else
	{
		for (int i = 1; i < size; i++)
			if ((*simplexTable)[0][i] > epsilon)
				return true;
	}
	return false;
}

void system_zlp::findDecision(){
	//Функция ищет оптимальное решение ЗЛП

	//Изначальная инициализация списка свободных переменных
	freeIndices = new vector<int>(amountX, 0);
	for (int i = 0; i < amountX; i++)
		freeIndices->at(i) = i + 1;

	//Изначальная инициализация списка базисных переменных
	basisIndices = new vector<int>(amountInequality, 0);
	for (int i = 0; i < amountInequality; i++)
		basisIndices->at(i) = i + 1 + amountX;

	//Создаём и инициализируем начальную  симплекс-таблицу
	simplexTable = new vector<vector<double>>(amountInequality + 1, vector<double>(amountX + 1, 0));

	//Первая строка симлпекс таблицы -- Функция цели
	//с вынесенными в скобки коэф-ми при xi со знаком '-'
	simplexTable->at(0) = *functionTarget;
	for (int i = 0; i < amountX; i++)
	{
		if (simplexTable->at(0).at(i + 1) != 0)
			simplexTable->at(0).at(i + 1) *= -1;
	}
	//Все остальные строки симплекс таблицы повторяют
	//аналогичные строчки из уравнений системы (при переносе bi влево)
	for (int i = 0; i < amountInequality; i++)
	{
		simplexTable->at(i + 1) = systemValue->at(i);
	}

	bool imposible = false;			//Флажок возможных ошибок
	//Повторяем, пока возможно улучшить функцию цели
	while (ableToEnhance() == true)
	{
		resolvingColumn = 0;	//Разрешающий столбец
		resolvingRow = 0;		//Разрешающая строка

		//Проверка условия неотрицательности xi (для базисных переменных)
		//и их невырожденности
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
		//Если базисная переменная отрицательна
		if (imposible)
			break;

		//Поиск разрешающего столбца
		double valueTmp = 0;
		if (target == true) //Если ищем максимум, то выбираем максимальный отрицательный элемент
		{
			for (int i = 0; i < amountX; i++)
				if (valueTmp > (*simplexTable)[0][i + 1])
				{
					resolvingColumn = i + 1;
					valueTmp = (*simplexTable)[0][i + 1];
				}
		}
		else //иначе максимальный положительный
		{
			for (int i = 0; i < amountX; i++)
				if (valueTmp < (*simplexTable)[0][i + 1])
				{
					resolvingColumn = i + 1;
					valueTmp = (*simplexTable)[0][i + 1];
				}
		}

		//Поиск разрешающей строки
		valueTmp = INFINITY;
		for (int i = 0; i < amountInequality; i++)
		{
			//Находим минимальное положительное отношение b[i] / c[i][resolvingColumn]
			if ((*simplexTable)[i + 1][resolvingColumn] > 0)
			{
				double curVal = (*simplexTable)[i + 1][0] / (*simplexTable)[i + 1][resolvingColumn];
				if (curVal < valueTmp){
					valueTmp = curVal;
					resolvingRow = i + 1;
				}
			}
		}
		//Если все отношения не удовлетворяют необходимым условиям(хотя бы одно положительно и конечно), то решений нет
		if (valueTmp == INFINITY)
		{
			imposible = true;
			break;
		}

		//Заменям базисную переменную(в таблице в строке resolvingRow) со свободной(в столбце resolvingColumn)
		int tmpIndex = freeIndices->at(resolvingColumn - 1);
		freeIndices->at(resolvingColumn - 1) = basisIndices->at(resolvingRow - 1);
		basisIndices->at(resolvingRow - 1) = tmpIndex;

		//Формирование новой симплекс-таблицы
		vector<vector<double>> *newSimplexTable = new vector<vector<double>>(amountInequality + 1, vector<double>(amountX + 1, 0));

		//Формирование значения на месте разрешающего элемента
		//a'[r][c] = 1 / a[r][c]
		newSimplexTable->at(resolvingRow).at(resolvingColumn) = 1 / (*simplexTable)[resolvingRow][resolvingColumn];

		//формирование значений на месте разрешающей строки
		//a'[r][j] = a[r][j] / a[r][k] (j = 0,1,...,amountX + 1; j != c)
		for (int j = 0; j < amountX + 1; j++)
			if (j != resolvingColumn)
				newSimplexTable->at(resolvingRow).at(j) = 
				(*simplexTable)[resolvingRow][j] / (*simplexTable)[resolvingRow][resolvingColumn];

		//формирование значений на месте разрешающего столбца
		//a'[i][c] = - a[i][c] / a[r][c] (i = 0,1,...,amountInequality + 1; i != r)
		for (int i = 0; i < amountInequality + 1; i++)
			if (i != resolvingRow)
				newSimplexTable->at(i).at(resolvingColumn) =
				(-(*simplexTable)[i][resolvingColumn]) / (*simplexTable)[resolvingRow][resolvingColumn];

		//формирование значений всей остальной таблицы
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
		//Делаем эту симплекс-таблицу текущей
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
	if (imposible == true) //Если была какая то ошибка в задаче
	{
		cout << "Нет решений" << endl;
	}
	else
	{
		//в simplexTable[0][0] будет хранится оптимальное значение функции цели,
		//при значениях свободных переменных, равных нулю
		//Для нахождения базисного решения соотносим номера строк из симплекс-таблицы
		//со списком, в котором хранятся номера базисных переменных
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
	//Вывод результата на экран
	cout << "Результат:" << endl;
	if (resultValues.size() > 0)
	{
		if (target == true)
			cout << "Max  ";
		else
			cout << "Min  ";
		//Оптимальное значение функции цели хранится в simplexTable[0][0]
		cout << "F = " << (*simplexTable)[0][0] << endl;
		cout << endl;
		//Вывод значения изначальных переменных и добавочных переменных
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
		
		//Проверка на единственность решения.
		bool singleDecision = true;
		for (int j = 0; j < amountX; j++)
			if ((*simplexTable)[0][j + 1] == 0)
			{
				singleDecision = false;
				break;
			}
		if (!singleDecision)
			cout << "Решение не единственно, т.к. в линейной форме входят не все переменные" << endl;
	}
	else
	{
		cout << "Решения нет" << endl;
		
		//Определение ограниченности функции (все отношения равны бесконечности)
		if (resolvingRow == 0){
			if (target == true)
				cout << "т.к. функция неограничена сверху" << endl;
			else
				cout << "т.к. функция неограничена  снизу" << endl;
		}
	}
}
void system_zlp::writeResult(string filename){
	//Печать результата в файл filename
	ofstream out(filename);
	out << "Результат:" << endl;
	//Если нет ошибок при решении:
	if (resultValues.size() > 0)
	{
		if (target == true)
			out << "Max  ";
		else
			out << "Min  ";
		//Оптимальное значение функции цели хранится в simplexTable[0][0]
		out << "F = " << (*simplexTable)[0][0] << endl;
		out << endl;
		//Вывод значения изначальных переменных и добавочных переменных
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

		//Проверка на единственность решения.
		bool singleDecision = true;
		for (int j = 0; j < amountX; j++)
			if ((*simplexTable)[0][j + 1] == 0)
			{
				singleDecision = false;
				break;
			}
		if (!singleDecision)
			out << "Решение не единственно, т.к. в линейной форме входят не все переменные" << endl;
	}
	else
		out << "Решения нет" << endl;
	out.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Rus");
	//Создаем и инициализируем изначальную систему
	system_zlp mySystem;
	mySystem.readFromFile("input.txt");
	//Дублируем изначальные условия на экран
	mySystem.showTaskConditions();
	//Решаем задачу
	mySystem.findDecision();
	//Печатаем результат на экран и в файл
	mySystem.showResult();
	mySystem.writeResult("output.txt");
	return 0;
}
