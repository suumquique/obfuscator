#include "main.h"


BOOL getCurrentFlagValue(wstring fullString) {
	// ����� ������, ������������, ��� �������� ����� ����������� ��� true
	wstring endings[] = { L"=true", L"=1" };

	// �������� ��� ������ �� ������� endings
	for (wstring ending : endings) {
		// ������ ������ ���������� � ������ ������ ������ �������
		if (fullString.length() >= ending.length()) {
			if (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending)) return TRUE;
		}
	}

	return FALSE;
}

Config* parseConfigFile(wfstream& configFile) {
	// ������� ��������� ������� � �������� ��� ���� ������ (�� ���� false)
	Config* config = new Config;
	memset(config, 0x00, sizeof(Config));

	wstring currentConfigString; // ������� ������ �������, �� ��������� ����� ������� �������� ����� � ��� ��������
	wstring currentFlag; // ��������� �������� �������� �����
	BOOL currentFlagValue; // �������� �������� ����� (true ��� false)

	// ��������� ������ ���������
	while (getline(configFile, currentConfigString)) {
		// "��������" �� ������ ������ �������� ������ �����
		currentFlag = currentConfigString.substr(0, currentConfigString.find_first_of(L'='));
		// �������� ������� �������� ����� (true ��� false)
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