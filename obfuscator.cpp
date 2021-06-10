#include "main.hpp"

wstring obfuscate(wfstream& codeFile, Config* config) {
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
	if (config->addTrashVariables) codeText = addTrashVariables(codeText);
	if (config->addTrashLoops) codeText = addTrashLoops(codeText);
	if (config->addTrashFunctions) codeText = addTrashFunctions(codeText);
	if (config->deleteSpaces) codeText = deleteSpaces(codeText);

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
		newFunctionReplacement = wstring(L"$1") + getRandomString(RANDOM_NAME_LENGTH) + wstring(L"$2");

		// �������� ������� ��� �� ����� ��� ������� �� ���� ������ ���������
		codeText = regex_replace(codeText, functionNameRegExp, newFunctionReplacement);
	}

	return codeText;
}

BOOL isValidVariableType(wstring variableType) {
	if (variableType == L"namespace" || variableType == L"return" || variableType == L"else") return FALSE;
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
	wregex variablesRegex(LR"((\w+)[*&]* +([A-Za-z]\w*) *(?:;| +=|,|\[[\d\w]+\]|\) *[;{]| *\{| *\((?:(?:[\w]+)(?:, )?)+\);)+)");

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
			newVariableReplacement = L"$1" + getRandomString(RANDOM_NAME_LENGTH) + L"$2";
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
		currentInsertIndex = findIndexToInsert(codeText, insertElement::COMMENT);
		codeText = codeText.insert(currentInsertIndex, currentComment);
	}

	return codeText;
}

wstring addTrashVariables(wstring codeText) {
	// �������, ������� ���������� ����� ���������
	size_t numberOfVariables = getLinesNumberInText(codeText) / INSERTION_FREQUENCY_BY_LINES_NUMBER * FREQUENCY_COEFFICIENT;
	wstring currentVariableString; // ������, ���������� ������������� ������� ����������
	size_t currentInsertIndex; // ������ ��� ������� ������� ���������� � ���������

	for (size_t i = 0; i < numberOfVariables; i++) {
		// �������� ��������� ���������� � ��������� � ������ ������� ������
		currentVariableString = LINE_BREAK + getRandomVariableInitializationString();

		// �������� ���������� ������ ��� ������� ���������� � ��������� ��� � ����� ���������
		currentInsertIndex = findIndexToInsert(codeText, insertElement::VARIABLE);
		codeText = codeText.insert(currentInsertIndex, currentVariableString);
	}

	return codeText;
}

