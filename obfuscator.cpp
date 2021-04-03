#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <io.h>
#include <fcntl.h>

using namespace std;

// Изменить кодировку консоли и кодировку вывода для работы с русскими символами
void normalizeEncoding() {
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);

	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
}

int main(void) {
	normalizeEncoding();
}