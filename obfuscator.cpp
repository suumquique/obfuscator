#include "main.h"

wstring obfuscate(wifstream& codeFile, Config* config) {
	// ��������� ����� �� ����� � ������
	wstringstream wss;
	wss << codeFile.rdbuf();
	wstring codeText(wss.str());

	// ������������� ��������� �������� ��� ���������� ��������� �����, ��� ��� ����� �� ������������
	srand(time(NULL));

	if (config->deleteComments) codeText = deleteComments(codeText);
	if (config->renameFunctions) codeText = renameFunctions(codeText);

	return codeText;
}

// �������, ��������� ����������� �� ������, ������� ������������ �� ���� ��� C, C++ ��� C#
wstring deleteComments(wstring codeText) {

	wstring startMultistringCommentSymbols = L"/*"; // �������, � ������� ������ ���������� �������������� �����������
	wstring endMultistringCommentSymbols = L"*/"; // �������, �������� ������������� ������������� �����������
	wstring startStandardCommentSymbols = L"//"; // �������, � ������� ������ ���������� ����������� �����������
	wchar_t standardCommentEndSymbol = '\n'; // ����������� ������ ������������ �����������
	size_t startPosMultistringComment = codeText.find(startMultistringCommentSymbols); // ������� ������ �������������� �����������
	size_t startPosStandardComment = codeText.find(startStandardCommentSymbols); // ������� ������ �������� �����������
	size_t endPosMultistringComment; // ������� ����� �������������� �����������
	size_t endPosStandardComment; // ������� ����� �������� �����������

	// ���� �� �����, ���� � ����� ���� �����������
	while (startPosMultistringComment != wstring::npos || startPosStandardComment != wstring::npos) {
		// ���� ������������� ����������� ���������� ������ ������������, �������� ��� ���������
		if (startPosMultistringComment < startPosStandardComment && startPosMultistringComment != wstring::npos) {
			// ���� ����� �������������� �����������
			endPosMultistringComment = codeText.find(endMultistringCommentSymbols) + endMultistringCommentSymbols.length();

			codeText.erase(startPosMultistringComment, endPosMultistringComment - startPosMultistringComment);
		}

		// ���� ��������, �������� ��������� �����������
		else if (startPosStandardComment < startPosMultistringComment && startPosStandardComment != wstring::npos) {
			// ���� ����� ������������ ����������� - �� ������������� ��� �������� ������
			endPosStandardComment = codeText.find(standardCommentEndSymbol, startPosStandardComment);
			// ������, ����������� ����������� ����� ���������� �� ��������� ������ �������� ��������� �����\
			���� ���� �������� �����, ���� ��������� ������ �������� ������, ��� ��� ������� ����������� ������ � ������
			while (codeText[endPosStandardComment - 1] == '\\') {
				endPosStandardComment = codeText.find(standardCommentEndSymbol, endPosStandardComment + 1);
			}

			codeText.erase(startPosStandardComment, endPosStandardComment - startPosStandardComment + 1);
		}

		// ��������� ������� ������ ������ ����� ������������
		startPosMultistringComment = codeText.find(startMultistringCommentSymbols);
		startPosStandardComment = codeText.find(startStandardCommentSymbols);
	}

	return codeText;
}

wstring renameFunctions(wstring codeText) {
	wstring textCopy = codeText; // ����� ������ � ����� ��� ������ � ���

	/*���������� ��������� ��� ������ �������.
	���� ���: ������ ���� ���, ����� �������� ���� ������, ����� �������� �������, ������������ � �����, ����� ����������� ������,
	� ������� ���� ��������� �������, ���� �����, ����������� ������, �����-�� ���������� �������� � ����������� ���� �������� ������.*/
	wregex functionRegExp(LR"([\w*]+ ([A-Za-z]\w*)\([\w *&]*\)\s*\{)");

	wsmatch currentMatch; // ������� ���������� (��������� �������)
	wstring currentFunctionName; // ��� ������� �������
	vector<wstring> functions; // ������ � ���������� ��������� �������
	wregex functionNameRegExp; // ���������� ��������� ��� ������ � ������ ������� � ������
	wstring newFunctionReplacement; // ������ ��� ������ ����� �������

	// ���� � ������ ��� ���������� ������� ���� �� ������
	while (regex_search(textCopy, currentMatch, functionRegExp)) {
		// �������� ��� ������� �� ������ ����� �� ������� ������� (������������ ��, ��� � ������� �������)
		currentFunctionName = currentMatch[1].str();
		// ��������� � ������ ���� ������� ��� ������� �������
		functions.push_back(currentFunctionName);
		// ������� ������ � ����� ������
		textCopy = currentMatch.suffix();
	}


	for (wstring functionName : functions) {
		// ������� main �� ������, ��������� ����� ��������� �� ����� ���������������
		if (functionName == L"main") continue;

		/* ������� ��� ������ ����� ������� ���������� ��������� ��� �� ������ � ������.
		��� ����� ������� ������ ���������� ��� ������ � �� ���� ������ ������ �������, �� ���� ����� ���
		�� ������ ������ ���� (����� ���� ������ �������, �� ���� ������������ ��� ������ ������, ��� �������� ������ - ������ �����).
		����� ����� ������� ������ ���� ����������� ������ (����� �������), ���� ������� (�������� ������� �� ��������� �
		�������� ���������), ���� ����� � ������� - ������������ �������.*/
		functionNameRegExp = wregex(wstring(L"(\\W)") + functionName + wstring(L"([(,;])"));

		/* ���������� ��������� ��� �������, �� 7 �� 15 �������� ������, ��������� �������, ������� ������ � ���������� ���������
		* � ������ � � ����� (����� �� ������ ������ � ������ � ������ ����� ����� �������) */
		newFunctionReplacement = wstring(L"$1") + getRandomString((rand() % 8) + 7) + wstring(L"$2");

		// �������� ������� ��� �� ����� ��� ������� �� ���� ������ ���������
		codeText = regex_replace(codeText, functionNameRegExp, newFunctionReplacement);
	}

	return codeText;
}