wstring addTrashLoops(wstring codeText) {
	// ��������, ������� ������������� ������ ����� ���������
	size_t numberOfCycles = getLinesNumberInText(codeText) / INSERTION_FREQUENCY_BY_LINES_NUMBER * FREQUENCY_COEFFICIENT;
	enum class loopTypes {WHILE, FOR, DO_WHILE}; // ��������� ���� ������
	loopTypes currentLoopType; // ������� ��� �����
	wstring currentCycleVariableInitialization; // ������, ���������� ������������� ������������ ���������� �����
	wstring currentCycleVariableName; // ��� ������� ������������ ���������� � �����
	wstring cycleStartPart; // ������ ����� (��, ��� �� ���� �����)
	wstring currentCycleBody = L""; // ���� (������� �����) �������� �����, �� ��������� ������
	wstring cycleEndPart; // ����������� ����� ����� (��, ��� ����� ���� �����)
	wstring currentLoopString; // ����� ������ ����� �����
	wstring currentRandomLoopLimit; // ������� ������� �������� � ����������� �����
	wstring currentLoopStep; // ��� ���������� � ������� �����
	wstring comparisonOperator = L" < "; // �������� ��������� (������) ��� ������
	wstring positiveAssignmentOperator = L" += "; // �������� ������������ � �������� ������������ ��� ������
	wstring endCycleBodySymbol = L"}"; // ����������� ������ ���� �����
	wstring variableInsideCycleInitialization; // ��������� ����������, ������������� ������� ������� ������ �����
	wstring variableInsideCycleName; // ��� ���������� ������ �����
	size_t currentInsertIndex; // ������ ��� ������� �������� ����� � ���������


	for (size_t i = 0; i < numberOfCycles; i++) {
		// �������� ������ � �������������� ��������� ���������� �������� ���������, ������ ���������� ����� �� �������� ����������
		currentCycleVariableInitialization = getRandomVariableInitializationString(TRUE, FALSE);
		// �������� ��� �������������� ���������� � ��������� ����
		currentCycleVariableName = getVariableNameFromInitializationString(currentCycleVariableInitialization);
		// ������� ����� �����, �� ���� ����� ���� ��������
		currentRandomLoopLimit = to_wstring(rand() % CHAR_MAX);
		// ������� ��� ����� (�� 2 �� 10, ����� ���� ��� �� �����������)
		currentLoopStep = to_wstring((rand() % 9) + 2);
		// ������� ���������� ��� ���������� ����� �������
		variableInsideCycleInitialization = getRandomVariableInitializationString(TRUE, FALSE);
		variableInsideCycleName = getVariableNameFromInitializationString(variableInsideCycleInitialization);

		// �������� ��� ����� - for, while ��� do...while
		currentLoopType = static_cast<loopTypes>(rand() % 3);
		switch (currentLoopType) {
		case loopTypes::FOR:

			// ������� ������ ����� for, �������� "for(size_t i = 0; i < 15; i+=2){"
			cycleStartPart = wstring(L"for (") + currentCycleVariableInitialization + currentCycleVariableName +
				comparisonOperator + currentRandomLoopLimit + wstring(L"; ") + currentCycleVariableName + positiveAssignmentOperator +
				currentLoopStep + wstring(L") {") + LINE_BREAK;
			// ��������� � ����� ����� ����������� ������ � ������� ������
			cycleEndPart = endCycleBodySymbol + LINE_BREAK;
			break;
		case loopTypes::WHILE:

			/* ������� ������ ����� while. �������, �� �����, ��������� ���������������� ������������ ����������, ����� �������
			* while(), � ������ ������ ��������� �������, � ����� ������� ���� ���� ����� */
			cycleStartPart = currentCycleVariableInitialization + LINE_BREAK + wstring(L"while(") + currentCycleVariableName +
				comparisonOperator + currentRandomLoopLimit + wstring(L") {") + LINE_BREAK + currentCycleVariableName +
				positiveAssignmentOperator + currentLoopStep + wstring(L";") + LINE_BREAK;
			// ��������� � ����� ����� ����������� ������ � ������� ������
			cycleEndPart = endCycleBodySymbol + LINE_BREAK;
			break;
		case loopTypes::DO_WHILE:

			/* � ������ ������ � ������ ��������� �� ����� ���������������� ������������ ����������, � ����� ������ �������� do{
			* � � ���� ����� ��������� � �������� �������� ��� ����� */
			cycleStartPart = currentCycleVariableInitialization + LINE_BREAK + wstring(L"do {") + LINE_BREAK +
				currentCycleVariableName + positiveAssignmentOperator + currentLoopStep + wstring(L";") + LINE_BREAK;
			// ����� ��� ������� � �����: ����� �������� ���� ����� ��������� ��������� while() � �������� � �������
			cycleEndPart = endCycleBodySymbol + wstring(L" while(") + currentCycleVariableName +
				comparisonOperator + currentRandomLoopLimit + wstring(L");") + LINE_BREAK;
			break;
		}

		// ��������� ������� ����������, ����� �� ������ ����� (� ���� �����) ��������� �������� ����������
		if (rand() % 2 == 0) {
			// ��������� ������������� �������� ����������
			currentCycleBody += variableInsideCycleInitialization + LINE_BREAK;
			// ��������� �������� � ��� � � ���������� ������ �����
			currentCycleBody += variableInsideCycleName + (rand() % 2 ? wstring(L" = ") : positiveAssignmentOperator)
				+ currentCycleVariableName + wstring(L";") +LINE_BREAK;
		}

		// �������� ������ ������ �����
		currentLoopString = LINE_BREAK + cycleStartPart + currentCycleBody + cycleEndPart;
		// ������� ������� ���� �����, ����� � ��������� ��� ��� �� ���������� � ���������� 
		currentCycleBody = L"";

		// �������� ���������� ������ ��� ������� ����� � ��������� ��� � ����� ���������
		currentInsertIndex = findIndexToInsert(codeText, insertElement::LOOP);
		codeText = codeText.insert(currentInsertIndex, currentLoopString);
	}

	return codeText;
}

