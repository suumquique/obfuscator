#include "main.hpp"

wstring obfuscate(wfstream& codeFile, Config* config) {
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
	if (config->addTrashVariables) codeText = addTrashVariables(codeText);
	if (config->addTrashLoops) codeText = addTrashLoops(codeText);
	if (config->addTrashFunctions) codeText = addTrashFunctions(codeText);
	if (config->deleteSpaces) codeText = deleteSpaces(codeText);

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
		newFunctionReplacement = wstring(L"$1") + getRandomString(RANDOM_NAME_LENGTH) + wstring(L"$2");

		// Заменяем текущее имя на новое имя функции во всем тексте программы
		codeText = regex_replace(codeText, functionNameRegExp, newFunctionReplacement);
	}

	return codeText;
}

BOOL isValidVariableType(wstring variableType) {
	if (variableType == L"namespace" || variableType == L"return" || variableType == L"else") return FALSE;
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
	wregex variablesRegex(LR"((\w+)[*&]* +([A-Za-z]\w*) *(?:;| +=|,|\[[\d\w]+\]|\) *[;{]| *\{| *\((?:(?:[\w]+)(?:, )?)+\);)+)");

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
			newVariableReplacement = L"$1" + getRandomString(RANDOM_NAME_LENGTH) + L"$2";
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
		currentInsertIndex = findIndexToInsert(codeText, insertElement::COMMENT);
		codeText = codeText.insert(currentInsertIndex, currentComment);
	}

	return codeText;
}

wstring addTrashVariables(wstring codeText) {
	// Считаем, сколько переменных будем добавлять
	size_t numberOfVariables = getLinesNumberInText(codeText) / INSERTION_FREQUENCY_BY_LINES_NUMBER * FREQUENCY_COEFFICIENT;
	wstring currentVariableString; // Строка, содержащая инициализацию текущей переменной
	size_t currentInsertIndex; // Индекс для вставки текущей переменной в программу

	for (size_t i = 0; i < numberOfVariables; i++) {
		// Получаем случайную переменную и добавляем в начале перенос строки
		currentVariableString = LINE_BREAK + getRandomVariableInitializationString();

		// Получаем корректный индекс для вставки переменной и добавляем его в текст программы
		currentInsertIndex = findIndexToInsert(codeText, insertElement::VARIABLE);
		codeText = codeText.insert(currentInsertIndex, currentVariableString);
	}

	return codeText;
}

