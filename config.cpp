#include "main.h"


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

Config* parseConfigFile(wfstream& configFile) {
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
		if (currentFlag == CONFIG_DELETE_SPACES) {
			config->deleteSpaces = currentFlagValue;
		}
	}

	return config;
}