wstring addTrashFunctions(wstring codeText) {
	// ��������, ������� ������������� ������� ����� ���������
	size_t numberOfFunctions = getLinesNumberInText(codeText) / INSERTION_FREQUENCY_BY_LINES_NUMBER * FREQUENCY_COEFFICIENT;
	wstring currentFunctionName; // ��� ������� �������
	wstring currentFunctionType; // ��� ������� �������
	wstring currentFunctionPrototype; // �������� �������
	wstring currentFunctionBody; // "����" ������� �������
	wstring currentFunctionString; // ����� ��������� ������������� ������� ��������� �������
	wstring currentReturningVariableName; // ��� ����������, ������� ����� ���������� �� ������� �������
	wstring currentReturningVariableInitialization; // ������������� ����������, ������� ����� ���������� �� ������� �������
	BOOL isCurrentFunctionTypePointer; // ����� �� ��� ������� ����������
	size_t currentFunctionArgumentNumber; // ���������� ���������� � ������� �������
	size_t currentInsertIndex; // ������ ��� ������� ������� ������� � ���������

	for (size_t i = 0; i < numberOfFunctions; i++) {
		currentFunctionName = getRandomString(RANDOM_NAME_LENGTH);
		isCurrentFunctionTypePointer = rand() % 2;
		// �������� ��������� ��� ��� ������� �������� � �������� ������� ����� �� ������ ���, � ��������� �� ����
		currentFunctionType = basicTypes[rand() % basicTypes.size()] + (isCurrentFunctionTypePointer ? POINTER_SYMBOL : L"");
		currentFunctionPrototype = currentFunctionType + wstring(L" ") + currentFunctionName + wstring(L"(");

		// �� ���� �� ������� ���������� � �������
		currentFunctionArgumentNumber = rand() % 4;
		for (size_t k = 0; k < currentFunctionArgumentNumber; k++) {
			/* �������� ������� �������� � ���� ������������� ���������� ��� ��������� ��������, � �������� � "," �����,
			� ��������� ��� � ��������. �������� ����� ���� ���������� */
			currentFunctionPrototype += getRandomVariableInitializationString(FALSE, TRUE, FALSE, L',');
		}
		// ���� � ������� ���� ���� ���� ��������, ������� ��������� ����������� �������
		if (currentFunctionArgumentNumber) currentFunctionPrototype.erase(currentFunctionPrototype.length() - 1);
		// ��������� ����������� ������ � ���������
		currentFunctionPrototype += L')';

		// �������� ��� ����������, ������� ����� ���������� �� �������
		currentReturningVariableName = getRandomString(RANDOM_NAME_LENGTH);
		// �������������� ��� ����������, ��� ������ �����, ����������� � ����� ������������� �������� ��������
		currentReturningVariableInitialization = currentFunctionType + wstring(L" ") + currentReturningVariableName +
			wstring(L" = ") + (isCurrentFunctionTypePointer ? wstring(L"0") : to_wstring(rand() % CHAR_MAX)) + wstring(L";");
		// ���������� "����" �����, ��, ��� ������ �������� ������
		currentFunctionBody = wstring(L"{") + LINE_BREAK + currentReturningVariableInitialization + LINE_BREAK +
			wstring(L"return ") + currentReturningVariableName + wstring(L";") + LINE_BREAK + wstring(L"}") + LINE_BREAK;

		// ���������� �� ��������� (�������������) � ���� ������� ���� �������
		currentFunctionString = LINE_BREAK + currentFunctionPrototype + currentFunctionBody;
	
		// �������� ���������� ������ ��� ������� ������� � ��������� ��� � ����� ���������
		currentInsertIndex = findIndexToInsert(codeText, insertElement::FUNCTION);
		codeText = codeText.insert(currentInsertIndex, currentFunctionString);
	}

	return codeText;
}

wstring deleteSpaces(wstring codeText) {
	// ���� ������� ��� ����� ������� ��������� ���� �� ������ ��������, ��� ������, ��� ������ ����� �������
	wstring allowedSymbols = L"=; \n\t\f{}+-,/ <>[]()!";
	// ���� ���� �� ������������� �������� - ������� ������ �� � ���� ������ ������
	wstring prohibitedSymbols = L"#";
	/* ����������, ����� �� ����� ������� ���������� ������ �� ������ �����, ��������� ������� ����������� ��������,
	* �������������� ����. ��������, ���� ������ ����� ����� �������, ������� ��� ����� �� ����� */
	BOOL mayDeleteSpaceSymbol; 
	// ����������, �������� �� ������ � ��� ������ ��� ����� ������� � ����������� ��������
	BOOL fallsIntoProhibitedInterval;
	size_t codeTextLen = codeText.length();
	

	for (size_t i = 0; i < codeTextLen - 2; i++) {
		// ���� ������ �� ���������� - ����� �� ����������=��
		if (!iswspace(codeText[i])) continue;

		// �������, ���� �� ���������� ��� ��������� ������� � ����������� �������� � ��� �� ��� � �����������
		mayDeleteSpaceSymbol = (allowedSymbols.find(codeText[i - 1]) != wstring::npos || allowedSymbols.find(codeText[i + 1]) != wstring::npos) &&
			prohibitedSymbols.find(codeText[i - 1]) == wstring::npos && prohibitedSymbols.find(codeText[i + 1]) == wstring::npos;
		if (!mayDeleteSpaceSymbol) continue;

		// ���������, �� ��������� �� ������ ��� ��� ������� (������, +-1 �� �������) � ����������� ���������
		fallsIntoProhibitedInterval = isInProhibitedInterval(codeText, i) || isInProhibitedInterval(codeText, i + 1)
			|| isInProhibitedInterval(codeText, i - 1);
		if (fallsIntoProhibitedInterval) continue;

		// ������� ������� ������
		codeText = codeText.erase(i, 1);
		/* ��������� �� ������� �����, ��������� ���� �� ����� �� �������, �� ���������� ��������� ������, ��� ���
		* ������� ������ � �� ��� ������� ��� ��������� ��������� */
		i--;
		// ��������� ����� ������
		codeTextLen = codeText.length();
	}

	return codeText;
}