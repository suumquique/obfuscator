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
	if (config->renameVariables) codeText = renameVariables(codeText);
	if (config->addTrashComments) codeText = addTrashComments(codeText);

	wofstream test("test.cpp");
	test << codeText;

	return codeText;
}

// Функция, удаляющая комментарии из строки, которая представляет из себя код C, C++ или C#
wstring deleteComments(wstring codeText) {
	size_t startPosMultistringComment = codeText.find(START_MULTISTRING_COMMENT_SYMBOLS); // Позиция начала многострочного комментария
	size_t startPosStandardComment = codeText.find(START_STANDARD_COMMENT_SYMBOLS); // Позиция начала обычного комментария
	size_t endPosMultistringComment; // Позиция конца многострочного комментация
	size_t endPosStandardComment; // Позиция конца обычного комментария

	// Идем по циклу, пока в файле есть комментарии
	while (startPosMultistringComment != wstring::npos || startPosStandardComment != wstring::npos) {
		// Если многострочный комментарий начинается раньше стандартного, вырезаем его полностью
		if (startPosMultistringComment < startPosStandardComment && startPosMultistringComment != wstring::npos) {
			// Ищем конец многострочного комментария
			endPosMultistringComment = codeText.find(END_MULTRISTRING_COMMENT_SYMBOLS) + END_MULTRISTRING_COMMENT_SYMBOLS.length();

			codeText.erase(startPosMultistringComment, endPosMultistringComment - startPosMultistringComment);
		}

		// Если наоборот, вырезаем полностью стандартный
		else if (startPosStandardComment < startPosMultistringComment && startPosStandardComment != wstring::npos) {
			// Ищем конец стандартного комментария - он заканчивается при переносе строки
			endPosStandardComment = codeText.find(STANDARD_COMMENT_END_SYMBOL, startPosStandardComment);
			// Однако, стандартные комментарии можно переносить на следующую строку символом обратного слеша\
			Пока идут обратные слеши, ищем следующий символ переноса строки, так как коммент переносится дальше и дальше
			while (codeText[endPosStandardComment - 1] == '\\') {
				endPosStandardComment = codeText.find(STANDARD_COMMENT_END_SYMBOL, endPosStandardComment + 1);
			}

			codeText.erase(startPosStandardComment, endPosStandardComment - startPosStandardComment + 1);
		}

		// Обновляем индексы начала разных типов комментариев
		startPosMultistringComment = codeText.find(START_MULTISTRING_COMMENT_SYMBOLS);
		startPosStandardComment = codeText.find(START_STANDARD_COMMENT_SYMBOLS);
	}

	return codeText;
}

