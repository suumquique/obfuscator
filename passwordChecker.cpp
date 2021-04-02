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
	cout << "������� ������: ";
	cin >> password;

	ifstream fileWithPassword(PATH_TO_FILE_WITH_PASSWORD);
	getline(fileWithPassword, correctPassword);

	if (password.compare(correctPassword) != 0) {
		cout << "������ �������� ������!" << endl;
		exit(1);
	}

	cout << "������ � ��������� �������." << endl;
}