#include "main.hpp"

int main(void) {
	normalizeEncoding();

	wfstream configFile = openAndCheckFile(L"Введите путь к конфигурационному файлу: ");
	Config* config = parseConfigFile(configFile);

	wfstream codeFile = openAndCheckFile(L"Введите путь к файлу с кодом, который необходимо обфусцировать: ");
	wstring obfuscatedCode = obfuscate(codeFile, config);

	wfstream outputObfuscatedCodeFile = openAndCheckFile(L"Введите путь к файлу, куда будет записан обфусцированный код: ", ios_base::out);
	outputObfuscatedCodeFile << obfuscatedCode;
	wcout << L"Обфусцированный код успешно записан в указанный файл." << endl;
}