// Итерационный метод решения матричных игр
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <math.h>

//const int NotUsed = system("color F0");

using namespace std;

// собственная функция для разделения строк на подстроки
string *split(string start_str, string delim, int *count_words){
	int len = start_str.length();
	int len_delim = delim.length();
	bool finish = false;
	bool same_symb = false;
	*count_words = 1;
	int i, j;

	//Подсчёт количества слов
	for (i = 0; i < len; i++)
	{
		if (start_str[i] == delim[0])
		{
			same_symb = true;
			for (j = 1; j < len_delim; j++)
			{
				if (len <= i + j)
				{
					finish = true;
					break;
				}
				if (start_str[i + j] != delim[j])
				{
					same_symb = false;
					break;
				}
			}
			if (!finish && same_symb == true)
			{
				(*count_words)++;
				i += j;
				same_symb = false;
			}
		}
		if (finish)
			break;
	}

	//Создаём массив для строк
	string *mass_words = new string[*count_words];
	
	//Выделяем отдельные слова
	int word_num = 0;
	string cur_word = "";
	for (i = 0; i < len; i++)
	{
		if (start_str[i] == delim[0])
		{
			same_symb = true;
			
			for (j = 1; j < len_delim; j++)
			{
				if (len <= i + j)
				{
					finish = true;
					break;
				}
				if (start_str[i + j] != delim[j])
				{
					same_symb = false;
					break;
				}
			}
			if (!finish && same_symb == true)
				i += j-1;
		}
		if (same_symb == true)
		{
			mass_words[word_num] = cur_word;
			word_num++;
			cur_word = "";
			same_symb = false;
		}
		else
		{
			cur_word += start_str[i];
		}
	}
	//Добавление последнего слова из конца строки
	if (word_num < *count_words)
		mass_words[word_num] = cur_word;

	//Возвращается массив строк
	return mass_words;
}

