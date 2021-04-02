#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include <bitset>

using namespace std;

// ���� � �����, ��� �� ��������� ����� ������ (��������, � ��� �� �����)
#define PATH_TO_FILE_WITH_PASSWORD "encryptedPassword.txt"
// ������������� ��������� Windows 1251 � ������� ������������
#define RUS_ENCODING 1251
// ��������� ����� ��� ���������� ������ � ������� XOR
#define XOR_KEY 521

// ������� ������ � ������� XOR-����������
string encryptString(string s) {
	for (size_t i = 0; i < s.length(); i++) {
		s[i] ^= XOR_KEY;
	}

	return s;
}

int main(void) {
	SetConsoleCP(RUS_ENCODING);
	SetConsoleOutputCP(RUS_ENCODING);

	// ������, �������� �������������, ������������� ���������������� ������ � ���������� ������ (�������������), ������� ������� �� �����
	string password, encryptedPassword, encryptedCorrectPassword;
	cout << "������� ������: ";
	cin >> password;
	encryptedPassword = encryptString(password);
	

	ifstream fileWithPassword(PATH_TO_FILE_WITH_PASSWORD);
	getline(fileWithPassword, encryptedCorrectPassword);

	if (encryptedPassword.compare(encryptedCorrectPassword) != 0) {
		cout << "������ �������� ������!" << endl;
		exit(1);
	}

	cout << "������ � ��������� �������." << endl;
}