#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#define INF 2147483647

using namespace std;

string string_cr(int amount, string str){ //Функция повторяет str amount раз
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

class Detail{			//Структура данных Деталь
public:

	vector<int> times;				//Нормы времени обработки детали
	vector<int> ways;				//Технологический маршрут детали

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

class Tool{			//Структура данных Станок
public:
	int currDetail = -1;			//Номер текущей детали, обрабатываемой на станке
	int workTime;					//Время работы станка
	unsigned int downTime;			//Время простоя станка
	string GantGraphic;				//Строка, содержащая график Ганта
	vector<string> *queue_graphics;	//Графики очередей (в виде строки для вывода)
	vector<int> queue_details;		//Номера деталей, находящиеся в очереди на данный момент
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

class Manufacture{		//Структура данных Завод
private:
	int ruleNumber;					//Номер правила по выбору детали для обработки
	int amount_details;				//Количество деталей
	int amount_tools;				//Количество станков
	int totalTime;					//Общее время работы
	vector<Detail> *details;		//Массив из деталей
	vector<Tool> *tools;			//Массив из станков
	int ** rest_time;				//Пролеживание деталей на каждом из станков
	vector<int> *done_time;			//Время, для каждой детали, обозначающее время технологического цикла
public:
	void showInfo();				//Выдаёт графики Ганта и очередей для всех станков
	void showInfo(string filename);	//Записывает графики Ганта и очередей в файл
	int nextDetail(vector<int> *queue_to_machine);//Выдаёт следующую деталь для обработки в соответствии с правилом
	void doWork();					//Обрабатывание всех деталей в соответствии с их маршрутом
	vector<int> getDownTime();		//Возвращает время простоя всех станках
	int getTotalTime();				//Возвращает общее время работы

	Manufacture(string fileName, int ruleNumber){//Инициализация станков и деталей (данные считываются из файла fileName)
												//создание начальных очередей обработки деталей
		//В файле с именем filename должны хранится данные в следующем порядке:
		//Количество деталей, количество станков, матрица технологических маршрутов, матрица норм времени обработки
		this->ruleNumber = ruleNumber;
		ifstream fileW(fileName);
		int cur_val;

		//Считывание матриц технологического маршрута и временных затрат на обработку деталей из файла
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

		//Формирование начальных очередей на станки
		for (int i = 0; i < amount_details; i++){
			tools->at(details->at(i).ways.at(0) - 1).queue_details.push_back(i);
		}

		//Создание матрицы для хранения времени пролеживания и времени технологического цикла:
		rest_time = new int*[amount_details];
		done_time = new vector<int>(amount_details);
		for (int i = 0; i < amount_details; i++)
			rest_time[i] = new int[amount_tools];
		for (int i = 0; i < amount_details; i++)
		{

			for (int j = 0; j < amount_tools; j++)
				rest_time[i][j] = 0;
		}

		//Первоначальная постановка детали на станок и формирование очередей обработки
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
void Manufacture::showInfo(){ //Показывает графики ганта для каждой детали
	cout << endl;
	cout << "Графики Ганта:" << endl;
	for (int i = 0; i < amount_tools; i++)
		cout << "Станок " << return_letter(i) << ": " << tools->at(i).GantGraphic << endl;
	cout << endl;
	cout << "Графики очередей для станков: " << endl;
	for (int i = 0; i < amount_tools; i++)
	{
		int size = tools->at(i).queue_graphics->size();
		cout << "Станок " << return_letter(i);
		for (int num_queue = 0; num_queue < size; num_queue++)
			if (num_queue > 0)
				cout << "\t: " << tools->at(i).queue_graphics->at(num_queue) << endl;
			else
				cout << ": " << tools->at(i).queue_graphics->at(num_queue) << endl;
	}
	cout << endl << endl;
	cout << "Время пролеживания:" << endl;
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
	out << "Графики Ганта:" << endl;
	for (int i = 0; i < amount_tools; i++)
		out << "Станок " << return_letter(i) << ": " << tools->at(i).GantGraphic << endl;
	out << endl << endl;
	out << "Графики очередей для станков: " << endl;
	for (int i = 0; i < amount_tools; i++)
	{
		int size = tools->at(i).queue_graphics->size();
		out << "Станок " << return_letter(i);
		for (int num_queue = 0; num_queue < size; num_queue++)
			if (num_queue > 0)
				out << "\t: " << tools->at(i).queue_graphics->at(num_queue) << endl;
			else
				out << ": " << tools->at(i).queue_graphics->at(num_queue) << endl;
	}
	out << endl << endl;
	out << "Общее время работы: " << totalTime << endl;
	out << "Время простоя станков:" << endl;
	vector<int> downTime = getDownTime();
	for (int i = 0; i < downTime.size(); i++)
		out << "На станке " << return_letter(i) << " = " << downTime[i] << endl;
	out << endl << endl;
	out << "Время пролеживания:" << endl;
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
int Manufacture::getTotalTime(){ /*Возвращает общее время работы*/
	return totalTime;
}
vector<int> Manufacture::getDownTime(){ /*Возвращает время простоя всех станков*/
	vector<int> downtime;
	for (int i = 0; i < amount_tools; i++)
		downtime.push_back(tools->at(i).downTime);
	return downtime;
}
int Manufacture::nextDetail(vector<int> *queue_to_machine){/*Получение следующей детали в очередь в соответствии с номером правила*/
	int decision;
	int max_num, max_time;
	if (queue_to_machine->size() > 0)
	{
		switch (ruleNumber)
		{
		case 1: 
			//Правило FIFO
			decision = queue_to_machine->front();
			break;
		case 2:
			//Правило по максимуму завершенной части производственного цикла
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
	int st = 1;														//Номер состояния
	while (true){

		//Нахождение минимального времени, оставшегося до особого состояния
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
		//Если минимальное время отсутствует, то все детали уже полностью обработаны
		if (minTime == INF)
			break;

		totalTime += minTime;										 //Добавляем к общему времени работы время состояния

		//Вычисление времени пролеживания деталей и части технологического цикла, которая уже выполнена на очередном состоянии
		for (int i = 0; i < amount_tools; i++)
			for (auto it = tools->at(i).queue_details.begin(); it != tools->at(i).queue_details.end(); it++)
			{
				if (details->at(*it).ways.size() != amount_tools)
					rest_time[*it][i] += minTime;
			}
		
		
		//Добавление информации о состоянии для каждого из графиков Ганта и графиков Очередей
		for (int i = 0; i < amount_tools; i++)
		{
			int number_detail = tools->at(i).currDetail;			 //Номер детали, обрабатываемой на очередном станке

			int sizeOfQueue = tools->at(i).queue_details.size();			 //Количество деталей в очереди на обработку
			int amountQueueGraphics = tools->at(i).queue_graphics->size();   //Наличие графиков для очередей
			//Создание графиков для второго(третьего и тд) эл-та
			//в очереди, если графики для таковых не имеются
			for (int k = 0; k < sizeOfQueue - amountQueueGraphics; k++)
			{
				tools->at(i).queue_graphics->push_back(string_cr(totalTime - minTime, "* "));
			}
			amountQueueGraphics = tools->at(i).queue_graphics->size();
			for (int symb = 0; symb < minTime; symb++)
				if (number_detail != -1)
				{
					tools->at(i).GantGraphic += to_string(number_detail + 1) + " ";
					//Если есть детали в очереди на этот станок
					for (int count_queue = 0; count_queue < amountQueueGraphics; count_queue++)
					{
						if (count_queue >= sizeOfQueue)
							tools->at(i).queue_graphics->at(count_queue) += "* ";
						else
							tools->at(i).queue_graphics->at(count_queue) += to_string(tools->at(i).queue_details.at(count_queue) + 1) + " ";
					}
				}
				else
					//Если никакая деталь на обрабатывается на станке i,
					//То заносим эту информацию в график Ганта
					//и устанавливаем время простоя для i-го станка
				{
					tools->at(i).downTime += 1;
					tools->at(i).GantGraphic += "* ";
					//Если есть детали в очереди на этот станок
					int size = tools->at(i).queue_graphics->size();
					for (int count_queue = 0; count_queue < size; count_queue++)
					{
						tools->at(i).queue_graphics->at(count_queue) += ("* ");
					}
				}
		}
			

		for (int i = 0; i < amount_tools; i++)							 //Для каждого станка, на котором обрабатывается деталь
		{
			if (tools->at(i).currDetail != -1)
			{
				Detail *curr_det = &details->at(tools->at(i).currDetail);
				curr_det->times[0] -= minTime;							 //Отнимаем от времени обработки детали на этом станке
																		 //время текущего состояния
				done_time->at(tools->at(i).currDetail) += minTime;
				if (curr_det->times[0] == 0)							 //Если деталь полностью обработана на этом станке на этом состоянии
				{
					curr_det->delWayTime();								 //То удаляем из её оставшегося маршрута этот станок
					if (curr_det->ways.size() > 0)						 //Если технологический маршрут детали ещё не был завершён
																		 //То заносим эту деталь в очередь обработки
																		 //станка, следующего в её маршруте
					tools->at(curr_det->ways[0] - 1).queue_details.push_back(tools->at(i).currDetail);
					tools->at(i).currDetail = -1;
				}
			}
		}


		for (int i = 0; i < amount_tools; i++)							 //Для каждого станка, на котором закончилась обработка детали
			if (tools->at(i).currDetail == -1)							 
			{
				int curr_det = nextDetail(&tools->at(i).queue_details);		 //Обновляем деталь, которая на нём обрабатывается
				tools->at(i).currDetail = curr_det;
				tools->at(i).queue_details.erase(remove(tools->at(i).queue_details.begin(), tools->at(i).queue_details.end(), curr_det),tools->at(i).queue_details.end());

			}
		st += 1;														 //И переходим к следующему состоянию
	}
	showInfo();														 //Выводим графики Ганта для этого особого состояния
	showInfo("outputRule" + to_string(ruleNumber) + ".txt");
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Rus");
	//Запускаем сначала станки по правилу предпочтения 1
	cout << "Завод с правилом предпочтения 1: " << endl;
	Manufacture fact1("input.txt", 1);
	fact1.doWork();															//Выполняем все работы на станках и выводим графики ганта и очередей
	cout << "Общее время равно: " << fact1.getTotalTime() << endl;				//Общее время обработки
	cout << "Время простоя станков:" << endl;
	vector<int> downTime = fact1.getDownTime();
	for (int i = 0; i < downTime.size(); i++)
		cout << "На станке " << return_letter(i) << " = " << downTime[i] << endl;		//Время простоя каждого из станков

	cout << endl <<  "##############################################################################################" << endl << endl;

	//Затем запускаем станки по правилу предпочтения 2
	cout << "Завод с правилом предпочтения 2: " << endl;
	Manufacture fact2("input.txt", 2);
	fact2.doWork();															//Выполняем все работы на станках и выводим графики ганта и очередей
	cout << "Общее время равно: " << fact2.getTotalTime() << endl;				//Общее время обработки
	cout << "Время простоя станков:" << endl;
	downTime = fact2.getDownTime();
	for (int i = 0; i < downTime.size(); i++)
		cout << "На станке " << return_letter(i) << " = " << downTime[i] << endl;		//Время простоя каждого из станков
	return 0;
}
