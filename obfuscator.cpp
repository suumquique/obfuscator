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

wifstream openAndCheckFile(wstring inputPrompt) {
	wstring filePath;

	wcout << inputPrompt;
	wcin >> filePath;
	wifstream file(filePath);
	if (!file.is_open()) {
		wcout << L"Не удалось открыть файл по указанному пути. Файла не существует, либо доступ к нему запрещен." << endl;
		exit(ERROR_FILE_INVALID);
	}

	return file;
}

int main(void) {
	normalizeEncoding();

	wifstream configFile = openAndCheckFile(L"Введите путь к конфигурационному файлу: ");
}