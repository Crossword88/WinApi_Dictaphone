#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#define _CRT_SECURE_NO_WARNINGS
#include <Mmsystem.h>
#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#define _CRT_SECURE_NO_WARNINGS
#include <thread>

#pragma comment(lib, "winmm.lib")

const int sampleRateF = 44100;
int durationSeconds = 0;
std::vector <short int> rec;	//all sounds

class Wav
{
public:
	char chunkId[4];		//"RIFF"
	uint32_t chunkSize;		//File size -8
	char format[4];			//"WAVE"
	char subChunk1Id[4];	//"fmt"
	uint32_t subChunk1Size;	//size format pcm
	uint16_t audioFormat;	//Format audio
	uint16_t numChannels;	//1 - Mono, 2 - Stereo
	uint32_t sampleRate;	//Frequency of discredit
	uint32_t byteRate;		//Bytes per second
	uint16_t blockAlign;	//Bytes for one sample
	uint16_t bitsPerSample;	//bites in sample, sound depth
	char subChunk2Id[4];	//"date"
	uint32_t subChunk2Size; //Data bytes

public:
	Wav() {
		strncpy(chunkId, "RIFF", 4);
		chunkSize = sizeof(Wav) - 8 + sampleRateF * durationSeconds * sizeof(int16_t);
		strncpy(format, "WAVE", 4);
		strncpy(subChunk1Id, "fmt ", 4);
		subChunk1Size = 16;
		audioFormat = 1;
		numChannels = 2;
		sampleRate = sampleRateF;
		bitsPerSample = sizeof(int16_t) * 8;
		byteRate = sampleRateF * numChannels * bitsPerSample / 8;
		blockAlign = numChannels * bitsPerSample / 8;
		strncpy(subChunk2Id, "data", 4);
		subChunk2Size = sampleRateF * durationSeconds * sizeof(int16_t);
	}
};

std::ofstream fout;

void writeF(const int NUMPTS)
{
	//Создаю объект структуры (В моём случае уже класса), и вызываю конструктор
	Wav wav;

	fout.open("SOME.wav", std::ios::binary);
	fout.write(reinterpret_cast<char*>(&wav), sizeof(Wav));

	for (int i = 0; unsigned(i) < rec.size(); i++)
		fout.write((char*)(&rec[i]), sizeof(short int));
	fout.close();

}

void recording(short int* waveIn, const int NUMPTS)
{
	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;     // simple, uncompressed format
	wf.nChannels = 2;                    //  1=mono, 2=stereo
	wf.wBitsPerSample = 16;              //  16 for high quality, 8 for telephone-grade
	wf.nSamplesPerSec = sampleRateF;
	wf.nAvgBytesPerSec = sampleRateF * wf.nChannels * wf.wBitsPerSample / 8;
	wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
	wf.cbSize = 0;

	HWAVEIN hwi = { 0 };

	WAVEHDR waveInHdr = { 0 };

	waveInHdr.lpData = (LPSTR)waveIn;
	waveInHdr.dwBufferLength = NUMPTS * 2;
	waveInHdr.dwBytesRecorded = 0;
	waveInHdr.dwUser = 0L;
	waveInHdr.dwFlags = 0L;
	waveInHdr.dwLoops = 0L;


	for (; true;)
	{
		waveInOpen(&hwi, WAVE_MAPPER, &wf, 0, 0, WAVE_FORMAT_DIRECT);

		waveInPrepareHeader(hwi, &waveInHdr, sizeof(WAVEHDR));
		// добавляем буфер данных в очередь
		waveInAddBuffer(hwi, &waveInHdr, sizeof(WAVEHDR));
		// начинаем запись звука
		waveInStart(hwi);
		waveInClose(hwi);
		Sleep(1000);

		for (int i = 0; i < NUMPTS; i++)
			rec.push_back(waveIn[i]);

		durationSeconds += 1 * 2;	// /2
	}
}

int	CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR szCmdLine, int nCmdShow)
{
	MSG msg{};
	HWND hwnd{};
	WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadCursor(nullptr, IDI_APPLICATION);
	wc.hInstance = hInstance;

	const int NUMPTS = sampleRateF * 2 * (1);	//last one - seconds

	wc.lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			HWND hButton = CreateWindow(
				L"BUTTON",
				L"Rec",
				WS_CHILD | WS_VISIBLE,
				100, 100, 300, 30, hWnd, reinterpret_cast<HMENU>(1337), nullptr, nullptr
			);

			HWND hButton1 = CreateWindow(
				L"BUTTON",
				L"Stop",
				WS_CHILD | WS_VISIBLE,
				100, 300, 300, 30, hWnd, reinterpret_cast<HMENU>(0), nullptr, nullptr
			);
		}
		return 0;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case 1337:
			{
				short int* waveIn = new short int[NUMPTS];   //suda suka
				std::thread th(recording, waveIn, NUMPTS);
				th.detach();
			}
			break;
			case 0:
			{
				writeF(NUMPTS);
			}
			break;
			}
		}
		return 0;

		case WM_DESTROY:
		{
			PostQuitMessage(EXIT_SUCCESS);
		}
		break;
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	};
	wc.lpszClassName = L"MyAppClass";
	wc.lpszMenuName = nullptr;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wc))
		return EXIT_FAILURE;

	hwnd = CreateWindow(wc.lpszClassName, L"Загаловок!", WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, nullptr, nullptr, wc.hInstance, nullptr);
	if (hwnd == INVALID_HANDLE_VALUE)
		return EXIT_FAILURE;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}