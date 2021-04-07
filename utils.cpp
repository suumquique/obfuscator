#include "main.h"

// ��������� ������� ���� ����� C (� ��������� �������������) ��� �������� ��������� ���������� ��� �������
vector<wstring> basicTypes{ L"int", L"char", L"long", L"unsigned", L"long long", L"short", L"unsigned short", L"unsigned long", L"unsigned char" };

wifstream openAndCheckFile(wstring inputPrompt) {
	wstring filePath;

	// ����������� � ������������ ���� ���� � �����
	wcout << inputPrompt;
	wcin >> filePath;
	wifstream file(filePath);
	if (!file.is_open()) {
		wcout << L"�� ������� ������� ���� �� ���������� ����. ����� �� ����������, ���� ������ � ���� ��������." << endl;
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
	// ����������� ������ � ����� � ��������� �����, ����� �� ���� ����� ���� ������ � ������� getline
	wistringstream text;
	text.str(codeText);

	wstring line;
	size_t i = 0;
	while (getline(text, line)) i++;

	return i;
}

wstring getRandomVariableInitializationString(BOOL alwaysSetVariableValue, BOOL createPointers) {
	wstring variableString; // ����� ������ ��� ������� ��������� ����������, ���������� � ���� ���, ���, ���������� ������� � �.�.
	wstring variableType; // ��� ������� ����������
	wstring variableName; // ��� ������� ����������
	char variableValue; // �������� ������� ���������� (��� ��� ��� ������� ���������� ����� ����������� � ������� �����)
	BOOL isVariablePointer = createPointers && rand() % 3 == 0; // ��������� ������� ������, ����� �� ����������
	BOOL hasVariableValue = alwaysSetVariableValue || rand() % 3 != 0; // ��������� ������� ������, ����� �� ���������� ����� ��������

	// ���������� ��������� ��� ���������� ��������� �����
	variableName = getRandomString(RANDOM_NAME_LENGTH);
	// �������� ��������� ������� ���
	variableType = basicTypes[rand() % basicTypes.size()];
	// � 1/4 ������� ������� � ��� ������� ���������� ������ ���������, ����� �������� �� ������ ������� ���, � ���������
	if (isVariablePointer) variableType += POINTER_SYMBOL;

	// ���������� ������, �� ���� �� ������: �� ����������� ���������� ��������
	variableString = LINE_BREAK + variableType + L' ' + variableName;
	// � 2/3 ������� �������� ���������� �����-������ ��������
	if (hasVariableValue) {
		/* �������� ��������� �������� ����������, ���� ��� �� �������� ����������, ���� ��� ���������, ������������ 
		��������� ��������� ����� ���� (NULL) */
		variableValue = isVariablePointer ? 0 : rand() % CHAR_MAX;
		// ��������� ��� � ���������� ����� ����������
		variableString += wstring(L" = ") + to_wstring(variableValue);
	}

	// ��������� ���������� ����� ����� �������������
	variableString += L';';

	return variableString;
}

// ���� ����� ������� �������� ���, � �������� ����������� �������� ��������� �������
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
	// ��������� ������� ������ ������������ ������������
	ProhibitedInterval standardComment{ 0, START_STANDARD_COMMENT_SYMBOLS, STANDARD_COMMENT_END_SYMBOL, L'\\' };
	// ��������� ������� ������ ������������� ������������
	ProhibitedInterval multistringComment{ 0, START_MULTISTRING_COMMENT_SYMBOLS, END_MULTRISTRING_COMMENT_SYMBOLS, 0 };
	// ��������� ������� � ��������� ������������� � �����������, ����� ��� #include, #error, #define, #pragma � �.�.
	ProhibitedInterval compilerDirectives{ 0, wstring(L"#"), LINE_BREAK, L'\\' };
	// ��������� ������� � ����������� ������ � ������ ���������
	ProhibitedInterval inlineString{ 0, wstring(L"\""), wstring(L"\""), L'\\' };
	// ��������� ������� � �����, ���������� �������� ��������, ��������, � ��������� ������ ��� � ������ �������
	ProhibitedInterval bracketGroup{ 0, wstring(L"("), wstring(L")"), 0 };

	vector<ProhibitedInterval> prohibitedIntervals { standardComment, multistringComment, inlineString, compilerDirectives, bracketGroup };

	size_t currentSearchStartPos = 0; // ������� �������, �� ������� ����� ������������ ����� � ������ ���������
	size_t currentIntervalStartPos; // ������� ������ �������� ���������
	size_t currentIntervalEndPos; // ������� ����� �������� ���������
	ProhibitedInterval currentInterval; // ������� �������� ��� ������ � ��� ������

	while (currentSearchStartPos < insertIndex) {
		// ������� ������ ��������� ���������� ������� (������) ������� ������������ ���������
		for (size_t i = 0; i < prohibitedIntervals.size(); i++) {
			prohibitedIntervals[i].startPos = codeText.find(prohibitedIntervals[i].start, currentSearchStartPos);
		}

		// �������� � ��� ����������, ������� ���������� ������
		currentInterval = getIntervalWithMinStartPos(prohibitedIntervals);
		currentIntervalStartPos = currentInterval.startPos;
		/* ���� ����� ��������� ������� � ���������, ������� ������ ����, ����� ����� ������, ��� ������, ��� ������� ������ ����� 
		* �� ��������� � ����������� ��������� */
		if (currentInterval.startPos == wstring::npos) return FALSE;
		// ���� ����� �������� ������������ ���������
		currentIntervalEndPos = codeText.find(currentInterval.end, currentIntervalStartPos);
		/*����� �������, ��� ���� ����� ������ �������� ��������� ����� "��������" - �������� ����� �����, �� ��� ������, ���
		* �������� ������������, � ���� ������ ����� ������ */
		while (codeText[currentIntervalEndPos - 1] == currentInterval.intervalBreak && currentInterval.intervalBreak) {
			currentIntervalEndPos = codeText.find(currentInterval.end, currentIntervalEndPos) + currentInterval.end.length();
		}

		// ���� ������� ������ ��������� ������ ������������ ��������� - ���������� TRUE
		if (insertIndex >= currentIntervalStartPos && insertIndex <= currentIntervalEndPos) return TRUE;
		// ��������� ����� ����� �������� � ����� �������� ���������, � �������� ��������� ����� ��������, ����������� ��������
		currentSearchStartPos = currentIntervalEndPos + currentInterval.end.length();
	}

	return FALSE;
}

