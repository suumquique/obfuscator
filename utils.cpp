#include "main.h"

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

wstring getRandomString(size_t length)
{

	auto randchar = []() -> wchar_t
	{
		const char charset[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"________________________";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};

	wstring str(length, 0);
	generate_n(str.begin(), length, randchar);
	return str;
}

BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex) {
	wstring startMultistringCommentSymbols = L"/*"; // Символы, с которых всегда начинается мультистрочный комментарий
	wstring endMultistringCommentSymbols = L"*/"; // Символы, которыми заканчивается многострочный комментарий
	wstring startStandardCommentSymbols = L"//"; // Символы, с которых всегда начинается стандартный комментарий
	wchar_t standardCommentEndSymbol = '\n'; // Завершающий символ стандартного комментария
	wchar_t stringSymbol = L'"'; // И начинающий, и завершающий символ строки
	size_t startPosMultistringComment = codeText.find(startMultistringCommentSymbols); // Позиция начала многострочного комментария
	size_t startPosStandardComment = codeText.find(startStandardCommentSymbols); // Позиция начала обычного комментария
	size_t startPosString = codeText.find(stringSymbol);
	size_t endPosProhibitedInterval = 0; // Конечная позиция найденного запрещенного интервала (строки или комментария)

	// Идем по циклу, пока в файле есть запрещенные интервалы, и пока она находятся раньше проверяемого индекса
	while (startPosMultistringComment != wstring::npos || startPosStandardComment != wstring::npos || startPosString != wstring::npos
		&& (startPosMultistringComment < insertIndex || startPosStandardComment < insertIndex || startPosString < insertIndex)) {

		if (startPosString < startPosMultistringComment && startPosString < startPosStandardComment && startPosString != wstring::npos) {
			// Ищем конец строки, причем добавляем единицу (длину самого символа) к позиции старта строки, чтобы не найти само начало
			endPosProhibitedInterval = codeText.find(stringSymbol, startPosString + 1) + 1;
			/* Если предыдущий элемент перед элементом конца строки - обратный слеш, это значит, что символ конца строки
			экранирован и не означает завершение строки. Листаем дальше, пока не найдем реальный конец строки */
			while (codeText[endPosProhibitedInterval - 1] == '\\') {
				endPosProhibitedInterval = codeText.find(stringSymbol, endPosProhibitedInterval) + 1;
			}

			// Если индекс для вставки находится между началом и концом строки, то возвращаем TRUE
			if (insertIndex >= startPosString && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// Если многострочный комментарий начинается раньше стандартного, смотрим его начало и конец
		else if (startPosMultistringComment < startPosStandardComment && startPosMultistringComment != wstring::npos) {
			// Ищем конец многострочного комментария
			endPosProhibitedInterval = codeText.find(endMultistringCommentSymbols, startPosMultistringComment) + endMultistringCommentSymbols.length();
			// Если индекс для вставки находится между началом и концом мультистрочного комментария, то возвращаем TRUE
			if (insertIndex >= startPosMultistringComment && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// Если наоборот, вырезаем полностью стандартный
		else if (startPosStandardComment < startPosMultistringComment && startPosStandardComment != wstring::npos) {
			// Ищем конец стандартного комментария - он заканчивается при переносе строки
			endPosProhibitedInterval= codeText.find(standardCommentEndSymbol, startPosStandardComment) + 1;
			// Однако, стандартные комментарии можно переносить на следующую строку символом обратного слеша\
			Пока идут обратные слеши, ищем следующий символ переноса строки, так как коммент переносится дальше и дальше
			while (codeText[endPosProhibitedInterval - 1] == '\\') {
				endPosProhibitedInterval = codeText.find(standardCommentEndSymbol, endPosProhibitedInterval) + 1;
			}
			// Если индекс для вставки находится между началом и концом стандартного комментария, то возвращаем TRUE
			if (insertIndex >= startPosStandardComment && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// Обновляем индексы начала разных типов запрещенных интервалов
		startPosMultistringComment = codeText.find(startMultistringCommentSymbols, endPosProhibitedInterval);
		startPosStandardComment = codeText.find(startStandardCommentSymbols, endPosProhibitedInterval);
		startPosString = codeText.find(stringSymbol, endPosProhibitedInterval);
	}

	return FALSE;
}

size_t findIndexToInsert(wstring codeText, insertElement insertElementType) {
	/*Поскольку мы ищем место для вставки либо функции, либо переменной/цикла/комментария, то нам соответственно требуется знать
	тип вставки, поскольку функцию можно вставлять только в глобальную область видимости, тогда как все остальное можно вставлять
	после завершения любой операции, то есть после символа ";". */

	return 0;
}