#include "main.h"

wstring obfuscate(wifstream& codeFile, Config* config) {
	// Считываем текст из файла в строку
	wstringstream wss;
	wss << codeFile.rdbuf();
	wstring codeText(wss.str());

	// Устанавливаем начальное значение для генератора случайных чисел, так как будем им пользоваться
	srand(time(NULL));

	if (config->deleteComments) codeText = deleteComments(codeText);
	if (config->renameFunctions) codeText = renameFunctions(codeText);

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

wstring renameFunctions(wstring codeText) {
	wstring textCopy = codeText; // Копия строки с кодом для поиска в ней

	/*Регулярное выражение для поиска функции.
	Ищет так: должен быть тип, после которого идет пробел, затем название функции, начинающееся с буквы, потом открывающая скобка,
	в которой либо аргументы функции, либо пусто, закрывающая скобка, какое-то количество пробелов и открывающая блок фигурная скобка.*/
	wregex functionRegExp(LR"([\w*]+ ([A-Za-z]\w*)\([\w *&]*\)\s*\{)");

	wsmatch currentMatch; // Текущее совпадение (найденная функция)
	wstring currentFunctionName; // Имя текущей функции
	vector<wstring> functions; // Строки с названиями найденных функций
	wregex functionNameRegExp; // Регулярное выражение для поиска и замены функции в тексте
	wstring newFunctionReplacement; // Строка для замены имени функции

	// Ищем в тексте все объявления функций друг за другом
	while (regex_search(textCopy, currentMatch, functionRegExp)) {
		// Получаем имя функции из общего матча по первому индексу (возвращается то, что в круглых скобках)
		currentFunctionName = currentMatch[1].str();
		// Добавляем в массив имен функций имя текущей функции
		functions.push_back(currentFunctionName);
		// Листаем строку с кодом дальше
		textCopy = currentMatch.suffix();
	}


	for (wstring functionName : functions) {
		// Функцию main не меняем, поскольку иначе программа не будет компилироваться
		if (functionName == L"main") continue;

		/* Создаем для каждой новой функции регулярное выражение для ее поиска в тексте.
		Для этого функция должна находиться вне текста и не быть частью другой функции, то есть перед ней
		не должно стоять букв (могут быть только символы, по типу присваивания или другой скобки, или фигурной скобки - начала блока).
		После имени функции должна быть открывающая скобка (вызов функции), либо запятая (передача функции по указателю в
		качестве параметра), либо точка с запятой - присваивание функции.*/
		functionNameRegExp = wregex(wstring(L"(\\W)") + functionName + wstring(L"([(,;])"));

		/* Генерируем рандомное имя функции, от 7 до 15 символов длиной, добавляем символы, которые попали в регулярное выражение
		* с начала и с конца (чтобы не пропал пробел в начале и скобка после имени функции) */
		newFunctionReplacement = wstring(L"$1") + getRandomString((rand() % 8) + 7) + wstring(L"$2");

		// Заменяем текущее имя на новое имя функции во всем тексте программы
		codeText = regex_replace(codeText, functionNameRegExp, newFunctionReplacement);
	}

	return codeText;
}
