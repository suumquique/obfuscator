#include "main.h"

int main(void) {
	normalizeEncoding();

	wifstream configFile = openAndCheckFile(L"Введите путь к конфигурационному файлу: ");
	Config* config = parseConfigFile(configFile);

	wifstream codeFile = openAndCheckFile(L"Введите путь к файлу с кодом, который необходимо обфусцировать: ");
	wstring obfuscatedCode = obfuscate(codeFile, config);
}