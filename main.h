#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <cctype>
#include <sstream>
#include <regex>
#include <vector>

using namespace std;

// �������� ��������� ������� � ��������� ������ ��� ������ � �������� ���������
void normalizeEncoding();

// ��������� ���� �, ���� �� �������� ���������, ���������� ����� ��� ��� ������ / ������
wfstream openAndCheckFile(wstring inputPrompt, ios_base::iostate openType = ios_base::in);

/*���������� ������� ��������� "��������" ��������� �� ����� ����� � ���������. �� ��������� - 1 ������� �� 50 �����,
������ ������ ���������� ���������� �������� ���� ��������� �� FREQUENCY_COEFFICIENT (�� ����, �� 1 �� 10 ������� �� 50 �����)*/
#define INSERTION_FREQUENCY_BY_LINES_NUMBER 50
// ��������� ����� �� 1 �� 10, ������������ ����������� ������� ������� "��������" ��������� � ���������
#define FREQUENCY_COEFFICIENT ((rand() % 10) + 1)
// ������� ��������� ������� � ��������� �����������
#define WHITESPACE_FREQUENCY_IN_COMMENTS (rand() % 8)
// ����������� ����� ������ (��������) � ����� � ������� wstring
#define LINE_BREAK wstring(L"\n")
// ������, ������������ ��������� � ���� ���������, � ��������� �������������
#define POINTER_SYMBOL wstring(L"*")
// ������� ������ ������������ ����������� � ���� C � �++
#define START_STANDARD_COMMENT_SYMBOLS wstring(L"//")
// ������, ������� ������ ��������� ����������� ������������ ����������� � ���� C � �++
#define STANDARD_COMMENT_END_SYMBOL LINE_BREAK
// �������, � ������� ������ ���������� ������������� ����������� � ���� C � �++
#define START_MULTISTRING_COMMENT_SYMBOLS wstring(L"/*")
// �������, �������� ������ ������������� ������������� ����������� � ���� C � �++
#define END_MULTRISTRING_COMMENT_SYMBOLS wstring(L"*/")

// ������������ ����� "���������" ����������� ��� ���������� ��� ����� ��������
#define MAX_COMMENT_LENGTH ((rand() % 80) + 20)
// ����� ���������� ����� ��� ���������� � ������� (��� ������ ������� ��� ����������� �����)
#define RANDOM_NAME_LENGTH ((rand() % 8) + 7)

// ���������, ������������, ����� ������ � ���������������� ����� �������� �� ���� �������� ��������� (������� ��� ���������)
#define CONFIG_DELETE_COMMENTS L"deleteComments"
// ������ �� �������, ���������� �� ���� ��������� ���� ����������
#define CONFIG_RENAME_VARIABLES L"renameVariables"
// ������ �� �������, ���������� �� ���� ��������� ���� ��������
#define CONFIG_RENAME_FUNCTIONS L"renameFunctions"
// ������ �� �������, ���������� �� ���� ��������� ������������ ������� � �����
#define CONFIG_SHUFFLE_FUNCTIONS L"shuffleFunctions"
// ������ �� �������, ���������� �� ���� ���������� ������������� ���������� � ��� ���������
#define CONFIG_ADD_TRASH_VARIABLES L"trashVariables"
// ������ �� �������, ���������� �� ���� ���������� ������������� ������� � ��� ���������
#define CONFIG_ADD_TRASH_FUNCTIONS L"trashFunctions"
// ������ �� �������, ���������� �� ���� ���������� ������������� ������ � ��� ���������
#define CONFIG_ADD_TRASH_LOOPS L"trashLoops"
// ������ �� �������, ���������� �� ���� ���������� ������������� ������������ � ��� ���������
#define CONFIG_ADD_TRASH_COMMENTS L"trashComments"
// ������ �������, ���������� �� �������� ���������� ���������� �������� �� ���
#define CONFIG_DELETE_SPACES L"deleteSpaces"

/*���������, ����������� ����������� ��������. ��� �������� (� ������� �� ������ � ������ ���������), � ������� ������ ���������
* �����, ������������� ����������, ����������� � ��� �����, ������ ��� ��� ���� �� ����� ����� ������, ���� ������� ������ ���������.
* ��� ������� ������� �����������, ������������ ��������: ������ ������ - "//", ������ ����� - "\n", ������� - "\".*/
struct ProhibitedInterval {
	size_t startPos; // ������� ��������� �������
	wstring start; // ������� ������ ������������ ���������
	wstring end; // ������� ��������� ������������ ���������
	wchar_t intervalBreak; // ������� ������������ ���������, ���� ���� (������������ ������, ��������� ������� ���������, ��� ����� �� ����)
};

/*���������, ����������� ������, ���������� ��������� ����������.
������ bool-���������� �������� �� ��������������� ����. ���� ���������� ����� �������� true,
�� ��������� ����������� ���� ����������� ��� �������������� �����, ���� ��� - �� �����������.*/
typedef struct _config {
	BOOL deleteSpaces;
	BOOL deleteComments;
	BOOL renameVariables;
	BOOL renameFunctions;
	BOOL shuffleFunctions;
	BOOL addTrashVariables;
	BOOL addTrashFunctions;
	BOOL addTrashLoops;
	BOOL addTrashComments;
} Config;

extern vector<wstring> basicTypes;

// ��������� �������� ��� ������� � ����� ���������
enum class insertElement {COMMENT, VARIABLE, FUNCTION, LOOP};

// ��������� ���� ������������ � ���� C � C++
enum class commentType {STANDARD, MULTISTRING};

/*������ ������ ���������� �� ���������������� ���� ���������, �������� ������ ������ �� ��� ����� �
��� ��������, �������� ��� �� true ��� false. ���������� ������������� ��������� Config, � ������� ����������� ��� ���������
� ���������������� ����� ����� (������, ����� ��������: �� �����, �������� ������� �� ������� � �����, �� ��������� ����� �������� false)*/
Config* parseConfigFile(wfstream& configFile);

/*������� ��������� ����� �� ����������� �� ����� � ����� � ����������� ���, ��������� ������, ���������� ���� ��������������� ���.
��������� ���������� ������ ���� ������ � ���������� �������.*/
wstring obfuscate(wfstream& codeFile, Config* config);

// ���������� ��������� ������ �������� �����
wstring getRandomString(size_t len);

/* ���������� ��������� ������, ���������� ������������� ���������� ���������� ���� � ������� ��������� ��������.
* �� ���� ����� ������������ ����� � �������� ����������:
alwaysSetVariableValue - ������ �� ������������� ���������� ��������, �� ��������� FALSE, �� ����
���������� ����� ���� ���������������� ��� ��������� ��������, �������� int x;
createPointers - ��������� �� �� ������ ���������� ������� �����, � ����� ��������� �� ������� ����. �� ��������� TRUE.
allowSetVariableValue - ��������� �� ������������� ���������� ��������, ��� ��� ���������� �������� ���������������������. �� ��������� TRUE.
endBlockSymbol - ����� ������ ��������� � �������� ����� �����, ����� ������������� ����������. �� ��������� ";"
*/
wstring getRandomVariableInitializationString(BOOL alwaysSetVariableValue = FALSE, BOOL createPointers = TRUE, BOOL allowSetVariableValue = TRUE, wchar_t endBlockSymbol = L';');
// �������� ��� ���������� �� ������ �� �������������
wstring getVariableNameFromInitializationString(wstring variableInitializationString);

/* ������� ����� � ����, ���� ����� �������� ����, ����������� ��� ����������, �� ������� ���������������� ���������.
������ �������� - ��� ������������ ��������, ��������� ����� ��� ������� ��� �������, ������ � ���������� ������ - 
��������, ������� ��������� ��������� � ���������� ������� ��������� */
size_t findIndexToInsert(wstring codeText, insertElement insertElementType);

// �������, ��������� ����������� �� ������, ������� ������������ �� ���� ���������� �� ����� ��� C, C++ ��� C#
wstring deleteComments(wstring codeText);

// �������, ���������� �������� ���� �������� � ����, �������������� � ���� ������, � ������������ ���������� ������ � �����
wstring renameFunctions(wstring codeText);

// �������, ���������� �������� ���� ���������� � ����
wstring renameVariables(wstring codeText);

// �������, ����������� � ��� ������������� �����������
wstring addTrashComments(wstring codeText);

// �������, ����������� � ��� ������������� ����������
wstring addTrashVariables(wstring codeText);

// �������, ����������� � ��� ������������� �����
wstring addTrashLoops(wstring codeText);

// �������, ����������� � ��� ������������� �������
wstring addTrashFunctions(wstring codeText);

// ���������, ��������� �� ��������� ������� � ����������� ��������� � ������ ���������
BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex);

// �������� ���������� ����� (�� ����, ��������� �����) � ����� � �����, ������� ����������� � ���� ������ codeText
size_t getLinesNumberInText(wstring codeText);

// �������, ��������� �� ������ � ����� ��� ���������� ���������� ������� (��, �������� ������� �������� �����������������)
wstring deleteSpaces(wstring codeText);