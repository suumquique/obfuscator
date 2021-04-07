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
	if (config->renameVariables) codeText = renameVariables(codeText);
	if (config->addTrashComments) codeText = addTrashComments(codeText);

	wofstream test("test.cpp");
	test << codeText;

	return codeText;
}

// �������, ��������� ����������� �� ������, ������� ������������ �� ���� ��� C, C++ ��� C#
wstring deleteComments(wstring codeText) {
	size_t startPosMultistringComment = codeText.find(START_MULTISTRING_COMMENT_SYMBOLS); // ������� ������ �������������� �����������
	size_t startPosStandardComment = codeText.find(START_STANDARD_COMMENT_SYMBOLS); // ������� ������ �������� �����������
	size_t endPosMultistringComment; // ������� ����� �������������� �����������
	size_t endPosStandardComment; // ������� ����� �������� �����������

	// ���� �� �����, ���� � ����� ���� �����������
	while (startPosMultistringComment != wstring::npos || startPosStandardComment != wstring::npos) {
		// ���� ������������� ����������� ���������� ������ ������������, �������� ��� ���������
		if (startPosMultistringComment < startPosStandardComment && startPosMultistringComment != wstring::npos) {
			// ���� ����� �������������� �����������
			endPosMultistringComment = codeText.find(END_MULTRISTRING_COMMENT_SYMBOLS) + END_MULTRISTRING_COMMENT_SYMBOLS.length();

			codeText.erase(startPosMultistringComment, endPosMultistringComment - startPosMultistringComment);
		}

		// ���� ��������, �������� ��������� �����������
		else if (startPosStandardComment < startPosMultistringComment && startPosStandardComment != wstring::npos) {
			// ���� ����� ������������ ����������� - �� ������������� ��� �������� ������
			endPosStandardComment = codeText.find(STANDARD_COMMENT_END_SYMBOL, startPosStandardComment);
			// ������, ����������� ����������� ����� ���������� �� ��������� ������ �������� ��������� �����\
			���� ���� �������� �����, ���� ��������� ������ �������� ������, ��� ��� ������� ����������� ������ � ������
			while (codeText[endPosStandardComment - 1] == '\\') {
				endPosStandardComment = codeText.find(STANDARD_COMMENT_END_SYMBOL, endPosStandardComment + 1);
			}

			codeText.erase(startPosStandardComment, endPosStandardComment - startPosStandardComment + 1);
		}

		// ��������� ������� ������ ������ ����� ������������
		startPosMultistringComment = codeText.find(START_MULTISTRING_COMMENT_SYMBOLS);
		startPosStandardComment = codeText.find(START_STANDARD_COMMENT_SYMBOLS);
	}

	return codeText;
}