size_t findIndexToInsert(wstring codeText, insertElement insertElementType) {
	size_t insertIndex = 0; // ������, �� �������� ����� ����� �������� �����-������ �������
	wchar_t endBlockSymbol = L';'; // ������ ����� �����, ����� �������� ����� ��������� �����, ����������� � ����������
	wchar_t startFunctionSymbol = L'{'; // ������ ������ ����������� ����� (�������)
	wchar_t endFunctionSymbol = L'}'; // ������ ����� ����������� ����� (������ �������), ����� �������� ����� ��������� �������
	vector<size_t> availableIndexes; // ������ ��������, �� ������� �������� �������
	size_t openParenthesesNumber = 0; // ���������� �������� �������� ������

	/*��������� �� ���� ����� ��� ������� ���� �������, ���� ����������/�����/�����������, �� ��� �������������� ��������� �����
	��� �������, ��������� ������� ����� ��������� ������ � ���������� ������� ���������, ����� ��� ��� ��������� ����� ���������
	����� ���������� ����� ��������, �� ���� ����� ������� ";". */
	if (insertElementType == insertElement::FUNCTION) {
		/*����� ������ ����� ��� ������� ������ � ���������� ������� ��������� (��� �������), �� ����� ���� �� ������ ��������� � 
		������� �������� ������ - "{" � "}". ���������� ������� - ����� ���������� ����������� ��������� � ����������� �����������,
		�� ����, ��� ����� �������.*/

		for (size_t i = 0; i < codeText.length(); i++) {
			
			/* ����� �������, ��� ���������� ��� ��������� �������, � �� ������ ������, ��� ���� ����������� �������� ������
			* � ������ ���������, ���� ����� �� ���� ����������� */
			if (codeText[i] == startFunctionSymbol && !isInProhibitedInterval(codeText, i)) openParenthesesNumber++;
			if (codeText[i] == endFunctionSymbol && !isInProhibitedInterval(codeText, i)) openParenthesesNumber--;

			/*���� ������� ������ - ����� ����� ��� ����� �������, � ��� ���� ���� ���������� � ���������� �����������, �� ����
			���������� ����������� � ����������� ������ ���������, � ������ ������ �� ���������� � ����������� ���������,
			��������� ��� � ��������� ������� ��� ������� �������*/
			if ((codeText[i] == endBlockSymbol || codeText[i] == endFunctionSymbol) && !isInProhibitedInterval(codeText, i) && openParenthesesNumber == 0) {
				availableIndexes.push_back(i + 1);
			}
		}
	}
	else {
		// ���� ��� ������� ���������� ������
		while ((insertIndex = codeText.find(endBlockSymbol, insertIndex)) != wstring::npos) {
			// �������, ��������� �� ������� ������ � ����������� ���������, ���� �� - ����� ���� ������ ��������� ������
			if (!isInProhibitedInterval(codeText, insertIndex)) {
				// ��������� ������� ������ ���� �� ������� ����� ������� ����� �����, ��������� �������
				availableIndexes.push_back(insertIndex + 1);
			}
			// ����������� ������ �� �������, ����� ��� ��������� ������ ����� ��������� ������ ����� �����
			insertIndex++;
		}
	}

	// ���������� ��������� ������ �� ������ ���, ���� �������� �������
	return availableIndexes[rand() % availableIndexes.size()];
}