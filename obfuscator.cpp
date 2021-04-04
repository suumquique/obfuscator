#include "main.h"

wstring obfuscate(wifstream& codeFile, Config* config) {
	// Считываем текст из файла в строку
	wstringstream wss;
	wss << codeFile.rdbuf();
	wstring codeText(wss.str());

	if (config->deleteComments) codeText = deleteComments(codeText);

	return codeText;
}

// Функция, удаляющая комментарии из строки, которая представляет из себя код C, C++ или C#
wstring deleteComments(wstring codeText) {

	wstring startMultistringCommentSymbols = L"/*"; // Символы, с которых всегда начинается мультистрочный комментарий
	wstring endMultistringCommentSymbols = L"*/"; // Символы, которыми заканчивается многострочный комментарий
	wstring startStandardCommentSymbols = L"//"; // Символы, с которых всегда начинается стандартный комментарий
	wchar_t standardCommentEndSymbol = '\n'; // Завершающий символ стандартного комментария
	size_t startPosMultistringComment = codeText.find(startMultistringCommentSymbols); // Позиция начала многострочного комментария
	size_t startPosStandardComment = codeText.find(startStandardCommentSymbols); // Позиция начала обычного комментария
	size_t endPosMultistringComment; // Позиция конца многострочного комментация
	size_t endPosStandardComment; // Позиция конца обычного комментария

	// Идем по циклу, пока в файле есть комментарии
	while (startPosMultistringComment != wstring::npos || startPosStandardComment != wstring::npos) {
		// Если многострочный комментарий начинается раньше стандартного, вырезаем его полностью
		if (startPosMultistringComment < startPosStandardComment && startPosMultistringComment != wstring::npos) {
			// Ищем конец многострочного комментария
			endPosMultistringComment = codeText.find(endMultistringCommentSymbols) + endMultistringCommentSymbols.length();

			codeText.erase(startPosMultistringComment, endPosMultistringComment - startPosMultistringComment);
		}

		// Если наоборот, вырезаем полностью стандартный
		else if (startPosStandardComment < startPosMultistringComment && startPosStandardComment != wstring::npos) {
			// Ищем конец стандартного комментария - он заканчивается при переносе строки
			endPosStandardComment = codeText.find(standardCommentEndSymbol, startPosStandardComment);
			// Однако, стандартные комментарии можно переносить на следующую строку символом обратного слеша\
			Пока идут обратные слеши, ищем следующий символ переноса строки, так как коммент переносится дальше и дальше
			while (codeText[endPosStandardComment - 1] == '\\') {
				endPosStandardComment = codeText.find(standardCommentEndSymbol, endPosStandardComment + 1);
			}

			codeText.erase(startPosStandardComment, endPosStandardComment - startPosStandardComment + 1);
		}

		// Обновляем индексы начала разных типов комментариев
		startPosMultistringComment = codeText.find(startMultistringCommentSymbols);
		startPosStandardComment = codeText.find(startStandardCommentSymbols);
	}

	return codeText;
}