wstring renameFunctions(wstring codeText) {
	wstring textCopy = codeText; // ����� ������ � ����� ��� ������ � ���

	/*���������� ��������� ��� ������ �������.
	���� ���: ������ ���� ���, ����� �������� ���� ������ (��� ���������), ����� �������� �������, ������������ � �����, ����� ����������� ������,
	� ������� ���� ��������� �������, ���� �����, ����������� ������, �����-�� ���������� �������� � ����������� ���� �������� ������.*/
	wregex functionRegExp(LR"([\w*]+ +([A-Za-z]\w*)\([\w *&]*\)\s*\{)");

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

BOOL isValidVariableType(wstring variableType) {
	if (variableType == L"namespace" || variableType == L"return") return FALSE;
	return TRUE;
}

wstring renameVariables(wstring codeText) {
	// ����� ������ ���� ��� ������ � ��� (�����, �������� ������)
	wstring textCopy = codeText;

	/*���������� ��������� ��� ������ ���������� ���������� � ���� ���������.
	����� ���������� ������� ������ ���, ����� ����� ���������� �������� (�� ������ � �����), ����� ���� ��� ����������,
	����� ���� ����� ����������, �� ���� ������ ";", ���� ������������ ����-������ ��, �� ���� ����� ����� ���������� �������� �
	������ ������������� - "=", ���� ���������� ����������� ��� �������� �������, � ����� ������ ��� ������.
	����� ����, ����� ���� ���������� ���������� ���������� �����, ��� ���� ����� ����������, �� ���� ��� � ����� ����� ���� �������.
	��� ��, ��������� �� ������ � ������� � ����������, ������� ����� ������������ uniform-�������������, ��� ����� ��������� "{"*/
	wregex variablesRegex(LR"((\w+)[*&]* +([A-Za-z]\w*) *(?:;| +=|,|\) *[;{]| *\{| *\((?:(?:[\w]+)(?:, )?)+\);)+)");

	wsmatch currentMatch; // ������� ���������� - ��������� ���������� ����������
	wstring currentVariableName; // ������� ���
	wstring currentVariableType; // ��� ������� ���������� ��� �������� ������������

	wregex variableNameRegExp; // ���������� ��������� ��� ������ ���������� �� ����� � ������ ��������� � ��� ������
	wstring newVariableReplacement; // ����� ���, �� ������� ����� �������� ����������

	while (regex_search(textCopy, currentMatch, variablesRegex)) {
		currentVariableType = currentMatch[1].str();
		currentVariableName = currentMatch[2].str();

		// ����� ���� �����, ��� ����� �� ���������� (�, ��������, ������������� ���� ����). ����� ���������
		if (isValidVariableType(currentVariableType)) {
			/* ������� ���������� ��������� ��� ������ ������� ���������� ����� ���������� �� ���� ���� ���������.
			������� �������� �� ������ ���������� � ������ � �� ������ ���� ������ ������� �������� (������� � ����������),
			������� �������, ����� ����� ��������� ���� �� ����� � �� �����, � ����� �� ������ ����� ����� - "*/
			variableNameRegExp = wregex(L"([^\"\\w])" + currentVariableName + L"([^\"\\w])");

			// ���������, �� ��� ����� ��������: ��������� ������ � ��������� ������� ������ � �����, ������� �� �������
			newVariableReplacement = L"$1" + getRandomString((rand() % 8) + 7) + L"$2";
			// �������� �� ���� ������ �� �������������� ����������� ���������
			codeText = regex_replace(codeText, variableNameRegExp, newVariableReplacement);
		}

		textCopy = currentMatch.suffix();
	}
	
	return codeText;
}

wstring addTrashComments(wstring codeText) {
	// �������, ������� ������������ ����� ���������
	size_t numberOfComments = getLinesNumberInText(codeText) / INSERTION_FREQUENCY_BY_LINES_NUMBER * FREQUENCY_COEFFICIENT;
	wstring currentComment; // ������� ����������� ��� ����������, ������� ����� ������������ ������ ���
	commentType currentCommentType; // ��� �������� ����������� - ������������� ��� ������������ (�����������)
	size_t currentInsertIndex; // ������ � ������ ��� ���������� �������� �����������

	wcout << numberOfComments << endl;

	for (size_t i = 0; i < numberOfComments; i++) {
		// �������� ��������� ������ �� ������������� ��������
		currentComment = getRandomString(MAX_COMMENT_LENGTH);

		// ��������� � ����������� ������� �� ��������� �������
		for (size_t k = 0; k < currentComment.length(); k++) {
			if (WHITESPACE_FREQUENCY_IN_COMMENTS == 1) currentComment[k] = L' ';
		}

		// ��������� ������� �������� ��� ������������ ����������� (����������� ��� �������������)
		currentCommentType = rand() % 2 ? commentType::STANDARD : commentType::MULTISTRING;
		if (currentCommentType == commentType::STANDARD) {
			currentComment = START_STANDARD_COMMENT_SYMBOLS + currentComment + STANDARD_COMMENT_END_SYMBOL;
		}
		else currentComment = START_MULTISTRING_COMMENT_SYMBOLS + currentComment + END_MULTRISTRING_COMMENT_SYMBOLS;

		// �������� ���������� ������ ��� ������� ����������� � ��������� ��� � ����� ���������
		size_t currentInsertIndex = findIndexToInsert(codeText, insertElement::COMMENT);
		codeText = codeText.insert(currentInsertIndex, currentComment);
	}

	return codeText;
}