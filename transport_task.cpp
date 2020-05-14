#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <conio.h>
#include <iomanip>

using namespace std;
const int NotUsed = system("color F0");

// Т-задача
class t_task{
private:
	int amount_provider;				//Количество поставщиков
	int amount_client;					//Количество потребителей
	vector<double> *supplies;			//Запасы у каждого поставщика
	vector<double> *needs;				//Нужды у каждого потребителя
	vector<vector<double>> *cost;		//cost[i][j] - затраты на доставку единицы продукции от поставщика i к потребителю j
	vector<vector<double>> *base_plan;	//опорный план
	int extra_resource;					//-1 - если был доп. потребитель, 1 - доп. поставщик, 0 - запросы равны нуждам
	double epsilon = 0.01;				//значение epsilon на случай вырожденности
public:
	t_task(string filename);		//Конструктор
	bool print_conditions();		//Печать условия Т-задачи
	void print_base_plan();			//Печать текущего опорного плана
	void find_first_plan();			//Формирование начального опорного плана
	void find_optimal_decision();	//Нахождение оптимального решения транспортной задачи
};

t_task::t_task(string filename){ //Считывание входной информации из файла filename
	ifstream inp(filename);	//Открытие файла с исходными данными
	if (inp.is_open())
	{
		//Считывание количества поставщиков и потребителей
		inp >> amount_provider;
		inp >> amount_client;
		//Создание необходимых структур данных
		supplies = new vector<double>(amount_provider, 0);
		needs = new vector<double>(amount_client, 0);
		cost = new vector<vector<double>>(amount_provider, vector<double>(amount_client, 0));
		//Считывание массива запасов
		for (int i = 0; i < amount_provider; i++)
			inp >> (*supplies)[i];
		//Считывание массива потребностей
		for (int j = 0; j < amount_client; j++)
			inp >> (*needs)[j];
		//Считывание матрицы стоимостей
		for (int i = 0; i < amount_provider; i++)
			for (int j = 0; j < amount_client; j++)
				inp >> (*cost)[i][j];
	}
	else
	{
		cout << "Ошибка открытия файла";
		amount_client = amount_provider = 0;
	}
}

bool t_task::print_conditions(){ //Печать транспортной таблицы задачи
	if (amount_provider == 0 || amount_client == 0)
	{
		cout << "Условия задачи отсутствуют" << endl;
		return false;
	}
	//Печать первой строки таблицы
	cout << "Таблица транспортной задачи:" << endl;
	cout << "Пункты\t\t";
	for (int j = 0; j < amount_client; j++)
		cout << j + 1 << "\t";
	cout << "\tЗапасы" << endl << endl;

	//Печать матрицы стоимостей и массива запасов
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

	//Печать массива потребностей
	cout << "Потребности\t";
	for (int j = 0; j < amount_client; j++)
		cout << (*needs)[j] << "\t";
	cout << endl;
	cout << "________________________________________________________________________________________________" << endl;
	return true;
}

void t_task::find_first_plan(){ //Формирование начального опорного плана
	if (base_plan != nullptr)
		delete base_plan; 

	//Создание нового базисного плана
	base_plan = new vector<vector<double>>(amount_provider, vector<double>(amount_client, 0));

	//Создаем векторы, хранящие нераспределенное количество товара
	vector<double> aa = *supplies;
	vector<double> bb = *needs;

	//Хранит номер первого ненулевого элемента векторов aa и bb;
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

	//Заполнение матрицы опорного плана, пока все aa и bb не равны 0
	bool is_degenerated = false;	// флаг вырожденности плана
	while (not_null_aa < amount_provider && not_null_bb < amount_client){
		//выбираем минимум из доступного объема запасов и нужд
		double s = std::fmin(aa[not_null_aa], bb[not_null_bb]);
		//и записываем этот минимум в матрицу опорного плана
		(*base_plan)[not_null_aa][not_null_bb] = s;
		//Вычитаем из запасов и потребностей распределенную часть ресурсов
		aa[not_null_aa] -= s;
		bb[not_null_bb] -= s;
		//Выбираем следующий элемент в алгоритме
		if (aa[not_null_aa] == 0 && bb[not_null_bb] == 0)		//Если занулились и запасы и потребности, то план вырожденный, добавляем epsilon
		{
			if (not_null_aa < amount_provider - 1 || not_null_bb < amount_client - 1)
			{
				is_degenerated = true;
				if (not_null_aa < amount_provider - 1 && (*base_plan)[not_null_aa + 1][not_null_bb] == 0)	//Если можем добавить epsilon вниз от вырожденного случая
					(*base_plan)[not_null_aa + 1][not_null_bb] = epsilon;
				else
					if (not_null_bb < amount_client - 1 && (*base_plan)[not_null_aa][not_null_bb + 1] == 0)	//Если можем добавить epsilon вправо от вырожденного случая
						(*base_plan)[not_null_aa][not_null_bb + 1] = epsilon;
					else //Иначе добавляем epsilon влево от вырожденного случая
						(*base_plan)[not_null_aa][not_null_bb - 1] = epsilon;
				epsilon /= 10;
			}
			not_null_aa++;
			not_null_bb++;
		}
		else
		{	//Иначе продолжаем обычное построение плана
			if (aa[not_null_aa] == 0)
				not_null_aa++;
			if (bb[not_null_bb] == 0)
				not_null_bb++;
		}
	}
	if (is_degenerated == true)
		cout << "Опорный план вырожденный" << endl;
}

