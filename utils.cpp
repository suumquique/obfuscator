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

size_t getLinesNumberInText(wstring codeText) {
	// Преобразуем строку с кодом в строковый поток, чтобы из него можно было читать с помощью getline
	wistringstream text;
	text.str(codeText);

	wstring line;
	size_t i = 0;
	while (getline(text, line)) i++;

	return i;
}

// Ищем среди массива интервал тот, у которого минимальное значение начальной позиции
ProhibitedInterval getIntervalWithMinStartPos(vector<ProhibitedInterval> &intervals) {
	size_t minPos = wstring::npos;
	ProhibitedInterval minInterval{ minPos };

	for (ProhibitedInterval currentInterval : intervals) {
		if (currentInterval.startPos < minPos) {
			minPos = currentInterval.startPos;
			minInterval = currentInterval;
		}
	}

	return minInterval;
}

BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex) {
	// Запрещаем вставку внутрь однострочных комментариев
	ProhibitedInterval standardComment{ 0, START_STANDARD_COMMENT_SYMBOLS, STANDARD_COMMENT_END_SYMBOL, L'\\' };
	// Запрещаем вставку внутрь многострочных комментариев
	ProhibitedInterval multistringComment{ 0, START_MULTISTRING_COMMENT_SYMBOLS, END_MULTRISTRING_COMMENT_SYMBOLS, 0 };
	// Запрещаем вставку в директивы препроцессора и компилятора, такие как #include, #error, #define, #pragma и т.д.
	ProhibitedInterval compilerDirectives{ 0, wstring(L"#"), LINE_BREAK, L'\\' };
	// Запрещаем вставку в константные строки в тексте программы
	ProhibitedInterval inlineString{ 0, wstring(L"\""), wstring(L"\""), L'\\' };
	// Запрещаем вставку в места, окруженные круглыми скобками, например, в заголовки циклов или в вызовы функций
	ProhibitedInterval bracketGroup{ 0, wstring(L"("), wstring(L")"), 0 };

	vector<ProhibitedInterval> prohibitedIntervals { standardComment, multistringComment, inlineString, compilerDirectives, bracketGroup };

	size_t currentSearchStartPos = 0; // Текущая позиция, от которой будем осуществлять поиск в тексте программы
	size_t currentIntervalStartPos; // Позиция начала текущего интервала
	size_t currentIntervalEndPos; // Позиция конца текущего интервала
	ProhibitedInterval currentInterval; // Текущий интервал для работы с его полями

	while (currentSearchStartPos < insertIndex) {
		// Находим первое вхождение стартового символа (строки) каждого запрещенного интервала
		for (size_t i = 0; i < prohibitedIntervals.size(); i++) {
			prohibitedIntervals[i].startPos = codeText.find(prohibitedIntervals[i].start, currentSearchStartPos);
		}

		// Работаем с тем интервалом, который начинается раньше
		currentInterval = getIntervalWithMinStartPos(prohibitedIntervals);
		currentIntervalStartPos = currentInterval.startPos;
		/* Если самый стартовая позиция в интервале, который раньше всех, равна концу текста, это значит, что искомый индекс точно 
		* не находится в запрещенном интервале */
		if (currentInterval.startPos == wstring::npos) return FALSE;
		// Ищем конец текущего запрещенного интервала
		currentIntervalEndPos = codeText.find(currentInterval.end, currentIntervalStartPos);
		/*Стоит помнить, что если перед концом текущего интервала стоит "заглушка" - обратная косая черта, то это значит, что
		* интервал продолжается, и надо искать конец дальше */
		while (codeText[currentIntervalEndPos - 1] == currentInterval.intervalBreak && currentInterval.intervalBreak) {
			currentIntervalEndPos = codeText.find(currentInterval.end, currentIntervalEndPos) + currentInterval.end.length();
		}

		// Если текущий индекс находится внутри запрещенного интервала - возвращаем TRUE
		if (insertIndex >= currentIntervalStartPos && insertIndex <= currentIntervalEndPos) return TRUE;
		// Следующий поиск будем начинать с конца текущего интервала, к которому добавлена длина символов, завершающих интервал
		currentSearchStartPos = currentIntervalEndPos + currentInterval.end.length();
	}

	return FALSE;
}

size_t findIndexToInsert(wstring codeText, insertElement insertElementType) {
	size_t insertIndex = 0; // Индекс, по которому можно будет вставить какой-нибудь элемент
	wchar_t endBlockSymbol = L';'; // Символ конца блока, после которого можно вставлять циклы, комментарии и переменные
	wchar_t startFunctionSymbol = L'{'; // Символ начала глобального блока (функции)
	wchar_t endFunctionSymbol = L'}'; // Символ конца глобального блока (другой функции), после которого можно вставлять функции
	vector<size_t> availableIndexes; // Массив индексов, по которым доступна вставка
	size_t openParenthesesNumber = 0; // Количество открытых фигурных скобок

	/*Поскольку мы ищем место для вставки либо функции, либо переменной/цикла/комментария, то нам соответственно требуется знать
	тип вставки, поскольку функцию можно вставлять только в глобальную область видимости, тогда как все остальное можно вставлять
	после завершения любой операции, то есть после символа ";". */
	if (insertElementType == insertElement::FUNCTION) {
		/*Чтобы искать место для вставки только в глобальной области видимости (вне функций), то будем идти по тексту программы и 
		считать фигурные скобки - "{" и "}". Глобальная область - когда количество открывающих совпадает с количеством закрывающих,
		то есть, все блоки закрыты.*/

		for (size_t i = 0; i < codeText.length(); i++) {
			
			/* Будем считать, что переданная нам программа валидна, и не бывает такого, что есть закрывающая фигурная скобка
			* в тексте программы, если ранее не было открывающей */
			if (codeText[i] == startFunctionSymbol && !isInProhibitedInterval(codeText, i)) openParenthesesNumber++;
			if (codeText[i] == endFunctionSymbol && !isInProhibitedInterval(codeText, i)) openParenthesesNumber--;

			/*Если текущий символ - конец блока или конец функции, и при этом блок расположен в глобальном простанстве, то есть
			количество открывающих и закрывающих скобок совпадает, и данный символ не расположен в запрещенном интервале,
			добавляем его в доступные индексы для вставки функции*/
			if ((codeText[i] == endBlockSymbol || codeText[i] == endFunctionSymbol) && !isInProhibitedInterval(codeText, i) && openParenthesesNumber == 0) {
				availableIndexes.push_back(i + 1);
			}
		}
	}
	else {
		// Ищем все символы завершения блоков
		while ((insertIndex = codeText.find(endBlockSymbol, insertIndex)) != wstring::npos) {
			// Смотрим, находится ли текущий индекс в запрещенном интервале, если да - после него ничего вставлять нельзя
			if (!isInProhibitedInterval(codeText, insertIndex)) {
				// Поскольку вставка должна идти по индексу после символа конца блока, добавляем единицу
				availableIndexes.push_back(insertIndex + 1);
			}
			// Увеличиваем индекс на единицу, чтобы при следующем поиске найти следующий символ конца блока
			insertIndex++;
		}
	}

	// Возвращаем случайный индекс из списка тех, куда доступна вставка
	return availableIndexes[rand() % availableIndexes.size()];
}