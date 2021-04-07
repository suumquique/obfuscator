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

BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex) {
	wchar_t stringSymbol = L'"'; // И начинающий, и завершающий символ строки
	size_t startPosMultistringComment = codeText.find(START_MULTISTRING_COMMENT_SYMBOLS); // Позиция начала многострочного комментария
	size_t startPosStandardComment = codeText.find(START_STANDARD_COMMENT_SYMBOLS); // Позиция начала обычного комментария
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
			endPosProhibitedInterval = codeText.find(END_MULTRISTRING_COMMENT_SYMBOLS, startPosMultistringComment) + END_MULTRISTRING_COMMENT_SYMBOLS.length();
			// Если индекс для вставки находится между началом и концом мультистрочного комментария, то возвращаем TRUE
			if (insertIndex >= startPosMultistringComment && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// Если наоборот, вырезаем полностью стандартный
		else if (startPosStandardComment < startPosMultistringComment && startPosStandardComment != wstring::npos) {
			// Ищем конец стандартного комментария - он заканчивается при переносе строки
			endPosProhibitedInterval= codeText.find(STANDARD_COMMENT_END_SYMBOL, startPosStandardComment) + 1;
			// Однако, стандартные комментарии можно переносить на следующую строку символом обратного слеша\
			Пока идут обратные слеши, ищем следующий символ переноса строки, так как коммент переносится дальше и дальше
			while (codeText[endPosProhibitedInterval - 1] == '\\') {
				endPosProhibitedInterval = codeText.find(STANDARD_COMMENT_END_SYMBOL, endPosProhibitedInterval) + 1;
			}
			// Если индекс для вставки находится между началом и концом стандартного комментария, то возвращаем TRUE
			if (insertIndex >= startPosStandardComment && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// Обновляем индексы начала разных типов запрещенных интервалов
		startPosMultistringComment = codeText.find(START_MULTISTRING_COMMENT_SYMBOLS, endPosProhibitedInterval);
		startPosStandardComment = codeText.find(START_STANDARD_COMMENT_SYMBOLS, endPosProhibitedInterval);
		startPosString = codeText.find(stringSymbol, endPosProhibitedInterval);
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