// считываниие платежной матрицы игры и точности из файла
bool read_from_file(string filename, float ***game_matrix, int *m, int *n, float *epsilon)
{
	//Считывание данных из файла
	ifstream inp(filename);

	//Проверка на успешность открытия
	if (inp.is_open())
	{
		string tmp;
		getline(inp, tmp);
		//Точность нахождения цены игры
		*epsilon = stof(tmp);

		//Считывание матрицы
		int size;

		//Размеры платежной матрицы
		getline(inp, tmp);
		string *m_n_sizes = split(tmp, " ", &size);
		*m = stoi(m_n_sizes[0]);
		*n = stoi(m_n_sizes[1]);
		delete[] m_n_sizes;
		
		//Создание платежной матрицы
		*game_matrix = new float*[*m];
		for (int i = 0; i < *m; i++)
			(*game_matrix)[i] = new float[*n];
		//Считывание самой матрицы
		string *substrings;
		for (int i = 0; i < *m; i++)
		{
			getline(inp, tmp);
			substrings = split(tmp, " ", &size);
			for (int j = 0; j < *n; j++)
			{
				(*game_matrix)[i][j] = stof(substrings[j]);
			}
			delete[] substrings;
		}
		return true;
	}
	else
	{
		return false;
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Rus");
	int m, n;						//Размеры матрицы
	float **game_matrix = nullptr;	//Платёжная матрица игры
	float epsilon;					//Точность

	//Чтение данных задачи
	read_from_file("input.txt", &game_matrix, &m, &n, &epsilon);

	//Вывод данных на экран монитора
	cout << "ПРОГРАММА РЕШАЕТ МАТРИЧНУЮ ИГРУ ИТЕРАЦИОННЫМ МЕТОДОМ" << endl;
	cout << "####################################################" << endl;
	cout << "Точность вычислений: " << epsilon << endl;
	cout << "Платежная матрица имеет вид:" << endl;
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
			cout << game_matrix[i][j] << "\t";
		cout << endl;
	}
	cout << endl << "####################################################" << endl;

	cout << endl << "Таблица итераций:" << endl;

	float max_elo, min_elo;

	int iter_numb = 1;						//Текущий номер итерации
	int index_i = 0, index_j = 0;			//Номера чистых стратегий игрока А и В
	int *ki = new int[m];					//Количество ходов Ai игрока A, выбранных за n итераций
	for (int i = 0; i < m; i++)
		ki[i] = 0;

	int *mj = new int[n];					//Количество ходов Bj игрока B, выбранных за n итераций
	for (int j = 0; j < n; j++)
		mj[j] = 0;

	float min_price;						//Минимально допустимый выигрыш игрока А, деленный на кол-во итераций		
	float max_price;						//Максимально возможный проигрыш игрока В, деленный на кол-во итераций
	float price_average;					//Приближенное значение цены игры

	float *player_a_decisions = new float[n];//хранятся значения Bj при принятии стратегий игроком A
	float *player_b_decisions = new float[m];//хранятся значения Ai при принятии стратегий игроком В

	//Поиск стратегии, которую выберет игрок А (с наибольшим выигрышем)
	max_elo= game_matrix[0][0];
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			if (game_matrix[i][j] > max_elo)
			{
				max_elo = game_matrix[i][j];
				index_i = i;
			}
	ki[index_i] += 1;		//Добавляем +1 к стратегии, которую выьрал игрок А

	//Поиск стратегии, которую выберет игрок В (с наименьшим проигрышем)
	min_elo = game_matrix[index_i][0];
	for (int j = 0; j < n; j++)
	{
		if (min_elo > game_matrix[index_i][j])
		{
			min_elo = game_matrix[index_i][j];
			index_j = j;
		}
		player_a_decisions[j] = game_matrix[index_i][j];
	}
	int prev_i = index_i, prev_j = index_j;
	//Ищем стратегию, которой ответит игрок А на следующий ход
	max_elo = game_matrix[0][index_j];
	index_i = 0;
	for (int i = 0; i < m; i++)
	{
		player_b_decisions[i] = game_matrix[i][index_j];
		if (player_b_decisions[i] > max_elo)
		{
			max_elo = player_b_decisions[i];
			index_i = i;
		}
	}
	mj[index_j] += 1;		//Добавляем +1 к стратегии, которую выбрал игрок В

	//Вычисление минимальной, максимальной и средней цен
	min_price = player_a_decisions[index_j] / iter_numb;
	max_price = player_b_decisions[index_i] / iter_numb;
	price_average = (min_price + max_price) / 2;

	//Шапка таблицы
	string table_hat = "";
	table_hat += "\tk\ti\t";
	for (int j = 0; j < n; j++)
		table_hat += "B" + to_string(j + 1) + "\t";
	table_hat += "|\tj\t";
	for (int i = 0; i < m; i++)
		table_hat += "A" + to_string(i + 1) + "\t";
	table_hat += "v_low\tv_high\tv*" ;
	cout << table_hat << endl << endl;

	//Вывод первой строки таблицы
	cout << "\t" << iter_numb << "\t";
	cout << prev_i + 1 << "\t";
	for (int j = 0; j < n; j++)
	{
		if (index_j == j)
			cout << "*";
		cout << player_a_decisions[j] << "\t";
	}
	cout << "|\t" << prev_j + 1 << "\t";
	for (int i = 0; i < m; i++)
	{
		if (index_i == i)
			cout << "*";
		cout << player_b_decisions[i] << "\t";
	}
	cout << min_price << "\t" << max_price << "\t" << price_average << endl;


	//Продолжаем итерации, пока разница между верхней и средней или средней и нижней ценами больше заданной точности
	while (fabs(min_price - price_average) > epsilon || fabs(max_price - price_average) > epsilon)
	{
		iter_numb++;			//Прибавляем номер итерации
		//Сохраняем индексы выбранных стратегий с прошлой итерации ( у игрока А в index_i лежит его текущая стратегия)
		prev_i = index_i; prev_j = index_j;
		ki[index_i] += 1;

		//Добавляем к элементам таблицы значения из соответствущей строки
		for (int j = 0; j < n; j++)
			player_a_decisions[j] += game_matrix[prev_i][j];

		//Ищем стратегию, которую выберет игрок В и её индекс
		min_elo = player_a_decisions[0];
		index_j = 0;
		for (int j = 0; j < n; j++)
			if (player_a_decisions[j] < min_elo)
			{
				min_elo = player_a_decisions[j];
				index_j = j;
			}

		mj[index_j] += 1;
		//Добавляем к элементам таблицы значения из соответствующего столбца
		for (int i = 0; i < m; i++)
			player_b_decisions[i] += game_matrix[i][index_j];
		//Определяем номер стратегии для игрока А на следующую итерацию
		max_elo = player_b_decisions[0];
		index_i = 0;
		for (int i = 0; i < m; i++)
			if (player_b_decisions[i] > max_elo)
			{
				max_elo = player_b_decisions[i];
				index_i = i;
			}

		//Обновляем цены для игры
		min_price = player_a_decisions[index_j] / iter_numb;
		max_price = player_b_decisions[index_i] / iter_numb;
		price_average = (min_price + max_price) / 2;

		//И печатаем строку для текущей итерации
		cout << "\t" << iter_numb << "\t";
		cout << prev_i + 1 << "\t";
		for (int j = 0; j < n; j++)
		{
			if (index_j == j)
				cout << "*";
			cout << player_a_decisions[j] << "\t";
		}
		cout << "|\t" << index_j + 1 << "\t";
		for (int i = 0; i < m; i++)
		{
			if (index_i == i)
				cout << "*";
			cout << player_b_decisions[i] << "\t";
		}
		cout << min_price << "\t" << max_price << "\t" << price_average << endl;
	}

	cout << endl << endl << "Итерации Окончены" << endl;

	//Получение вероятностей смешанной стратегии:
	float *pi = new float[m];
	float *qj = new float[n];
	for (int i = 0; i < m; i++)
		pi[i] = (float)ki[i] / iter_numb;
	for (int j = 0; j < n; j++)
		qj[j] = (float)mj[j] / iter_numb;

	//Вывод ответа
	int precision = to_string((int)(1 / epsilon)).size();
	cout << std::setprecision(precision);
	cout << "РЕЗУЛЬТАТ:" << endl;
	cout << "Цена игры v ~ " << price_average << endl;
	cout << "S(A) ~ (  ";
	for (int i = 0; i < m; i++)
		cout << pi[i] << "  ";
	cout << ");\n";
	cout << "S(B) ~ (  ";
	for (int j = 0; j < n; j++)
		cout << qj[j] << "  ";
	cout << ");\n";
	return 0;
}