wstring addTrashLoops(wstring codeText) {
	// Рандомим, сколько бессмысленных циклов будем добавлять
	size_t numberOfCycles = getLinesNumberInText(codeText) / INSERTION_FREQUENCY_BY_LINES_NUMBER * FREQUENCY_COEFFICIENT;
	enum class loopTypes {WHILE, FOR, DO_WHILE}; // Возможные типы циклов
	loopTypes currentLoopType; // Текущий тип цикла
	wstring currentCycleVariableInitialization; // Строка, содержащая инициализацию итерационной переменной цикла
	wstring currentCycleVariableName; // Имя текущей итерационной переменной в цикле
	wstring cycleStartPart; // начало цикла (то, что до тела цикла)
	wstring currentCycleBody = L""; // Тело (средняя часть) текущего цикла, по умолчанию пустая
	wstring cycleEndPart; // Завершающая часть цикла (то, что после тела цикла)
	wstring currentLoopString; // Общая строка всего цикла
	wstring currentRandomLoopLimit; // Текущая граница итерации в создаваемом цикле
	wstring currentLoopStep; // Шаг увеличения в текущем цикле
	wstring comparisonOperator = L" < "; // Оператор сравнения (меньше) для циклов
	wstring positiveAssignmentOperator = L" += "; // Оператор присваивания и сложения одновременно для циклов
	wstring endCycleBodySymbol = L"}"; // Закрывающая скобка тела цикла
	wstring variableInsideCycleInitialization; // рандомная переменная, инициализацию которой добавим внутри цикла
	wstring variableInsideCycleName; // Имя переменной внутри цикла
	size_t currentInsertIndex; // Индекс для вставки текущего цикла в программу


	for (size_t i = 0; i < numberOfCycles; i++) {
		// Получаем строку с инициализацией случайной переменной числовым значением, причем переменная точно не является указателем
		currentCycleVariableInitialization = getRandomVariableInitializationString(TRUE, FALSE);
		// Получаем имя новосозднанной переменной в строковом виде
		currentCycleVariableName = getVariableNameFromInitializationString(currentCycleVariableInitialization);
		// Текущий лимит цикла, до куда будет идти итерация
		currentRandomLoopLimit = to_wstring(rand() % CHAR_MAX);
		// Текущий шаг цикла (от 2 до 10, чтобы цикл был не бесконечным)
		currentLoopStep = to_wstring((rand() % 9) + 2);
		// Создаем переменную для заполнения цикла мусором
		variableInsideCycleInitialization = getRandomVariableInitializationString(TRUE, FALSE);
		variableInsideCycleName = getVariableNameFromInitializationString(variableInsideCycleInitialization);

		// Рандомим тип цикла - for, while или do...while
		currentLoopType = static_cast<loopTypes>(rand() % 3);
		switch (currentLoopType) {
		case loopTypes::FOR:

			// Создаем начало цикла for, допустим "for(size_t i = 0; i < 15; i+=2){"
			cycleStartPart = wstring(L"for (") + currentCycleVariableInitialization + currentCycleVariableName +
				comparisonOperator + currentRandomLoopLimit + wstring(L"; ") + currentCycleVariableName + positiveAssignmentOperator +
				currentLoopStep + wstring(L") {") + LINE_BREAK;
			// Добавляем в конец цикла закрывающую скобку и перенос строки
			cycleEndPart = endCycleBodySymbol + LINE_BREAK;
			break;
		case loopTypes::WHILE:

			/* Создаем начало цикла while. Сначала, до цикла, требуется инициализировать итерационную переменную, потом открыть
			* while(), и внутри скобок прописать условие, а затем открыть блок тела цикла */
			cycleStartPart = currentCycleVariableInitialization + LINE_BREAK + wstring(L"while(") + currentCycleVariableName +
				comparisonOperator + currentRandomLoopLimit + wstring(L") {") + LINE_BREAK + currentCycleVariableName +
				positiveAssignmentOperator + currentLoopStep + wstring(L";") + LINE_BREAK;
			// Добавляем в конец цикла закрывающую скобку и перенос строки
			cycleEndPart = endCycleBodySymbol + LINE_BREAK;
			break;
		case loopTypes::DO_WHILE:

			/* В данном случае в начале требуется до цикла инициализировать итерационную переменную, а затем просто написать do{
			* и в теле цикла прибавить к текущему значению шаг цикла */
			cycleStartPart = currentCycleVariableInitialization + LINE_BREAK + wstring(L"do {") + LINE_BREAK +
				currentCycleVariableName + positiveAssignmentOperator + currentLoopStep + wstring(L";") + LINE_BREAK;
			// Здесь все сложнее в конце: после закрытия тела цикла требуется прописать while() с условием в скобках
			cycleEndPart = endCycleBodySymbol + wstring(L" while(") + currentCycleVariableName +
				comparisonOperator + currentRandomLoopLimit + wstring(L");") + LINE_BREAK;
			break;
		}

		// Случайным образом определяем, будем ли внутрь цикла (в тело цикла) добавлять мусорные переменные
		if (rand() % 2 == 0) {
			// Добавляем инициализацию мусорной переменной
			currentCycleBody += variableInsideCycleInitialization + LINE_BREAK;
			// Добавляем действие с ней и с переменной внутри цикла
			currentCycleBody += variableInsideCycleName + (rand() % 2 ? wstring(L" = ") : positiveAssignmentOperator)
				+ currentCycleVariableName + wstring(L";") +LINE_BREAK;
		}

		// Собираем полную строку цикла
		currentLoopString = LINE_BREAK + cycleStartPart + currentCycleBody + cycleEndPart;
		// Очищаем текущее тело цикла, чтобы в следующий раз оно не добавилось к имеющемуся 
		currentCycleBody = L"";

		// Получаем корректный индекс для вставки цикла и добавляем его в текст программы
		currentInsertIndex = findIndexToInsert(codeText, insertElement::LOOP);
		codeText = codeText.insert(currentInsertIndex, currentLoopString);
	}

	return codeText;
}

