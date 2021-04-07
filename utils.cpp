#include "main.h"

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

BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex) {
	wchar_t stringSymbol = L'"'; // � ����������, � ����������� ������ ������
	size_t startPosMultistringComment = codeText.find(START_MULTISTRING_COMMENT_SYMBOLS); // ������� ������ �������������� �����������
	size_t startPosStandardComment = codeText.find(START_STANDARD_COMMENT_SYMBOLS); // ������� ������ �������� �����������
	size_t startPosString = codeText.find(stringSymbol);
	size_t endPosProhibitedInterval = 0; // �������� ������� ���������� ������������ ��������� (������ ��� �����������)

	// ���� �� �����, ���� � ����� ���� ����������� ���������, � ���� ��� ��������� ������ ������������ �������
	while (startPosMultistringComment != wstring::npos || startPosStandardComment != wstring::npos || startPosString != wstring::npos
		&& (startPosMultistringComment < insertIndex || startPosStandardComment < insertIndex || startPosString < insertIndex)) {

		if (startPosString < startPosMultistringComment && startPosString < startPosStandardComment && startPosString != wstring::npos) {
			// ���� ����� ������, ������ ��������� ������� (����� ������ �������) � ������� ������ ������, ����� �� ����� ���� ������
			endPosProhibitedInterval = codeText.find(stringSymbol, startPosString + 1) + 1;
			/* ���� ���������� ������� ����� ��������� ����� ������ - �������� ����, ��� ������, ��� ������ ����� ������
			����������� � �� �������� ���������� ������. ������� ������, ���� �� ������ �������� ����� ������ */
			while (codeText[endPosProhibitedInterval - 1] == '\\') {
				endPosProhibitedInterval = codeText.find(stringSymbol, endPosProhibitedInterval) + 1;
			}

			// ���� ������ ��� ������� ��������� ����� ������� � ������ ������, �� ���������� TRUE
			if (insertIndex >= startPosString && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// ���� ������������� ����������� ���������� ������ ������������, ������� ��� ������ � �����
		else if (startPosMultistringComment < startPosStandardComment && startPosMultistringComment != wstring::npos) {
			// ���� ����� �������������� �����������
			endPosProhibitedInterval = codeText.find(END_MULTRISTRING_COMMENT_SYMBOLS, startPosMultistringComment) + END_MULTRISTRING_COMMENT_SYMBOLS.length();
			// ���� ������ ��� ������� ��������� ����� ������� � ������ ��������������� �����������, �� ���������� TRUE
			if (insertIndex >= startPosMultistringComment && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// ���� ��������, �������� ��������� �����������
		else if (startPosStandardComment < startPosMultistringComment && startPosStandardComment != wstring::npos) {
			// ���� ����� ������������ ����������� - �� ������������� ��� �������� ������
			endPosProhibitedInterval= codeText.find(STANDARD_COMMENT_END_SYMBOL, startPosStandardComment) + 1;
			// ������, ����������� ����������� ����� ���������� �� ��������� ������ �������� ��������� �����\
			���� ���� �������� �����, ���� ��������� ������ �������� ������, ��� ��� ������� ����������� ������ � ������
			while (codeText[endPosProhibitedInterval - 1] == '\\') {
				endPosProhibitedInterval = codeText.find(STANDARD_COMMENT_END_SYMBOL, endPosProhibitedInterval) + 1;
			}
			// ���� ������ ��� ������� ��������� ����� ������� � ������ ������������ �����������, �� ���������� TRUE
			if (insertIndex >= startPosStandardComment && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// ��������� ������� ������ ������ ����� ����������� ����������
		startPosMultistringComment = codeText.find(START_MULTISTRING_COMMENT_SYMBOLS, endPosProhibitedInterval);
		startPosStandardComment = codeText.find(START_STANDARD_COMMENT_SYMBOLS, endPosProhibitedInterval);
		startPosString = codeText.find(stringSymbol, endPosProhibitedInterval);
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