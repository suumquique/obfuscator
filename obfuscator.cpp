#include "main.h"

wstring obfuscate(wifstream& codeFile, Config* config) {
	// ��������� ����� �� ����� � ������
	wstringstream wss;
	wss << codeFile.rdbuf();
	wstring codeText(wss.str());

	if (config->deleteComments) codeText = deleteComments(codeText);

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