wstring addTrashFunctions(wstring codeText) {
	// Рандомим, сколько бессмысленных функций будем добавлять
	size_t numberOfFunctions = getLinesNumberInText(codeText) / INSERTION_FREQUENCY_BY_LINES_NUMBER * FREQUENCY_COEFFICIENT;
	wstring currentFunctionName; // Имя текущей функции
	wstring currentFunctionType; // Тип текущей функции
	wstring currentFunctionPrototype; // Прототип функции
	wstring currentFunctionBody; // "Тело" текущей функции
	wstring currentFunctionString; // Общее строковое представление текущей созданной функции
	wstring currentReturningVariableName; // Имя переменной, которая будет возвращена из текущей функции
	wstring currentReturningVariableInitialization; // Инициализация переменной, которая будет возвращена из текущей функции
	BOOL isCurrentFunctionTypePointer; // Будет ли тип функции указателем
	size_t currentFunctionArgumentNumber; // Количество аргументов у текущей функции
	size_t currentInsertIndex; // Индекс для вставки текущей функции в программу

	for (size_t i = 0; i < numberOfFunctions; i++) {
		currentFunctionName = getRandomString(RANDOM_NAME_LENGTH);
		isCurrentFunctionTypePointer = rand() % 2;
		// Выбираем случайный тип для текущей функциии в половину случаем берем не просто тип, а указатель на него
		currentFunctionType = basicTypes[rand() % basicTypes.size()] + (isCurrentFunctionTypePointer ? POINTER_SYMBOL : L"");
		currentFunctionPrototype = currentFunctionType + wstring(L" ") + currentFunctionName + wstring(L"(");

		// От нуля до четырех аргументов у функции
		currentFunctionArgumentNumber = rand() % 4;
		for (size_t k = 0; k < currentFunctionArgumentNumber; k++) {
			/* Получаем текущий аргумент в виде инициализации переменной без установки значения, с символом в "," конце,
			и добавляем его в прототип. Аргумент может быть указателем */
			currentFunctionPrototype += getRandomVariableInitializationString(FALSE, TRUE, FALSE, L',');
		}
		// Если у функции есть хоть один аргумент, удаляем последнюю добавленную запятую
		if (currentFunctionArgumentNumber) currentFunctionPrototype.erase(currentFunctionPrototype.length() - 1);
		// Добавляем закрывающую скобку к прототипу
		currentFunctionPrototype += L')';

		// Рандомим имя переменной, которую будем возвращать из функции
		currentReturningVariableName = getRandomString(RANDOM_NAME_LENGTH);
		// Инициализируем эту переменную, она должна иметь, совпадающий с типом возвращаемого функцией значения
		currentReturningVariableInitialization = currentFunctionType + wstring(L" ") + currentReturningVariableName +
			wstring(L" = ") + (isCurrentFunctionTypePointer ? wstring(L"0") : to_wstring(rand() % CHAR_MAX)) + wstring(L";");
		// Генерируем "тело" цикла, то, что внутри фигурных скобок
		currentFunctionBody = wstring(L"{") + LINE_BREAK + currentReturningVariableInitialization + LINE_BREAK +
			wstring(L"return ") + currentReturningVariableName + wstring(L";") + LINE_BREAK + wstring(L"}") + LINE_BREAK;

		// Складываем из прототипа (инициализации) и тела функции саму функцию
		currentFunctionString = LINE_BREAK + currentFunctionPrototype + currentFunctionBody;
	
		// Получаем корректный индекс для вставки функции и добавляем его в текст программы
		currentInsertIndex = findIndexToInsert(codeText, insertElement::FUNCTION);
		codeText = codeText.insert(currentInsertIndex, currentFunctionString);
	}

	return codeText;
}

wstring deleteSpaces(wstring codeText) {
	// Если спереди или сзади пробела находится один из данных символов, это значит, что пробел можно удалять
	wstring allowedSymbols = L"=; \n\t\f{}+-,/ <>[]()!";
	// Если один из нижеуказанных символов - удалять пробел ни в коем случае нельзя
	wstring prohibitedSymbols = L"#";
	/* Показывает, имеет ли смысл удалять пробельный символ из данной точки, пользуясь списком разрешенных символов,
	* представленным выше. Например, если пробел между двумя словами, удалять его точно не нужно */
	BOOL mayDeleteSpaceSymbol; 
	// Показывает, попадает ли символ и его правая или левая граница в запрещенный интервал
	BOOL fallsIntoProhibitedInterval;
	size_t codeTextLen = codeText.length();
	

	for (size_t i = 0; i < codeTextLen - 2; i++) {
		// Если символ не пробельный - сразу же проопускам=ем
		if (!iswspace(codeText[i])) continue;

		// Смотрим, есть ли предыдущий или следующий элемент в разрешенных символах и нет ли его в запрещенных
		mayDeleteSpaceSymbol = (allowedSymbols.find(codeText[i - 1]) != wstring::npos || allowedSymbols.find(codeText[i + 1]) != wstring::npos) &&
			prohibitedSymbols.find(codeText[i - 1]) == wstring::npos && prohibitedSymbols.find(codeText[i + 1]) == wstring::npos;
		if (!mayDeleteSpaceSymbol) continue;

		// Проверяем, не находится ли символ или его границы (соседи, +-1 по индексу) в запрещенном интервале
		fallsIntoProhibitedInterval = isInProhibitedInterval(codeText, i) || isInProhibitedInterval(codeText, i + 1)
			|| isInProhibitedInterval(codeText, i - 1);
		if (fallsIntoProhibitedInterval) continue;

		// Удаляем текущий пробел
		codeText = codeText.erase(i, 1);
		/* Отступаем на единицу назад, поскольку если мы этого не сделаем, то перескочим следующий символ, так как
		* текущий удален и по его индексу уже находится следующий */
		i--;
		// Обновляем длину текста
		codeTextLen = codeText.length();
	}

	return codeText;
}