void t_task::print_base_plan(){
	//Вывод опорного плана
	cout << "________________________________________________________________________________________________" << endl;
	cout << "Опорный план имеет вид:" << endl;
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
	//вычисление значения функции цели и излишков
	vector<vector<double>> x_copy = (*base_plan);
	double func_val = 0;
	double extra_val = 0;
	for (int i = 0; i < amount_provider; i++)
		for (int j = 0; j < amount_client; j++)
			if (x_copy[i][j] > 0)
				func_val += (double)round(x_copy[i][j]) * (*cost)[i][j];
	//Вводился ли дополнительный поставщик или потребитель
	if (extra_resource == 1){//доп. поставщик
		for (int j = 0; j < amount_client; j++)
			extra_val += x_copy[amount_provider - 1][j];
		cout << "(Поставщик A" << amount_provider << " является фиктивным)" << endl;
	}
	else if (extra_resource == -1){//доп. потребитель
		for (int i = 0; i < amount_provider; i++)
			extra_val += x_copy[i][amount_client - 1];
		cout << "(Потребитель B" << amount_client << " является фиктивным)" << endl;
	}

	//Вывод значения функции цели
	cout << "Функция цели F = " << func_val << " усл. ед." << endl;
	if (extra_resource == 1)
		cout << "Недостаток = " << extra_val  << " ед. продукции" << endl;
	if (extra_resource == -1)
		cout << "Излишек = " << extra_val << " ед. продукции" << endl;
}

