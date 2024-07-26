#include <windows.h>
#include <shlobj.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <vector>
#include <cstdio>

#define ID_START_WRITE_TEST_SEQUENTIAL 1001
#define ID_START_READ_TEST_SEQUENTIAL 1002
#define ID_START_WRITE_TEST_RANDOM 1003
#define ID_START_READ_TEST_RANDOM 1004
#define ID_CHOOSE_DISK 1005

bool IsSequentialReadAvailable = false;
bool IsRandomReadAvailable = false;


HWND hwnd;
HWND hStatus;
HWND hMessage;
HWND hStartWriteTestSequentialButton;
HWND hStartReadTestSequentialButton;
HWND hStartWriteTestRandomButton;
HWND hStartReadTestRandomButton;
HWND hChosseDiskButton;

std::wstring currentPath = L"";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void WriteTestSequential();
void ReadTestSequential();
void WriteTestRandom();
void ReadTestRandom();
void ChooseDisk();
void DeleteTestFiles(std::wstring FilePath1, std::wstring FilePath2, std::wstring FilePath3);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";

    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0,
        L"MyWindowClass",
        L"FlashDrive Tester",
        WS_OVERLAPPEDWINDOW /* & S_THICKFRAME & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX */ ,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 500,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    hChosseDiskButton = CreateWindow(
        L"BUTTON",
        L"Выбор флеш-накопителя",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        250, 70, 300, 50,
        hwnd,
        (HMENU)ID_CHOOSE_DISK,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);
    hStartWriteTestSequentialButton = CreateWindow(
        L"BUTTON",
        L"Начать тест последовательной записи",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        85, 140, 300, 50,
        hwnd,
        (HMENU)ID_START_WRITE_TEST_SEQUENTIAL,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);
    hStartReadTestSequentialButton = CreateWindow(
        L"BUTTON",
        L"Начать тест последовательного чтения",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        85, 210, 300, 50,
        hwnd,
        (HMENU)ID_START_READ_TEST_SEQUENTIAL,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);
   hStartWriteTestRandomButton = CreateWindow(
       L"BUTTON",
       L"Начать тест случайной записи",
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
       415, 140, 300, 50,
       hwnd,
       (HMENU)ID_START_WRITE_TEST_RANDOM,
       (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
       NULL);
   hStartReadTestRandomButton = CreateWindow(
       L"BUTTON",
       L"Начать тест случайного чтения",
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
       415, 210, 300, 50,
       hwnd,
       (HMENU)ID_START_READ_TEST_RANDOM,
       (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
       NULL);

   hStatus = CreateWindow(L"STATIC", L"Выберите вид тестирования.\nСоответствуюший тест чтения доступен после соответствующего теста записи.",
       WS_VISIBLE | WS_CHILD, 250, 290, 300, 50, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
   hMessage = CreateWindow(L"STATIC", L"Извлечение тестируемого носителя может вызвать некорректное поведение программы.",
	   WS_VISIBLE | WS_CHILD, 250, 360, 300, 50, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

    EnableWindow(hStartWriteTestSequentialButton, FALSE);
    EnableWindow(hStartWriteTestRandomButton, FALSE);
    EnableWindow(hStartReadTestSequentialButton, FALSE);
    EnableWindow(hStartReadTestRandomButton, FALSE);
	EnableWindow(hChosseDiskButton, TRUE);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_START_WRITE_TEST_SEQUENTIAL:
            WriteTestSequential();
            break;
        case ID_START_READ_TEST_SEQUENTIAL:
            ReadTestSequential();
            break;
        case ID_START_WRITE_TEST_RANDOM:
            WriteTestRandom();
            break;
        case ID_START_READ_TEST_RANDOM:
            ReadTestRandom();
            break;
		case ID_CHOOSE_DISK:
			ChooseDisk();
			break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void ChooseDisk() {
	MessageBox(hwnd, L"Перед выбором накопителя для тестирования убедитесь, что он был предварительно отформатирован и имеет общий объём не менее 4 ГБ.", L"Примечание", MB_OK | MB_ICONWARNING);
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"Выберите flash-носитель";
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != NULL) {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            wchar_t drive[4] = { path[0], path[1], path[2], L'\0' };
            if (GetDriveType(drive) == DRIVE_REMOVABLE) {
                CoTaskMemFree(pidl);
                currentPath = path;
                EnableWindow(hStartWriteTestSequentialButton, TRUE);
                EnableWindow(hStartWriteTestRandomButton, TRUE);
				EnableWindow(hStartReadTestSequentialButton, FALSE);
				EnableWindow(hStartReadTestRandomButton, FALSE);
            }
            else {
                MessageBox(NULL, L"Пожалуйста, выберите флеш-накопитель.", L"Предупреждение", MB_OK | MB_ICONWARNING);
            }
        }
    }
}

void WriteTestSequential() {
	std::wstring StatusText = L"Идёт тестирование последовательной записи, ожидайте результатов.";
	SetWindowText(hStatus, StatusText.c_str());
	EnableWindow(hStartWriteTestSequentialButton, FALSE);
	EnableWindow(hStartReadTestSequentialButton, FALSE);
	EnableWindow(hStartWriteTestRandomButton, FALSE);
	EnableWindow(hStartReadTestRandomButton, FALSE);
	EnableWindow(hChosseDiskButton, FALSE);

	//Siquental Write Tests
	std::wstring fileName25MBSequential = L"testfile25MBSequential.bin";
	std::wstring fileName250MBSequential = L"testfile250MBSequential.bin";
	std::wstring fileName2500MBSequential = L"testfile2500MBSequential.bin";
	std::wstring filePath25MBSequential = currentPath + fileName25MBSequential;
	std::wstring filePath250MBSequential = currentPath + fileName250MBSequential;
	std::wstring filePath2500MBSequential = currentPath + fileName2500MBSequential;
	const int dataSize25MBSequential = 25000000;
	const int dataSize250MBSequential = 250000000;
	const long long int dataSize2500MBSequential = 2500000000;

	//25MB Sequential Write Test
	std::string data25MBSequential(dataSize25MBSequential, 'A');
	auto startTime25MBSequential = std::chrono::high_resolution_clock::now();
	std::ofstream outFile25MBSequential(filePath25MBSequential, std::ios::binary);
	if (outFile25MBSequential.is_open()) {
		outFile25MBSequential.write(data25MBSequential.data(), dataSize25MBSequential);
		outFile25MBSequential.close();
	}
	else {
		MessageBox(NULL, L"Ошибка открытия флеш-накопителя (25 МБ Тест Последовательной Записи).", L"Предупреждение", MB_OK | MB_ICONERROR);
		if (IsRandomReadAvailable) {
			EnableWindow(hStartReadTestRandomButton, TRUE);
		}
		EnableWindow(hStartWriteTestSequentialButton, TRUE);
		EnableWindow(hStartReadTestSequentialButton, FALSE);
		EnableWindow(hStartWriteTestRandomButton, TRUE);
		EnableWindow(hChosseDiskButton, TRUE);
		SetWindowText(hStatus, L"Выберите вид тестирования.");
		return;
	}
	auto endTime25MBSequential = std::chrono::high_resolution_clock::now();
	auto duration25MBSequential = std::chrono::duration_cast<std::chrono::milliseconds>(endTime25MBSequential - startTime25MBSequential).count();
	double speedWrite25MBSequential = (dataSize25MBSequential / 1024.0 / 1024.0) / (duration25MBSequential / 1000.0);

	//250MB Sequential Write Test
	std::string data250MBSequential(dataSize250MBSequential, 'A');
	auto startTime250MBSequential = std::chrono::high_resolution_clock::now();
	std::ofstream outFile250MBSequential(filePath250MBSequential, std::ios::binary);
	if (outFile250MBSequential.is_open()) {
		outFile250MBSequential.write(data250MBSequential.data(), dataSize250MBSequential);
		outFile250MBSequential.close();
	}
	else {
		MessageBox(NULL, L"Ошибка открытия флеш-накопителя (250 МБ Тест Последовательной Записи).", L"Предупреждение", MB_OK | MB_ICONERROR);
		if (IsRandomReadAvailable) {
			EnableWindow(hStartReadTestRandomButton, TRUE);
		}
		EnableWindow(hStartWriteTestSequentialButton, TRUE);
		EnableWindow(hStartReadTestSequentialButton, FALSE);
		EnableWindow(hStartWriteTestRandomButton, TRUE);
		EnableWindow(hChosseDiskButton, TRUE);
		SetWindowText(hStatus, L"Выберите вид тестирования.");
		return;
	}
	auto endTime250MBSequential = std::chrono::high_resolution_clock::now();
	auto duration250MBSequential = std::chrono::duration_cast<std::chrono::milliseconds>(endTime250MBSequential - startTime250MBSequential).count();
	double speedWrite250MBSequential = (dataSize250MBSequential / 1024.0 / 1024.0) / (duration250MBSequential / 1000.0);

	//2500MB Sequential Write Test
	std::string data2500MBSequential(dataSize2500MBSequential, 'A');
	auto startTime2500MBSequential = std::chrono::high_resolution_clock::now();
	std::ofstream outFile2500MBSequential(filePath2500MBSequential, std::ios::binary);
	if (outFile2500MBSequential.is_open()) {
		outFile2500MBSequential.write(data2500MBSequential.data(), dataSize2500MBSequential);
		outFile2500MBSequential.close();
	}
	else {
		MessageBox(NULL, L"Ошибка открытия флеш-накопителя (2500 МБ Тест Последовательной Записи).", L"Предупреждение", MB_OK | MB_ICONERROR);
		if (IsRandomReadAvailable) {
			EnableWindow(hStartReadTestRandomButton, TRUE);
		}
		EnableWindow(hStartWriteTestSequentialButton, TRUE);
		EnableWindow(hStartReadTestSequentialButton, FALSE);
		EnableWindow(hStartWriteTestRandomButton, TRUE);
		EnableWindow(hChosseDiskButton, TRUE);
		SetWindowText(hStatus, L"Выберите вид тестирования.");
		return;
	}
	auto endTime2500MBSequential = std::chrono::high_resolution_clock::now();
	auto duration2500MBSequential = std::chrono::duration_cast<std::chrono::milliseconds>(endTime2500MBSequential - startTime2500MBSequential).count();
	double speedWrite2500MBSequential = (dataSize2500MBSequential / 1024.0 / 1024.0) / (duration2500MBSequential / 1000.0);

	std::wstring Output25MBP1Sequential = L"Длительность последовательной записи(25МБ):\n" + std::to_wstring((duration25MBSequential / 1000.0)) + L" секунд \n";
	std::wstring Output25MBP2Sequential = L"Средняя скорость последовательной записи(25МБ):\n" + std::to_wstring(speedWrite25MBSequential) + L" МБ/c \n\n";
	std::wstring Output250MBP1Sequential = L"Длительность последовательной записи(250МБ):\n" + std::to_wstring((duration250MBSequential / 1000.0)) + L" секунд \n";
	std::wstring Output250MBP2Sequential = L"Средняя скорость последовательной записи(250МБ):\n" + std::to_wstring(speedWrite250MBSequential) + L" МБ/c \n\n";
	std::wstring Output2500MBP1Sequential = L"Длительность последовательной записи(2500МБ):\n" + std::to_wstring((duration2500MBSequential / 1000.0)) + L" секунд \n";
	std::wstring Output2500MBP2Sequential = L"Средняя скорость последовательной записи(2500МБ):\n" + std::to_wstring(speedWrite2500MBSequential) + L" МБ/c \n\n";
	std::wstring WriteTestOutput = Output25MBP1Sequential + Output25MBP2Sequential + Output250MBP1Sequential + Output250MBP2Sequential + Output2500MBP1Sequential + Output2500MBP2Sequential;
	MessageBox(hwnd, WriteTestOutput.c_str(), L"Результат теста последовательной записи", MB_OK | MB_ICONINFORMATION);
	IsSequentialReadAvailable = true;
	if (IsRandomReadAvailable) {
		EnableWindow(hStartReadTestRandomButton, TRUE);
	}
	EnableWindow(hStartWriteTestSequentialButton, TRUE);
	EnableWindow(hStartReadTestSequentialButton, TRUE);
	EnableWindow(hStartWriteTestRandomButton, TRUE);
	EnableWindow(hChosseDiskButton, TRUE);
	SetWindowText(hStatus, L"Выберите вид тестирования.");
}

void WriteTestRandom() {
	std::wstring StatusText = L"Идёт тестирование случайной записи, ожидайте результатов.";
	SetWindowText(hStatus, StatusText.c_str());
	EnableWindow(hStartWriteTestSequentialButton, FALSE);
	EnableWindow(hStartReadTestSequentialButton, FALSE);
	EnableWindow(hStartWriteTestRandomButton, FALSE);
	EnableWindow(hStartReadTestRandomButton, FALSE);
	EnableWindow(hChosseDiskButton, FALSE);

	//Random Write Tests
	std::wstring fileName250KBRandom = L"testfile250KBRandom.bin";
	std::wstring fileName2500KBRandom = L"testfile2500KBRandom.bin";
	std::wstring fileName25000KBRandom = L"testfile25000KBRandom.bin";
	std::wstring filePath250KBRandom = currentPath + fileName250KBRandom;
	std::wstring filePath2500KBRandom = currentPath + fileName2500KBRandom;
	std::wstring filePath25000KBRandom = currentPath + fileName25000KBRandom;
	const int dataSize250KBRandom = 250000;
	const int dataSize2500KBRandom = 2500000;
	const int dataSize25000KBRandom = 25000000;
	const size_t blockSize = 4096;

	//250KB Random Write Test
	std::string data250KBRandom(dataSize250KBRandom, 'A');
	std::srand(static_cast<unsigned int>(std::time(0)));
	size_t arraySize250KBRandom = (dataSize250KBRandom / blockSize) + 1;
	std::vector<bool> writtenPositions250KBRandom(arraySize250KBRandom, false);
	auto startTime250KBRandom = std::chrono::high_resolution_clock::now();
	std::ofstream outFile250KBRandom(filePath250KBRandom, std::ios::binary | std::ios::out);
	if (outFile250KBRandom.is_open()) {
		for (size_t i = 0; i < dataSize250KBRandom; i += blockSize) {
			size_t index = 0;
			size_t writePosition;
			do {
				index = std::rand() % arraySize250KBRandom;
				writePosition = index * blockSize;
			} while (writtenPositions250KBRandom[index]);
			size_t writeSize = static_cast<size_t>((blockSize < (dataSize250KBRandom - writePosition)) ? blockSize : (dataSize250KBRandom - writePosition));
			outFile250KBRandom.seekp(writePosition, std::ios::beg);
			outFile250KBRandom.write(data250KBRandom.data() + i, writeSize);
			writtenPositions250KBRandom[index] = true;
		}
		outFile250KBRandom.close();
	}
	else {
		MessageBox(NULL, L"Ошибка открытия флеш-накопителя (250 КБ Тест Случайной Записи).", L"Предупреждение", MB_OK | MB_ICONERROR);
		if (IsSequentialReadAvailable) {
			EnableWindow(hStartReadTestSequentialButton, TRUE);
		}
		EnableWindow(hStartWriteTestSequentialButton, TRUE);
		EnableWindow(hStartReadTestRandomButton, FALSE);
		EnableWindow(hStartWriteTestRandomButton, TRUE);
		EnableWindow(hChosseDiskButton, TRUE);
		SetWindowText(hStatus, L"Выберите вид тестирования.");
		return;
	}
	auto endTime250KBRandom = std::chrono::high_resolution_clock::now();
	auto duration250KBRandom = std::chrono::duration_cast<std::chrono::milliseconds>(endTime250KBRandom - startTime250KBRandom).count();
	double speedWrite250KBRandom = (dataSize250KBRandom / 1024.0 / 1024.0) / (duration250KBRandom / 1000.0);

	//2500KB Random Write Test
	std::string data2500KBRandom(dataSize2500KBRandom, 'A');
	std::srand(static_cast<unsigned int>(std::time(0)));
	size_t arraySize2500KBRandom = (dataSize2500KBRandom / blockSize) + 1;
	std::vector<bool> writtenPositions2500KBRandom(arraySize2500KBRandom, false);
	auto startTime2500KBRandom = std::chrono::high_resolution_clock::now();
	std::ofstream outFile2500KBRandom(filePath2500KBRandom, std::ios::binary | std::ios::out);
	if (outFile2500KBRandom.is_open()) {
		for (size_t i = 0; i < dataSize2500KBRandom; i += blockSize) {
			size_t index = 0;
			size_t writePosition;
			do {
				index = std::rand() % arraySize2500KBRandom;
				writePosition = index * blockSize;
			} while (writtenPositions2500KBRandom[index]);
			size_t writeSize = static_cast<size_t>((blockSize < (dataSize2500KBRandom - writePosition)) ? blockSize : (dataSize2500KBRandom - writePosition));
			outFile2500KBRandom.seekp(writePosition, std::ios::beg);
			outFile2500KBRandom.write(data2500KBRandom.data() + i, writeSize);
			writtenPositions2500KBRandom[index] = true;
		}
		outFile2500KBRandom.close();
	}
	else {
		MessageBox(NULL, L"Ошибка открытия флеш-накопителя (2500 КБ Тест Случайной Записи).", L"Предупреждение", MB_OK | MB_ICONERROR);
		if (IsSequentialReadAvailable) {
			EnableWindow(hStartReadTestSequentialButton, TRUE);
		}
		EnableWindow(hStartWriteTestSequentialButton, TRUE);
		EnableWindow(hStartReadTestRandomButton, FALSE);
		EnableWindow(hStartWriteTestRandomButton, TRUE);
		EnableWindow(hChosseDiskButton, TRUE);
		SetWindowText(hStatus, L"Выберите вид тестирования.");
		return;
	}
	auto endTime2500KBRandom = std::chrono::high_resolution_clock::now();
	auto duration2500KBRandom = std::chrono::duration_cast<std::chrono::milliseconds>(endTime2500KBRandom - startTime2500KBRandom).count();
	double speedWrite2500KBRandom = (dataSize2500KBRandom / 1024.0 / 1024.0) / (duration2500KBRandom / 1000.0);

	//25000KB RandomWriteTest
	std::string data25000KBRandom(dataSize25000KBRandom, 'A');
	std::srand(static_cast<unsigned int>(std::time(0)));
	size_t arraySize25000KBRandom = (dataSize25000KBRandom / blockSize) + 1;
	std::vector<bool> writtenPositions25000KBRandom(arraySize25000KBRandom, false);
	auto startTime25000KBRandom = std::chrono::high_resolution_clock::now();
	std::ofstream outFile25000KBRandom(filePath25000KBRandom, std::ios::binary | std::ios::out);
	if (outFile25000KBRandom.is_open()) {
		for (size_t i = 0; i < dataSize25000KBRandom; i += blockSize) {
			size_t index = 0;
			size_t writePosition;
			do {
				index = std::rand() % arraySize25000KBRandom;
				writePosition = index * blockSize;
			} while (writtenPositions25000KBRandom[index]);
			size_t writeSize = static_cast<size_t>((blockSize < (dataSize25000KBRandom - writePosition)) ? blockSize : (dataSize25000KBRandom - writePosition));
			outFile25000KBRandom.seekp(writePosition, std::ios::beg);
			outFile25000KBRandom.write(data25000KBRandom.data() + i, writeSize);
			writtenPositions25000KBRandom[index] = true;
		}
		outFile25000KBRandom.close();
	}
	else {
		MessageBox(NULL, L"Ошибка открытия флеш-накопителя (25000 КБ Тест Случайной Записи).", L"Предупреждение", MB_OK | MB_ICONERROR);
		if (IsSequentialReadAvailable) {
			EnableWindow(hStartReadTestSequentialButton, TRUE);
		}
		EnableWindow(hStartWriteTestSequentialButton, TRUE);
		EnableWindow(hStartReadTestRandomButton, FALSE);
		EnableWindow(hStartWriteTestRandomButton, TRUE);
		EnableWindow(hChosseDiskButton, TRUE);
		SetWindowText(hStatus, L"Выберите вид тестирования.");
		return;
	}
	auto endTime25000KBRandom = std::chrono::high_resolution_clock::now();
	auto duration25000KBRandom = std::chrono::duration_cast<std::chrono::milliseconds>(endTime25000KBRandom - startTime25000KBRandom).count();
	double speedWrite25000KBRandom = (dataSize25000KBRandom / 1024.0 / 1024.0) / (duration25000KBRandom / 1000.0);

	std::wstring Output250KBP1Random = L"Длительность случайной записи(250КБ):\n" + std::to_wstring((duration250KBRandom / 1000.0)) + L" секунд \n";
	std::wstring Output250KBP2Random = L"Средняя скорость случайной записи(250КБ):\n" + std::to_wstring(speedWrite250KBRandom) + L" МБ/c \n\n";
	std::wstring Output2500KBP1Random = L"Длительность случайной записи(2500КБ):\n" + std::to_wstring((duration2500KBRandom / 1000.0)) + L" секунд \n";
	std::wstring Output2500KBP2Random = L"Средняя скорость случайной записи(2500КБ):\n" + std::to_wstring(speedWrite2500KBRandom) + L" МБ/c \n\n";
	std::wstring Output25000KBP1Random = L"Длительность случайной записи(25000КБ):\n" + std::to_wstring((duration25000KBRandom / 1000.0)) + L" секунд \n";
	std::wstring Output25000KBP2Random = L"Средняя скорость случайной записи(25000КБ):\n" + std::to_wstring(speedWrite25000KBRandom) + L" МБ/c \n\n";
	std::wstring WriteTestOutput = Output250KBP1Random + Output250KBP2Random + Output2500KBP1Random + Output2500KBP2Random + Output25000KBP1Random + Output25000KBP2Random;
	MessageBox(hwnd, WriteTestOutput.c_str(), L"Результат теста случайной записи", MB_OK | MB_ICONINFORMATION);
	IsRandomReadAvailable = true;
	if (IsSequentialReadAvailable) {
		EnableWindow(hStartReadTestSequentialButton, TRUE);
	}
	EnableWindow(hStartWriteTestSequentialButton, TRUE);
	EnableWindow(hStartWriteTestRandomButton, TRUE);
	EnableWindow(hStartReadTestRandomButton, TRUE);
	EnableWindow(hChosseDiskButton, TRUE);
	SetWindowText(hStatus, L"Выберите вид тестирования.");

}

void ReadTestSequential() {
    int result = MessageBox(
        NULL,
        L"Для корректных результатов тестирования чтения, пока это окно открыто, достаньте из порта USB и вставьте обратно флеш-накопитель, на котором до этого происходило тестирование записи. Вы совершили указанные действия?",
        L"Примечание",
        MB_YESNO | MB_ICONQUESTION
    );
    if (result == IDYES) {
		MessageBox(NULL, L"После завершения теста или при ошибке тестирования будут удалены соответствующие файлы ('testfile25MBSequential.bin', 'testfile250MBSequential.bin', 'testfile2500MBSequential.bin') .", L"Предупреждение", MB_OK | MB_ICONINFORMATION);
        std::wstring StatusText = L"Идёт тестирование последовательного чтения, ожидайте результатов.";
        SetWindowText(hStatus, StatusText.c_str());
		EnableWindow(hStartWriteTestSequentialButton, FALSE);
		EnableWindow(hStartReadTestSequentialButton, FALSE);
		EnableWindow(hStartWriteTestRandomButton, FALSE);
		EnableWindow(hStartReadTestRandomButton, FALSE);
		EnableWindow(hChosseDiskButton, FALSE);
        std::wstring fileName25MBSequential = L"testfile25MBSequential.bin";
        std::wstring fileName250MBSequential = L"testfile250MBSequential.bin";
        std::wstring fileName2500MBSequential = L"testfile2500MBSequential.bin";
        std::wstring filePath25MBSequential = currentPath + fileName25MBSequential;
        std::wstring filePath250MBSequential = currentPath + fileName250MBSequential;
        std::wstring filePath2500MBSequential = currentPath + fileName2500MBSequential;
        const int dataSize25MBSequential = 25000000;
        const int dataSize250MBSequential = 250000000;
        const long long int dataSize2500MBSequential = 2500000000;

        //25MB Read Test
        std::vector<char> buffer25MBSequential(dataSize25MBSequential);
        auto startTime25MBSequential = std::chrono::high_resolution_clock::now();
        std::ifstream inFile25MBSequential(filePath25MBSequential, std::ios::binary);
        if (inFile25MBSequential.is_open()) {
            inFile25MBSequential.read(buffer25MBSequential.data(), dataSize25MBSequential);
            inFile25MBSequential.close();
        }
        else {
            MessageBox(NULL, L"Ошибка открытия флеш-накопителя (25 МБ Тест Последовательного Чтения).", L"Предупреждение", MB_OK | MB_ICONERROR);
			if (IsRandomReadAvailable) {
				EnableWindow(hStartReadTestRandomButton, TRUE);
			}
			EnableWindow(hStartWriteTestSequentialButton, TRUE);
			EnableWindow(hStartWriteTestRandomButton, TRUE);
			IsSequentialReadAvailable = false;
			EnableWindow(hStartReadTestSequentialButton, FALSE);
			EnableWindow(hChosseDiskButton, TRUE);
			DeleteTestFiles(filePath25MBSequential, filePath250MBSequential, filePath2500MBSequential);
			SetWindowText(hStatus, L"Выберите вид тестирования.");
            return;
        }
        auto endTime25MBSequential = std::chrono::high_resolution_clock::now();
        auto duration25MBSequential = std::chrono::duration_cast<std::chrono::milliseconds>(endTime25MBSequential - startTime25MBSequential).count();
        double speedRead25MBSequential = (dataSize25MBSequential / 1024.0 / 1024.0) / (duration25MBSequential / 1000.0);

        //250MB Read Test
        std::vector<char> buffer250MBSequential(dataSize250MBSequential);
        auto startTime250MBSequential = std::chrono::high_resolution_clock::now();
        std::ifstream inFile250MBSequential(filePath250MBSequential, std::ios::binary);
        if (inFile250MBSequential.is_open()) {
            inFile250MBSequential.read(buffer250MBSequential.data(), dataSize250MBSequential);
            inFile250MBSequential.close();
        }
        else {
            MessageBox(NULL, L"Ошибка открытия флеш-накопителя (250 МБ Тест Последовательного Чтения).", L"Предупреждение", MB_OK | MB_ICONERROR);
			if (IsRandomReadAvailable) {
				EnableWindow(hStartReadTestRandomButton, TRUE);
			}
			EnableWindow(hStartWriteTestSequentialButton, TRUE);
			EnableWindow(hStartWriteTestRandomButton, TRUE);
			IsSequentialReadAvailable = false;
			EnableWindow(hStartReadTestSequentialButton, FALSE);
			EnableWindow(hChosseDiskButton, TRUE);
			DeleteTestFiles(filePath25MBSequential, filePath250MBSequential, filePath2500MBSequential);
			SetWindowText(hStatus, L"Выберите вид тестирования.");
			return;
        }
        auto endTime250MBSequential = std::chrono::high_resolution_clock::now();
        auto duration250MBSequential = std::chrono::duration_cast<std::chrono::milliseconds>(endTime250MBSequential - startTime250MBSequential).count();
        double speedRead250MBSequential = (dataSize250MBSequential / 1024.0 / 1024.0) / (duration250MBSequential / 1000.0);

        //2500MB Read Test
        std::vector<char> buffer2500MBSequential(dataSize2500MBSequential);
        auto startTime2500MBSequential = std::chrono::high_resolution_clock::now();
        std::ifstream inFile2500MBSequential(filePath2500MBSequential, std::ios::binary);
        if (inFile2500MBSequential.is_open()) {
            inFile2500MBSequential.read(buffer2500MBSequential.data(), dataSize2500MBSequential);
            inFile2500MBSequential.close();
        }
        else {
            MessageBox(NULL, L"Ошибка открытия флеш-накопителя (2500 МБ Тест Последовательного Чтения).", L"Предупреждение", MB_OK | MB_ICONERROR);
			if (IsRandomReadAvailable) {
				EnableWindow(hStartReadTestRandomButton, TRUE);
			}
			EnableWindow(hStartWriteTestSequentialButton, TRUE);
			EnableWindow(hStartWriteTestRandomButton, TRUE);
			IsSequentialReadAvailable = false;
			EnableWindow(hStartReadTestSequentialButton, FALSE);
			EnableWindow(hChosseDiskButton, TRUE);
			DeleteTestFiles(filePath25MBSequential, filePath250MBSequential, filePath2500MBSequential);
			SetWindowText(hStatus, L"Выберите вид тестирования.");
			return;
        }
        auto endTime2500MBSequential = std::chrono::high_resolution_clock::now();
        auto duration2500MBSequential = std::chrono::duration_cast<std::chrono::milliseconds>(endTime2500MBSequential - startTime2500MBSequential).count();
        double speedRead2500MBSequential = (dataSize2500MBSequential / 1024.0 / 1024.0) / (duration2500MBSequential / 1000.0);

        std::wstring Output25MBP1Sequential = L"Длительность последовательного чтения(25МБ):\n" + std::to_wstring((duration25MBSequential / 1000.0)) + L" секунд \n";
        std::wstring Output25MBP2Sequential = L"Средняя скорость последовательного чтения(25МБ):\n" + std::to_wstring(speedRead25MBSequential) + L" МБ/c \n\n";
        std::wstring Output250MBP1Sequential = L"Длительность последовательного чтения(250МБ):\n" + std::to_wstring((duration250MBSequential / 1000.0)) + L" секунд \n";
        std::wstring Output250MBP2Sequential = L"Средняя скорость последовательного чтения(250МБ):\n" + std::to_wstring(speedRead250MBSequential) + L" МБ/c \n\n";
        std::wstring Output2500MBP1Sequential = L"Длительность последовательного чтения(2500МБ):\n" + std::to_wstring((duration2500MBSequential / 1000.0)) + L" секунд \n";
        std::wstring Output2500MBP2Sequential = L"Средняя скорость последовательного чтения(2500МБ):\n" + std::to_wstring(speedRead2500MBSequential) + L" МБ/c \n\n";
        std::wstring WriteTestOutput = Output25MBP1Sequential + Output25MBP2Sequential + Output250MBP1Sequential + Output250MBP2Sequential + Output2500MBP1Sequential + Output2500MBP2Sequential;
        MessageBox(hwnd, WriteTestOutput.c_str(), L"Результат теста последовательного чтения", MB_OK | MB_ICONINFORMATION);
		SetWindowText(hStatus, L"Идёт удаление тестовых файлов. Ожидайте.");
		DeleteTestFiles(filePath25MBSequential, filePath250MBSequential, filePath2500MBSequential);
		if (IsRandomReadAvailable) {
			EnableWindow(hStartReadTestRandomButton, TRUE);
		}
        EnableWindow(hStartWriteTestSequentialButton, TRUE);
		EnableWindow(hStartWriteTestRandomButton, TRUE);
		IsSequentialReadAvailable = false;
        EnableWindow(hStartReadTestSequentialButton, FALSE);
		EnableWindow(hChosseDiskButton, TRUE);
        SetWindowText(hStatus, L"Выберите вид тестирования.");

    }
    else {
        return;
    }
}

void ReadTestRandom() {
	int result = MessageBox(
		NULL,
		L"Для корректных результатов тестирования чтения, пока это окно открыто, достаньте из порта USB и вставьте обратно флеш-накопитель, на котором до этого происходило тестирование записи. Вы совершили указанные действия?",
		L"Примечание",
		MB_YESNO | MB_ICONQUESTION
	);
	if (result == IDYES) {
		MessageBox(NULL, L"После завершения теста или при ошибке тестирования будут удалены соответствующие файлы ('testfile250KBRandom.bin', 'testfile2500KBRandom.bin', 'testfile25000KBRandom.bin') .", L"Предупреждение", MB_OK | MB_ICONINFORMATION);
		std::wstring StatusText = L"Идёт тестирование случайного чтения, ожидайте результатов.";
		SetWindowText(hStatus, StatusText.c_str());
		EnableWindow(hStartWriteTestSequentialButton, FALSE);
		EnableWindow(hStartReadTestSequentialButton, FALSE);
		EnableWindow(hStartWriteTestRandomButton, FALSE);
		EnableWindow(hStartReadTestRandomButton, FALSE);
		EnableWindow(hChosseDiskButton, FALSE);
		std::wstring fileName250KBRandom = L"testfile250KBRandom.bin";
		std::wstring fileName2500KBRandom = L"testfile2500KBRandom.bin";
		std::wstring fileName25000KBRandom = L"testfile25000KBRandom.bin";
		std::wstring filePath250KBRandom = currentPath + fileName250KBRandom;
		std::wstring filePath2500KBRandom = currentPath + fileName2500KBRandom;
		std::wstring filePath25000KBRandom = currentPath + fileName25000KBRandom;
		const int dataSize250KBRandom = 250000;
		const int dataSize2500KBRandom = 2500000;
		const int dataSize25000KBRandom = 25000000;
		const size_t blockSize = 4096;

		//250KBRandomReadTest
		std::srand(static_cast<unsigned int>(std::time(0)));
		size_t arraySize250KBRandom = (dataSize250KBRandom / blockSize) + 1;
		std::vector<bool> readPositions250KBRandom(arraySize250KBRandom, false);
		auto startTime250KBRandom = std::chrono::high_resolution_clock::now();
		std::ifstream inFile250KBRandom(filePath250KBRandom, std::ios::binary | std::ios::in);
		if (inFile250KBRandom.is_open()) {
			for (size_t i = 0; i < dataSize250KBRandom; i += blockSize) {
				size_t index = 0;
				size_t readPosition;
				do {
					index = std::rand() % arraySize250KBRandom;
					readPosition = index * blockSize;
				} while (readPositions250KBRandom[index]);
				size_t readSize = static_cast<size_t>((blockSize < (dataSize250KBRandom - readPosition)) ? blockSize : (dataSize250KBRandom - readPosition));
				inFile250KBRandom.seekg(readPosition, std::ios::beg);
				std::vector<char> buffer(readSize);
				inFile250KBRandom.read(buffer.data(), readSize);
				readPositions250KBRandom[index] = true;
			}
			inFile250KBRandom.close();
		}
		else {
			MessageBox(NULL, L"Ошибка открытия флеш-накопителя (250 КБ Тест Случайного Чтения).", L"Предупреждение", MB_OK | MB_ICONERROR);
			if (IsSequentialReadAvailable) {
				EnableWindow(hStartReadTestSequentialButton, TRUE);
			}
			EnableWindow(hStartWriteTestSequentialButton, TRUE);
			EnableWindow(hStartWriteTestRandomButton, TRUE);
			IsRandomReadAvailable = false;
			EnableWindow(hStartReadTestRandomButton, FALSE);
			EnableWindow(hChosseDiskButton, TRUE);
			DeleteTestFiles(filePath250KBRandom, filePath2500KBRandom, filePath25000KBRandom);
			SetWindowText(hStatus, L"Выберите вид тестирования.");
			return;
		}
		auto endTime250KBRandom = std::chrono::high_resolution_clock::now();
		auto duration250KBRandom = std::chrono::duration_cast<std::chrono::milliseconds>(endTime250KBRandom - startTime250KBRandom).count();
		double speedRead250KBRandom = (dataSize250KBRandom / 1024.0 / 1024.0) / (duration250KBRandom / 1000.0);

		//2500KB RandomReadTest
		std::srand(static_cast<unsigned int>(std::time(0)));
		size_t arraySize2500KBRandom = (dataSize2500KBRandom / blockSize) + 1;
		std::vector<bool> readPositions2500KBRandom(arraySize2500KBRandom, false);
		auto startTime2500KBRandom = std::chrono::high_resolution_clock::now();
		std::ifstream inFile2500KBRandom(filePath2500KBRandom, std::ios::binary | std::ios::in);
		if (inFile2500KBRandom.is_open()) {
			for (size_t i = 0; i < dataSize2500KBRandom; i += blockSize) {
				size_t index = 0;
				size_t readPosition;
				do {
					index = std::rand() % arraySize2500KBRandom;
					readPosition = index * blockSize;
				} while (readPositions2500KBRandom[index]);
				size_t readSize = static_cast<size_t>((blockSize < (dataSize2500KBRandom - readPosition)) ? blockSize : (dataSize2500KBRandom - readPosition));
				inFile2500KBRandom.seekg(readPosition, std::ios::beg);
				std::vector<char> buffer(readSize);
				inFile2500KBRandom.read(buffer.data(), readSize);
				readPositions2500KBRandom[index] = true;
			}
			inFile2500KBRandom.close();
		}
		else {
			MessageBox(NULL, L"Ошибка открытия флеш-накопителя (2500 КБ Тест Случайного Чтения).", L"Предупреждение", MB_OK | MB_ICONERROR);
			if (IsSequentialReadAvailable) {
				EnableWindow(hStartReadTestSequentialButton, TRUE);
			}
			EnableWindow(hStartWriteTestSequentialButton, TRUE);
			EnableWindow(hStartWriteTestRandomButton, TRUE);
			IsRandomReadAvailable = false;
			EnableWindow(hStartReadTestRandomButton, FALSE);
			EnableWindow(hChosseDiskButton, TRUE);
			DeleteTestFiles(filePath250KBRandom, filePath2500KBRandom, filePath25000KBRandom);
			SetWindowText(hStatus, L"Выберите вид тестирования.");
			return;
		}
		auto endTime2500KBRandom = std::chrono::high_resolution_clock::now();
		auto duration2500KBRandom = std::chrono::duration_cast<std::chrono::milliseconds>(endTime2500KBRandom - startTime2500KBRandom).count();
		double speedRead2500KBRandom = (dataSize2500KBRandom / 1024.0 / 1024.0) / (duration2500KBRandom / 1000.0);

		//25000KB RandomReadTest
		std::srand(static_cast<unsigned int>(std::time(0)));
		size_t arraySize25000KBRandom = (dataSize25000KBRandom / blockSize) + 1;
		std::vector<bool> readPositions25000KBRandom(arraySize25000KBRandom, false);
		auto startTime25000KBRandom = std::chrono::high_resolution_clock::now();
		std::ifstream inFile25000KBRandom(filePath25000KBRandom, std::ios::binary | std::ios::in);
		if (inFile25000KBRandom.is_open()) {
			for (size_t i = 0; i < dataSize25000KBRandom; i += blockSize) {
				size_t index = 0;
				size_t readPosition;
				do {
					index = std::rand() % arraySize25000KBRandom;
					readPosition = index * blockSize;
				} while (readPositions25000KBRandom[index]);
				size_t readSize = static_cast<size_t>((blockSize < (dataSize25000KBRandom - readPosition)) ? blockSize : (dataSize25000KBRandom - readPosition));
				inFile25000KBRandom.seekg(readPosition, std::ios::beg);
				std::vector<char> buffer(readSize);
				inFile25000KBRandom.read(buffer.data(), readSize);
				readPositions25000KBRandom[index] = true;
			}
			inFile25000KBRandom.close();
		}
		else {
			MessageBox(NULL, L"Ошибка открытия флеш-накопителя (25000 КБ Тест Случайного Чтения).", L"Предупреждение", MB_OK | MB_ICONERROR);
			if (IsSequentialReadAvailable) {
				EnableWindow(hStartReadTestSequentialButton, TRUE);
			}
			EnableWindow(hStartWriteTestSequentialButton, TRUE);
			EnableWindow(hStartWriteTestRandomButton, TRUE);
			IsRandomReadAvailable = false;
			EnableWindow(hStartReadTestRandomButton, FALSE);
			EnableWindow(hChosseDiskButton, TRUE);
			DeleteTestFiles(filePath250KBRandom, filePath2500KBRandom, filePath25000KBRandom);
			SetWindowText(hStatus, L"Выберите вид тестирования.");
			return;
		}
		auto endTime25000KBRandom = std::chrono::high_resolution_clock::now();
		auto duration25000KBRandom = std::chrono::duration_cast<std::chrono::milliseconds>(endTime25000KBRandom - startTime25000KBRandom).count();
		double speedRead25000KBRandom = (dataSize25000KBRandom / 1024.0 / 1024.0) / (duration25000KBRandom / 1000.0);
		
		std::wstring Output250KBP1Random = L"Длительность случайного чтения(250КБ):\n" + std::to_wstring((duration250KBRandom / 1000.0)) + L" секунд \n";
		std::wstring Output250KBP2Random = L"Средняя скорость случайного чтения(250КБ):\n" + std::to_wstring(speedRead250KBRandom) + L" МБ/c \n\n";
		std::wstring Output2500KBP1Random = L"Длительность случайного чтения(2500КБ):\n" + std::to_wstring((duration2500KBRandom / 1000.0)) + L" секунд \n";
		std::wstring Output2500KBP2Random = L"Средняя скорость случайного чтения(2500КБ):\n" + std::to_wstring(speedRead2500KBRandom) + L" МБ/c \n\n";
		std::wstring Output25000KBP1Random = L"Длительность случайного чтения(25000КБ):\n" + std::to_wstring((duration25000KBRandom / 1000.0)) + L" секунд \n";
		std::wstring Output25000KBP2Random = L"Средняя скорость случайного чтения(25000КБ):\n" + std::to_wstring(speedRead25000KBRandom) + L" МБ/c \n\n";
		std::wstring WriteTestOutput = Output250KBP1Random + Output250KBP2Random + Output2500KBP1Random + Output2500KBP2Random + Output25000KBP1Random + Output25000KBP2Random;
		MessageBox(hwnd, WriteTestOutput.c_str(), L"Результат теста случайного чтения", MB_OK | MB_ICONINFORMATION);
		SetWindowText(hStatus, L"Идёт удаление тестовых файлов. Ожидайте.");
		DeleteTestFiles(filePath250KBRandom, filePath2500KBRandom, filePath25000KBRandom);
		if (IsSequentialReadAvailable) {
			EnableWindow(hStartReadTestSequentialButton, TRUE);
		}
		EnableWindow(hStartWriteTestSequentialButton, TRUE);
		EnableWindow(hStartWriteTestRandomButton, TRUE);
		IsRandomReadAvailable = false;
		EnableWindow(hStartReadTestRandomButton, FALSE);
		EnableWindow(hChosseDiskButton, TRUE);
		SetWindowText(hStatus, L"Выберите вид тестирования.");
	}
	else {
		return;
	}
}

void DeleteTestFiles(std::wstring FilePath1, std::wstring FilePath2, std::wstring FilePath3) {
	if (DeleteFile(FilePath1.c_str())) {
		if (DeleteFile(FilePath2.c_str())) {
			if (DeleteFile(FilePath3.c_str())) {
				MessageBox(NULL, L"Удалены соответствующие файлы, принадлежащие к текущему тесту.", L"Предупреждение", MB_OK | MB_ICONINFORMATION);
			}
			else {
				MessageBox(NULL, L"Ошибка удаления соответствующих текущему тесту файлов.", L"Предупреждение", MB_OK | MB_ICONERROR);
			}
		}
		else {
			MessageBox(NULL, L"Ошибка удаления соответствующих текущему тесту файлов.", L"Предупреждение", MB_OK | MB_ICONERROR);
		}
	}
	else {
		MessageBox(NULL, L"Ошибка удаления соответствующих текущему тесту файлов.", L"Предупреждение", MB_OK | MB_ICONERROR);
	}
}

