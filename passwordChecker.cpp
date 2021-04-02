#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>

using namespace std;

#define PATH_TO_FILE_WITH_PASSWORD "password.txt"
#define RUS_ENCODING 1251

int main(void) {
	SetConsoleCP(RUS_ENCODING);
	SetConsoleOutputCP(RUS_ENCODING);

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