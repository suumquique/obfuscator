#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include <bitset>

using namespace std;

// Путь к файлу, где по умолчанию лежит пароль (локально, в той же папке)
#define PATH_TO_FILE_WITH_PASSWORD "encryptedPassword.txt"
// Идентификатор кодировки Windows 1251 с русской локализацией
#define RUS_ENCODING 1251
// Случайное число для шифрования пароля с помощью XOR
#define XOR_KEY 521

// Шифруем строку с помощью XOR-шифрования
string encryptString(string s) {
	for (size_t i = 0; i < s.length(); i++) {
		s[i] ^= XOR_KEY;
	}

	return s;
}

int main(void) {
	SetConsoleCP(RUS_ENCODING);
	SetConsoleOutputCP(RUS_ENCODING);

	// Пароль, вводимый пользователем, зашифрованный пользовательский пароль и корректный пароль (зашифрованный), который берется из файла
	string password, encryptedPassword, encryptedCorrectPassword;
	cout << "Введите пароль: ";
	cin >> password;
	encryptedPassword = encryptString(password);
	

	ifstream fileWithPassword(PATH_TO_FILE_WITH_PASSWORD);
	getline(fileWithPassword, encryptedCorrectPassword);

	if (encryptedPassword.compare(encryptedCorrectPassword) != 0) {
		cout << "Введен неверный пароль!" << endl;
		exit(1);
	}

	cout << "Доступ к программе получен." << endl;
}