void t_task::find_optimal_decision(){ /*Нахождение оптимального решения*/
	print_conditions();
	//Проверка задачи на сбалансированность
	double sum1 = 0, sum2 = 0;
	for (int i = 0; i < amount_provider; i++)
		sum1 += supplies->at(i);
	for (int j = 0; j < amount_client; j++)
		sum2 += needs->at(j);
	//Если запасы превышают потребности
	//то добавляем фиктивного потребителя
	if (sum1 > sum2)
	{
		cout << "Задача несбалансированная. Ввод дополнительного потребителя" << endl;
		amount_client += 1;
		for (int i = 0; i < amount_provider; i++)
			cost->at(i).push_back(0);
		needs->push_back(sum1 - sum2);
		extra_resource = -1;
	}
	//Если потребности превышают запасы
	//то добавляем фиктивного поставщика
	if (sum1 < sum2)
	{
		cout << "Задача несбалансированная. Ввод дополнительного поставщика" << endl;
		amount_provider += 1;
		cost->push_back(vector<double>(amount_client, 0));
		supplies->push_back(sum2 - sum1);
		extra_resource = 1;
	}
	//Иначе задача сбалансированная
	if (sum1 == sum2)
		extra_resource = 0;

	// Нахождение начального допустимого опорного плана
	find_first_plan();
	//Печать начального опорного плана
	print_base_plan();

	//Метод потенциалов
	int iter_count = 0;
	while (true)
	{
		iter_count++;
		cout << endl << "#######################################################################################" << endl;
		cout << "ИТЕРАЦИЯ № " << iter_count << endl;
		cout << "#######################################################################################" << endl;
		bool degenerate = false;	//Является ли план вырожденным на данной итерациии

		vector<double> *pb = new vector<double>(amount_client, DBL_MAX);		//Потенциалы для поставщиков Ai
		vector<double> *pa = new vector<double>(amount_provider, DBL_MAX);		//Потенциалы для потребителей Bj

		//Вспомогательная матрица потенциалов
		vector<vector<double>> *h = new vector<vector<double>>(amount_provider, vector<double>(amount_client, 0));

		//формирование начальных потенциалов
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
			//Проверка, что все потенциалы были найдены
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
		//формирование матрицы потенциалов
		for (int i = 0; i < amount_provider; i++)
			for (int j = 0; j < amount_client; j++)
			{
				(*h)[i][j] = (*cost)[i][j] + (*pa)[i] - (*pb)[j];
			}


		//Проверка плана на оптимальность и нахождение минимального отрицательного элемента
		bool able_to_enhance = false;
		double start_el = 0;
		pair<int, int> start_index;
		for (int i = 0; i < amount_provider; i++)
			for (int j = 0; j < amount_client; j++)
				if ((*h)[i][j] < 0)		//Есть ли в матрице H отрицательные элементы
				{
					able_to_enhance = true;
					if ((*h)[i][j] < start_el)	//поиск минимального отрицательного элемента
					{
						start_el = (*h)[i][j];
						start_index = make_pair(i, j);
					}
				}
		//Если план нельзя улучшить, то он оптимальный
		if (able_to_enhance == false)
		{
			cout << "Опорный план на предыдущей итерации невозможно улучшить, значит он оптимальный" << endl;
			cout << "________________________________________________________________________________________________" << endl;
			cout << "________________________________________________________________________________________________" << endl;
			break;
		}

		//Иначе строим цикл из заданной вершины
		vector<vector<double>> x_copy = *(base_plan);
		x_copy[start_index.first][start_index.second] = DBL_MAX;
		//Цикл строится методом вычеркивания, а именно:
		//После добавления ненулевого числа в базисное решение у нас получается цикл
		//и притом единственный.
		//1. Вычеркиваем строки, в которых ровно 1 базисная клетка
		//2. Вычеркиваем столбцы, в которых ровно 1 базисная клетка
		//3. Повторяем п.1, пока можно что-то вычеркнуть
		//4. Оставшиеся клетки образуют цикл. Для его поиска начнем из начальной клетки
		//5. Будем брать базисную клетку в той же строке, затем в столбце последней - следующую базисную и тд
		//   пока не вернемся в исходную 
		//(А.И. Сеславин, Е.А. Сеславина "Оптимизация и математические методы принятия решений" Москва - 2011, стр. 114)
		int count = 0;
		bool cutMade = false;
		while (true)
		{
			//Ищем строки, где ровно 1 ненулевой элемент  и зануляем их
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
			//Ищем столбцы, где ровно 1 ненулевой элемент и зануляем их
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

			//Если ни одной строки или столбца не было вырезано, то окончание цикла
			if (cutMade == false)
				break;
		}

		pair<int, int> curr_index = start_index;
		vector< pair<int, int>> cycle;			//Содержит вершины цикла
		cycle.push_back(start_index);

		//Поиск цикла: сначала ищется ненулевой элемент по строке, потом ненулевой по столбцу и так пока не вернемся в исходный
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

		//Ищем минимальный четный элемент (по порядку)
		pair<int, int> min_index = cycle[1];
		double min_el = (*base_plan)[cycle[1].first][cycle[1].second];
		for (unsigned i = 3; i < cycle.size(); i += 2)
			if ((*base_plan)[cycle[i].first][cycle[i].second] < min_el)
			{
				min_el = (*base_plan)[cycle[i].first][cycle[i].second];
				min_index = cycle[i];
			}

		//Прибавление ко всем нечётным элементам цикла и вычитание из всех четных элементов цикла
		for (unsigned i = 0; i < cycle.size(); i += 2)
		{
			(*base_plan)[cycle[i].first][cycle[i].second] += min_el;
			(*base_plan)[cycle[i+1].first][cycle[i+1].second] -= min_el;
		}

		//Проверка плана на вырожденность
		int amount_not_null = 0;
		//Подсчёт ненулевых элементов в опорном плане
		for (int i = 0; i < amount_provider; i++)
			for (int j = 0; j < amount_client; j++)
				if ((*base_plan)[i][j] > 0)
					amount_not_null++;

		//Если not_null > m + n - 1, то план вырожденный
		bool already_found = false;
		//Добавляем epsilon в опорный план
		if (amount_not_null != amount_client + amount_provider - 1)
		{
			degenerate = true;	//отметка о вырожденности плана
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
			cout << "Опорный план на текущей итерации вырожденный" << endl;
		//печать опорного плана
		print_base_plan();
	}

	//Печать оптимального решения транспортной задачи
	cout << endl;
	cout << "ОПТИМАЛЬНОЕ РЕШЕНИЕ:" << endl;
	//округляем до ближайшего целого, чтобы избавиться от епсилон
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
