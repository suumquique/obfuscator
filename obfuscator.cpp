#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <io.h>
#include <fcntl.h>

using namespace std;

// Изменить кодировку консоли и кодировку вывода для работы с русскими символами
void normalizeEncoding();

// Открывает файл и, если он открылся корректно, возвращает поток для его чтения
wifstream openAndCheckFile(wstring inputPrompt);

// Константа, показывающая, какая строка в конфигурационном файле отвечает за флаг удаления комментов (удалять или оставлять)
#define CONFIG_DELETE_COMMENTS L"deleteComments"
// Строка из конфига, отвечающая за флаг изменения имен переменных
#define CONFIG_RENAME_VARIABLES L"renameVariables"
// Строка из конфига, отвечающая за флаг изменения имен функцией
#define CONFIG_RENAME_FUNCTIONS L"renameFunctions"
// Строка из конфига, отвечающая за флаг изменения расположения функций в файле
#define CONFIG_SHUFFLE_FUNCTIONS L"shuffleFunctions"
// Строка из конфига, отвечающая за флаг добавления бессмысленных переменных в код программы
#define CONFIG_ADD_TRASH_VARIABLES L"trashVariables"
// Строка из конфига, отвечающая за флаг добавления бессмысленных функций в код программы
#define CONFIG_ADD_TRASH_FUNCTIONS L"trashFunctions"
// Строка из конфига, отвечающая за флаг добавления бессмысленных циклов в код программы
#define CONFIG_ADD_TRASH_LOOPS L"trashLoops"
// Строка из конфига, отвечающая за флаг добавления бессмысленных комментариев в код программы
#define CONFIG_ADD_TRASH_COMMENTS L"trashComments"

/*Структура, описывающая конфиг, содержащий параметры обфускации.
Каждая bool-переменная отвечает за соответствующий флаг. Если переменная имеет значение true,
то изменения подходящего типа применяются для обфусцируемого файла, если нет - не применяются.
Флаг deleteSpaces должен быть всегда установлен в true (без него не будут работать остальные)*/
typedef struct _config {
	BOOL deleteSpaces;
	BOOL deleteComments;
	BOOL renameVariables;
	BOOL renameFunctions;
	BOOL shuffleFunctions;
	BOOL addTrashVariables;
	BOOL addTrashFunctions;
	BOOL addTrashLoops;
	BOOL addTrashComments;
} Config;

/*Фунция парсит переданный ей конфигурационный файл построчно, разбивая каждую строку на имя флага и
его значение, принимая его за true или false. Возвращает вышеописанную структуру Config, в которой установлены все имеющиеся
в конфигурационном файле флаги (однако, стоит уточнить: deleteSpace всегда принимает значение true, а те флаги, значение которых
не указано в файле, по умолчанию имеют значение false*/
Config* parseConfigFile(wifstream& configFile);

wstring obfuscate(wifstream& codeFile);

int main(void) {
	normalizeEncoding();

	wifstream configFile = openAndCheckFile(L"Введите путь к конфигурационному файлу: ");
	Config* config = parseConfigFile(configFile);
}

wifstream openAndCheckFile(wstring inputPrompt) {
	wstring filePath;

	// Запрашиваем у пользователя ввод пути к файлу
	wcout << inputPrompt;
	wcin >> filePath;
	wifstream file(filePath);
	if (!file.is_open()) {
		wcout << L"Не удалось открыть файл по указанному пути. Файла не существует, либо доступ к нему запрещен." << endl;
		exit(ERROR_FILE_INVALID);
	}

	return file;
}

void normalizeEncoding() {
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);

	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
}

BOOL getCurrentFlagValue(wstring fullString) {
	// Конец строки, показывающий, что значение флага установлено как true
	wstring endings[] = { L"=true", L"=1" };
	
	// Проходим все строки из массива endings
	for (wstring ending : endings) {
		// Каждую строку сравниваем с концом полной строки конфига
		if (fullString.length() >= ending.length()) {
			if (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending)) return TRUE;
		}
	}

	return FALSE;
}

Config* parseConfigFile(wifstream& configFile) {
	// Создаем экземпляр конфига и заполяем все поля нулями (то есть false)
	Config* config = new Config;
	memset(config, 0x00, sizeof(Config));

	
	wstring currentConfigString; // Текущая строка конфига, из которогой будем парсить название флага и его значение
	wstring currentFlag; // Строковое значение текущего флага
	BOOL currentFlagValue; // Значение текущего флага (true или false)

	// Считываем конфиг построчно
	while (getline(configFile, currentConfigString)) {
		// "Вырезаем" из начала строки название нашего флага
		currentFlag = currentConfigString.substr(0, currentConfigString.find_first_of(L'='));
		// Получаем текущее значение флага (true или false)
		currentFlagValue = getCurrentFlagValue(currentConfigString);

		if (currentFlag == CONFIG_DELETE_COMMENTS) {
			config->deleteComments = currentFlagValue;
		}
		if (currentFlag == CONFIG_RENAME_VARIABLES) {
			config->renameVariables = currentFlagValue;
		}
		if (currentFlag == CONFIG_RENAME_FUNCTIONS) {
			config->renameFunctions = currentFlagValue;
		}
		if (currentFlag == CONFIG_SHUFFLE_FUNCTIONS) {
			config->shuffleFunctions = currentFlagValue;
		}
		if (currentFlag == CONFIG_ADD_TRASH_VARIABLES) {
			config->addTrashVariables = currentFlagValue;
		}
		if (currentFlag == CONFIG_ADD_TRASH_FUNCTIONS) {
			config->addTrashFunctions = currentFlagValue;
		}
		if (currentFlag == CONFIG_ADD_TRASH_LOOPS) {
			config->addTrashLoops = currentFlagValue;
		}
		if (currentFlag == CONFIG_ADD_TRASH_COMMENTS) {
			config->addTrashComments = currentFlagValue;
		}
	}

	// Устанавливаем обязательному флагу значение TRUE
	config->deleteSpaces = TRUE;

	return config;
}

wstring obfuscate(wifstream& codeFile) {

}