wstring renameFunctions(wstring codeText) {
	wstring textCopy = codeText; // Копия строки с кодом для поиска в ней

	/*Регулярное выражение для поиска функции.
	Ищет так: должен быть тип, после которого идет пробел (или несколько), затем название функции, начинающееся с буквы, потом открывающая скобка,
	в которой либо аргументы функции, либо пусто, закрывающая скобка, какое-то количество пробелов и открывающая блок фигурная скобка.*/
	wregex functionRegExp(LR"([\w*]+ +([A-Za-z]\w*)\([\w *&]*\)\s*\{)");

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

BOOL isValidVariableType(wstring variableType) {
	if (variableType == L"namespace" || variableType == L"return") return FALSE;
	return TRUE;
}

wstring renameVariables(wstring codeText) {
	// Копия текста кода для работы с ним (поиск, меняющий строку)
	wstring textCopy = codeText;

	/*Регулярное выражение для поиска объявления переменных в коде программы.
	Перед переменной сначала указан тип, затем некое количество пробелов (от одного и более), затем само имя переменной,
	потом либо конец объявления, то есть символ ";", либо присваивание чего-нибудь ей, то есть снова некое количество пробелов и
	символ присванивания - "=", либо переменная объявляется как аргумент функции, в таком случае там скобка.
	Кроме того, может быть объявление нескольких переменных поряд, что тоже нужно обработать, то есть там в конце может быть запятая.
	Так же, требуется не забыть о классах и структурах, которые могут использовать uniform-инициализацию, для этого добавляем "{"*/
	wregex variablesRegex(LR"((\w+)[*&]* +([A-Za-z]\w*) *(?:;| +=|,|\) *[;{]| *\{| *\((?:(?:[\w]+)(?:, )?)+\);)+)");

	wsmatch currentMatch; // Текущее совпадение - найденное объявление переменной
	wstring currentVariableName; // Текущее имя
	wstring currentVariableType; // Тип текущей переменной для проверки корректности

	wregex variableNameRegExp; // Регулярное выражение для поиска переменной по имени в тексте программы и его замене
	wstring newVariableReplacement; // Новое имя, на которое будет заменена переменная

	while (regex_search(textCopy, currentMatch, variablesRegex)) {
		currentVariableType = currentMatch[1].str();
		currentVariableName = currentMatch[2].str();

		// Может быть такое, что нашло не переменную (а, например, использование поля имен). Такое исключаем
		if (isValidVariableType(currentVariableType)) {
			/* Создаем регулярное выражение для поиска каждого упоминания имени переменной во всем коде программы.
			Искомое значение не должно находиться в строке и не должно быть частью другого названия (функции и переменной),
			поэтому смотрим, чтобы перед названием была не буква и не цифра, а также не символ новой строк - "*/
			variableNameRegExp = wregex(L"([^\"\\w])" + currentVariableName + L"([^\"\\w])");

			// Указываем, на что будем заменять: случайная строка и найденные символы справа и слева, которые мы удалили
			newVariableReplacement = L"$1" + getRandomString((rand() % 8) + 7) + L"$2";
			// Заменяем во всем тексте по вышеуказанному регулярному выражению
			codeText = regex_replace(codeText, variableNameRegExp, newVariableReplacement);
		}

		textCopy = currentMatch.suffix();
	}
	
	return codeText;
}

wstring addTrashComments(wstring codeText) {
	// Считаем, сколько комментариев будем добавлять
	size_t numberOfComments = getLinesNumberInText(codeText) / INSERTION_FREQUENCY_BY_LINES_NUMBER * FREQUENCY_COEFFICIENT;
	wstring currentComment; // Текущий комментарий для добавления, который будем генерировать каждый раз
	commentType currentCommentType; // Тип текущего комментария - многострочный или однострочный (стандартный)
	size_t currentInsertIndex; // Индекс в тексте для добавления текущего комментария

	wcout << numberOfComments << endl;

	for (size_t i = 0; i < numberOfComments; i++) {
		// Получаем случайную строку из бессмысленных символов
		currentComment = getRandomString(MAX_COMMENT_LENGTH);

		// Добавляем в комментарий пробелы на случайные позиции
		for (size_t k = 0; k < currentComment.length(); k++) {
			if (WHITESPACE_FREQUENCY_IN_COMMENTS == 1) currentComment[k] = L' ';
		}

		// Случайным образом выбираем тип добавляемого комментария (стандартный или многострочный)
		currentCommentType = rand() % 2 ? commentType::STANDARD : commentType::MULTISTRING;
		if (currentCommentType == commentType::STANDARD) {
			currentComment = START_STANDARD_COMMENT_SYMBOLS + currentComment + STANDARD_COMMENT_END_SYMBOL;
		}
		else currentComment = START_MULTISTRING_COMMENT_SYMBOLS + currentComment + END_MULTRISTRING_COMMENT_SYMBOLS;

		// Получаем корректный индекс для вставки комментария и добавляем его в текст программы
		size_t currentInsertIndex = findIndexToInsert(codeText, insertElement::COMMENT);
		codeText = codeText.insert(currentInsertIndex, currentComment);
	}

	return codeText;
}