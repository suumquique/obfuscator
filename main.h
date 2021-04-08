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

// Открывает файл и, если он открылся корректно, возвращает поток для его чтения / записи
wfstream openAndCheckFile(wstring inputPrompt, ios_base::iostate openType = ios_base::in);

/*Количество вставок различных "мусорных" элементов на число строк в программе. По умолчанию - 1 вставка на 50 строк,
однако обычно количество рандомится делением этой константы на FREQUENCY_COEFFICIENT (то есть, от 1 до 10 вставок на 50 строк)*/
#define INSERTION_FREQUENCY_BY_LINES_NUMBER 50
// Случайное число от 1 до 10, определяющее коэффициент частоты вставок "мусорных" элементов в программу
#define FREQUENCY_COEFFICIENT ((rand() % 10) + 1)
// Частота появления пробела в случайном комментарии
#define WHITESPACE_FREQUENCY_IN_COMMENTS (rand() % 8)
// Определение конца строки (переноса) в файле в формате wstring
#define LINE_BREAK wstring(L"\n")
// Символ, обозначающий указатель в коде программы, в строковом представлении
#define POINTER_SYMBOL wstring(L"*")
// Символы начала стандартного комментария в коде C и С++
#define START_STANDARD_COMMENT_SYMBOLS wstring(L"//")
// Символ, которым всегда кончается стандартный однострочный комментарий в коде C и С++
#define STANDARD_COMMENT_END_SYMBOL LINE_BREAK
// Символы, с которых всегда начинается многострочный комментарий в коде C и С++
#define START_MULTISTRING_COMMENT_SYMBOLS wstring(L"/*")
// Символы, которыми всегда заканчивается многострочный комментарий в коде C и С++
#define END_MULTRISTRING_COMMENT_SYMBOLS wstring(L"*/")

// Максимальная длина "мусорного" комментария для добавления без учета пробелов
#define MAX_COMMENT_LENGTH ((rand() % 80) + 20)
// Длина случайного имени для переменных и функций (для замены текущих или добавлениях новых)
#define RANDOM_NAME_LENGTH ((rand() % 8) + 7)

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
// Строка конфига, отвечающая за удаление незначащих пробельных символов из код
#define CONFIG_DELETE_SPACES L"deleteSpaces"

/*Структура, описывающая запрещенный интервал. Это интервал (с индекса по индекс в тексте программы), в который нельзя добавлять
* циклы, инициализацию переменных, комментарии и так далее, потому что это либо не будет иметь смысла, либо нарушит работу программы.
* Для примера возьмем стандартный, однострочный интервал: строка начала - "//", строка конца - "\n", перенос - "\".*/
struct ProhibitedInterval {
	size_t startPos; // Текущая стартовая позиция
	wstring start; // Признак начала запрещенного интервала
	wstring end; // Признак окончания запрещенного интервала
	wchar_t intervalBreak; // Перенос запрещенного интервала, если есть (экранирующий символ, убирающий признак окончания, его может не быть)
};

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

extern vector<wstring> basicTypes;

// Возможные элементы для вставки в текст программы
enum class insertElement {COMMENT, VARIABLE, FUNCTION, LOOP};

// Возможные типы комментариев в коде C и C++
enum class commentType {STANDARD, MULTISTRING};

/*Фунция парсит переданный ей конфигурационный файл построчно, разбивая каждую строку на имя флага и
его значение, принимая его за true или false. Возвращает вышеописанную структуру Config, в которой установлены все имеющиеся
в конфигурационном файле флаги (однако, стоит уточнить: те флаги, значение которых не указано в файле, по умолчанию имеют значение false)*/
Config* parseConfigFile(wfstream& configFile);

/*Функция считывает текст из переданного ей файла с кодом и обфусцирует его, возвращая строку, содержащую весь обфусцированный код.
Параметры обфускации должны быть заданы в переданном конфиге.*/
wstring obfuscate(wfstream& codeFile, Config* config);

// Генерирует случайную строку заданной длины
wstring getRandomString(size_t len);

/* Генерируем случайную строку, содержащую инициализацию переменной случайного типа и имеющую случайное значение.
* На вход могут передаваться флаги в качестве параметров:
alwaysSetVariableValue - всегда ли устанавливать переменной значение, по умолчанию FALSE, то есть
переменная может быть инициалазирована без установки значения, например int x;
createPointers - создавать ли не только переменные базовых типов, а также указатели на базовые типы. По умолчанию TRUE.
allowSetVariableValue - разрешить ли устанавливать переменной значение, или все переменные отдавать неинициализированными. По умолчанию TRUE.
endBlockSymbol - какой символ добавлять в качестве конца блока, после инициализации переменной. По умолчанию ";"
*/
wstring getRandomVariableInitializationString(BOOL alwaysSetVariableValue = FALSE, BOOL createPointers = TRUE, BOOL allowSetVariableValue = TRUE, wchar_t endBlockSymbol = L';');
// Получаем имя переменной из строки ее инициализации
wstring getVariableNameFromInitializationString(wstring variableInitializationString);

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

// Функция, добавляющая в код бессмысленные переменные
wstring addTrashVariables(wstring codeText);

// Функция, добавляющая в код бессмысленные циклы
wstring addTrashLoops(wstring codeText);

// Функция, добавляющая в код бессмысленные функции
wstring addTrashFunctions(wstring codeText);

// Проверяет, находится ли указанный инедекс в запрещенном интервале в тексте программы
BOOL isInProhibitedInterval(wstring codeText, size_t insertIndex);

// Получает количество строк (то есть, переносов строк) в файле с кодом, который представлен в виде строки codeText
size_t getLinesNumberInText(wstring codeText);

// Функция, удаляющая из строки с кодом все незначащие пробельные символы (те, удаление которых повредит работоспособности)
wstring deleteSpaces(wstring codeText);