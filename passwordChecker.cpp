#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>

using namespace std;

// Путь к файлу, где по умолчанию лежит пароль (локально, в той же папке)
#define PATH_TO_FILE_WITH_PASSWORD "password.txt"
// Идентификатор кодировки Windows 1251 с русской локализацией
#define RUS_ENCODING 1251

int main(void) {
	SetConsoleCP(RUS_ENCODING);
	SetConsoleOutputCP(RUS_ENCODING);

	// Пароль, вводимый пользователем, и корректный пароль, который берется из файла
	string password, correctPassword; 
	cout << "Введите пароль: ";
	cin >> password;

	ifstream fileWithPassword(PATH_TO_FILE_WITH_PASSWORD);
	getline(fileWithPassword, correctPassword);

	if (password.compare(correctPassword) != 0) {
		cout << "Введен неверный пароль!" << endl;
		exit(1);
	}

	cout << "Доступ к программе получен." << endl;
}