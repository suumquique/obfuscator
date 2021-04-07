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

BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex) {
	wstring startMultistringCommentSymbols = L"/*"; // �������, � ������� ������ ���������� �������������� �����������
	wstring endMultistringCommentSymbols = L"*/"; // �������, �������� ������������� ������������� �����������
	wstring startStandardCommentSymbols = L"//"; // �������, � ������� ������ ���������� ����������� �����������
	wchar_t standardCommentEndSymbol = '\n'; // ����������� ������ ������������ �����������
	wchar_t stringSymbol = L'"'; // � ����������, � ����������� ������ ������
	size_t startPosMultistringComment = codeText.find(startMultistringCommentSymbols); // ������� ������ �������������� �����������
	size_t startPosStandardComment = codeText.find(startStandardCommentSymbols); // ������� ������ �������� �����������
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
			endPosProhibitedInterval = codeText.find(endMultistringCommentSymbols, startPosMultistringComment) + endMultistringCommentSymbols.length();
			// ���� ������ ��� ������� ��������� ����� ������� � ������ ��������������� �����������, �� ���������� TRUE
			if (insertIndex >= startPosMultistringComment && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// ���� ��������, �������� ��������� �����������
		else if (startPosStandardComment < startPosMultistringComment && startPosStandardComment != wstring::npos) {
			// ���� ����� ������������ ����������� - �� ������������� ��� �������� ������
			endPosProhibitedInterval= codeText.find(standardCommentEndSymbol, startPosStandardComment) + 1;
			// ������, ����������� ����������� ����� ���������� �� ��������� ������ �������� ��������� �����\
			���� ���� �������� �����, ���� ��������� ������ �������� ������, ��� ��� ������� ����������� ������ � ������
			while (codeText[endPosProhibitedInterval - 1] == '\\') {
				endPosProhibitedInterval = codeText.find(standardCommentEndSymbol, endPosProhibitedInterval) + 1;
			}
			// ���� ������ ��� ������� ��������� ����� ������� � ������ ������������ �����������, �� ���������� TRUE
			if (insertIndex >= startPosStandardComment && insertIndex <= endPosProhibitedInterval) return TRUE;
		}

		// ��������� ������� ������ ������ ����� ����������� ����������
		startPosMultistringComment = codeText.find(startMultistringCommentSymbols, endPosProhibitedInterval);
		startPosStandardComment = codeText.find(startStandardCommentSymbols, endPosProhibitedInterval);
		startPosString = codeText.find(stringSymbol, endPosProhibitedInterval);
	}

	return FALSE;
}

size_t findIndexToInsert(wstring codeText, insertElement insertElementType) {
	/*��������� �� ���� ����� ��� ������� ���� �������, ���� ����������/�����/�����������, �� ��� �������������� ��������� �����
	��� �������, ��������� ������� ����� ��������� ������ � ���������� ������� ���������, ����� ��� ��� ��������� ����� ���������
	����� ���������� ����� ��������, �� ���� ����� ������� ";". */

	return 0;
}