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

// Изменить кодировку консоли и кодировку вывода для работы с русскими символами
void normalizeEncoding();

// Открывает файл и, если он открылся корректно, возвращает поток для его чтения
wifstream openAndCheckFile(wstring inputPrompt);

// Константа, показывающая, какая строка в конфигурационном файле отвечает за флаг удаления комментов (удалять или оставлять)
#define CONFIG_DELETE_COMMENTS L"deleteComments"
// Строка из конфига, отвечающая за флаг изменения имен переменных
#define CONFIG_RENAME_VARIABLES L"renameVariables"
// Строка из конфига, отвечающая за флаг изменения имен функцией
#define CONFIG_RENAME_FUNCTIONS L"renameFunctions"
// Строка из конфига, отвечающая за флаг изменения расположения функций в файле
#define CONFIG_SHUFFLE_FUNCTIONS L"shuffleFunctions"
// Строка из конфига, отвечающая за флаг добавления бессмысленных переменных в код программы
#define CONFIG_ADD_TRASH_VARIABLES L"trashVariables"
// Строка из конфига, отвечающая за флаг добавления бессмысленных функций в код программы
#define CONFIG_ADD_TRASH_FUNCTIONS L"trashFunctions"
// Строка из конфига, отвечающая за флаг добавления бессмысленных циклов в код программы
#define CONFIG_ADD_TRASH_LOOPS L"trashLoops"
// Строка из конфига, отвечающая за флаг добавления бессмысленных комментариев в код программы
#define CONFIG_ADD_TRASH_COMMENTS L"trashComments"

/*Структура, описывающая конфиг, содержащий параметры обфускации.
Каждая bool-переменная отвечает за соответствующий флаг. Если переменная имеет значение true,
то изменения подходящего типа применяются для обфусцируемого файла, если нет - не применяются.*/
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

// Возможные элементы для вставки в текст программы
enum insertElement {COMMENT, VARIABLE, FUNCTION, LOOP};

/*Фунция парсит переданный ей конфигурационный файл построчно, разбивая каждую строку на имя флага и
его значение, принимая его за true или false. Возвращает вышеописанную структуру Config, в которой установлены все имеющиеся
в конфигурационном файле флаги (однако, стоит уточнить: те флаги, значение которых не указано в файле, по умолчанию имеют значение false)*/
Config* parseConfigFile(wifstream& configFile);

/*Функция считывает текст из переданного ей файла с кодом и обфусцирует его, возвращая строку, содержащую весь обфусцированный код.
Параметры обфускации должны быть заданы в переданном конфиге.*/
wstring obfuscate(wifstream& codeFile, Config* config);

// Генерирует случайную строку заданной длины
wstring getRandomString(size_t len);

/* Находит место в коде, куда можно вставить цикл, комментарий или переменную, не изменив функциональность программы.
Второй аргумент - тип вставляемого значения, поскольку места для вставки для функций, циклов и переменных разные - 
например, функции требуется вставлять в глобальную область видимости */
size_t findIndexToInsert(wstring codeText, insertElement insertElementType);

// Функция, удаляющая комментарии из строки, которая представляет из себя полученный из файла код C, C++ или C#
wstring deleteComments(wstring codeText);

// Функция, изменяющая названия всех функцией в коде, представленном в виде строки, и возвращающая измененную строку с кодом
wstring renameFunctions(wstring codeText);

// Функция, изменяющая названия всех переменных в коде
wstring renameVariables(wstring codeText);

// Функция, добавляющая в код бессмысленные комментарии
wstring addTrashComments(wstring codeText);

// Проверяет, находится ли указанный инедекс в запрещенном интервале в тексте программы
BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex);