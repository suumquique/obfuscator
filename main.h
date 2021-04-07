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

// ��������� ���� �, ���� �� �������� ���������, ���������� ����� ��� ��� ������
wifstream openAndCheckFile(wstring inputPrompt);

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

// ��������� �������� ��� ������� � ����� ���������
enum insertElement {COMMENT, VARIABLE, FUNCTION, LOOP};

/*������ ������ ���������� �� ���������������� ���� ���������, �������� ������ ������ �� ��� ����� �
��� ��������, �������� ��� �� true ��� false. ���������� ������������� ��������� Config, � ������� ����������� ��� ���������
� ���������������� ����� ����� (������, ����� ��������: �� �����, �������� ������� �� ������� � �����, �� ��������� ����� �������� false)*/
Config* parseConfigFile(wifstream& configFile);

/*������� ��������� ����� �� ����������� �� ����� � ����� � ����������� ���, ��������� ������, ���������� ���� ��������������� ���.
��������� ���������� ������ ���� ������ � ���������� �������.*/
wstring obfuscate(wifstream& codeFile, Config* config);

// ���������� ��������� ������ �������� �����
wstring getRandomString(size_t len);

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

// ���������, ��������� �� ��������� ������� � ����������� ��������� � ������ ���������
BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex);