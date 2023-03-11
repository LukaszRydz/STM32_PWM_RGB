/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim4_ch1;
DMA_HandleTypeDef hdma_tim4_ch2;
DMA_HandleTypeDef hdma_tim4_ch3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define RX_BF_length 512												// Max długość buforu odczytu
#define TX_BF_length 1512												// Max długość buforu transmisji
uint8_t RX_BF[RX_BF_length];											// Deklaracja zmiennej dla bufora odczytu
uint8_t TX_BF[TX_BF_length];											// Deklaracja zmiennej dla bufora transmisji

// Indeksy buforów, pobieranych bezpośrednio z pamięci (volatile)

__IO int RX_IDX_EMPTY = 0;
__IO int RX__IDX_BUSY = 0;
__IO int TX_IDX_EMPTY = 0;
__IO int TX_IDX_BUSY = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

// Prototypy funkcji
int getSection(int8_t dlugoscSekcji, int8_t sekcja, int16_t globalIndex, char oczekiwanyZnak);	// Funkcja pobierająca sekcjie ramki oddzielone separatorami
int isNUM(int8_t numberLength, int16_t maxValue, int8_t minValue ,int8_t section);				// Funkcja sprawdzajaca czy znaki są z systemu dziesiętnego oraz konwertująca STR -> INT
void frameRecall(int ID, int sectionID);		// Komunikaty zwrotne
void searchFrame();								// Poszukiwanie ramki / kolekcjonowanie
void readFrame();								// Wydzielanie ramki na sekcjie
void getCommand();								// Wydzielanie danych na komende i argument
void getNumArgument();							// Zamiana argumentu String -> Int
void executeCommand();							// Wykonanie komendy
void checkCRC();								// Sprawdzenie CRC
void checkData();								// Sprawdzenie czy sekcja danych nie zawiera błędów

// Prototypy komend
void BRGHT(int stockBrigthness);				// Zmiana jasności diody
void STCLR();									// Zmiana koloru poprzez wpisanie 3 zmiennych
void COLOR();									// Zmiana koloru poprzez wpisanie nazwy koloru
void EFFCT();									// Zmiania efektu diody
void RGBST();									// Wł/Wył diody
void GETST();									// Pobieranie infomracji o diodzie stan/jasność/efekt

// Protorypy funkcji generujących efekty.
void generatePoliceLights(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues);
void generateDiscoEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues);
void generateBreathingEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues);
void generateHeartBeatEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues);
void generateBlinkingEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues);
void generaterandomBlinkingEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues);

// Prototyp funkcji oraz zmiennej do srpawdzania CRC
unsigned int crc(char *data, unsigned int length);

// Tablica wartości CRC, kod generujący tą tablice jest w pliku help.txt
unsigned int crc_table[256] = {0, 49345, 49537, 320, 49921, 960, 640, 49729, 50689, 1728, 1920, 51009, 1280, 50625, 50305, 1088, 52225,
		3264, 3456, 52545, 3840, 53185, 52865, 3648, 2560, 51905, 52097, 2880, 51457, 2496, 2176, 51265, 55297, 6336, 6528, 55617,
		6912, 56257, 55937, 6720, 7680, 57025, 57217, 8000, 56577, 7616, 7296, 56385, 5120, 54465, 54657, 5440, 55041, 6080, 5760,
		54849, 53761, 4800, 4992, 54081, 4352, 53697, 53377, 4160, 61441, 12480, 12672, 61761, 13056, 62401, 62081, 12864, 13824,
		63169, 63361, 14144, 62721, 13760, 13440, 62529, 15360, 64705, 64897, 15680, 65281, 16320, 16000, 65089, 64001, 15040, 15232,
		64321, 14592, 63937, 63617, 14400, 10240, 59585, 59777, 10560, 60161, 11200, 10880, 59969, 60929, 11968, 12160, 61249, 11520,
		60865, 60545, 11328, 58369, 9408, 9600, 58689, 9984, 59329, 59009, 9792, 8704, 58049, 58241, 9024, 57601, 8640, 8320, 57409,
		40961, 24768, 24960, 41281, 25344, 41921, 41601, 25152, 26112, 42689, 42881, 26432, 42241, 26048, 25728, 42049, 27648, 44225,
		44417, 27968, 44801, 28608, 28288, 44609, 43521, 27328, 27520, 43841, 26880, 43457, 43137, 26688, 30720, 47297, 47489, 31040,
		47873, 31680, 31360, 47681, 48641, 32448, 32640, 48961, 32000, 48577, 48257, 31808, 46081, 29888, 30080, 46401, 30464, 47041,
		46721, 30272, 29184, 45761, 45953, 29504, 45313, 29120, 28800, 45121, 20480, 37057, 37249, 20800, 37633, 21440, 21120, 37441,
		38401, 22208, 22400, 38721, 21760, 38337, 38017, 21568, 39937, 23744, 23936, 40257, 24320, 40897, 40577, 24128, 23040, 39617,
		39809, 23360, 39169, 22976, 22656, 38977, 34817, 18624, 18816, 35137, 19200, 35777, 35457, 19008, 19968, 36545, 36737, 20288,
		36097, 19904, 19584, 35905, 17408, 33985, 34177, 17728, 34561, 18368, 18048, 34369, 33281, 17088, 17280, 33601, 16640, 33217, 32897, 16448, };


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


// __________________________________ USART FUNCTIONS BEGIN __________________________________



uint8_t USART_kbhit()																			// Funkcja sprawdza czy jest cos w buforze,
{																								// jezeli nie ma zwraca false(0) inaczej true(1)
	if (RX_IDX_EMPTY == RX__IDX_BUSY) return 0;
	else return 1;
}

int16_t USART_getChar()																			// Funkcja pobierająca znak z bufora
{
	int16_t charTMP;
	if (RX_IDX_EMPTY != RX__IDX_BUSY)
	{
		charTMP = RX_BF[RX__IDX_BUSY];
		RX__IDX_BUSY++;
		if (RX__IDX_BUSY >= RX_BF_length) RX__IDX_BUSY = 0;
		return charTMP;
	}
	else return -1;
}

void USART_fsend(char* format, ...)														// Wysyłanie do terminala
{
	char tmp_rs[RX_BF_length];															// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Jezeli tu damy za małą wartość program sie wysypie???
	int i;
	__IO int idx;
	va_list arglist;																	// Zmienna do obsługi formatowania argumentów
	va_start(arglist,format);															// Umożliwia dostęp do zmiennych argumentów funkcji
	vsprintf(tmp_rs,format,arglist);													// Połączenie łańcucha argumentów i przypisanie do tmp_rs
	va_end(arglist);
	idx = TX_IDX_EMPTY;

	for(i = 0; i < strlen(tmp_rs) ; i++)
	{
		TX_BF[idx] = tmp_rs[i];															// Przepisanie znaku do bufora transmisji
		idx++;
		if (idx >= TX_BF_length) idx = 0;												// Zawinięcie bufora jeżeli indeks wyjdzie poza zakres
	}
	__disable_irq();																	// Wyłączenie zapytań przerwań

	if ((TX_IDX_EMPTY == TX_IDX_BUSY) && (__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TXE)==SET))
	{
		TX_IDX_EMPTY = idx;
		uint8_t tmp = TX_BF[TX_IDX_BUSY];												// Zmienna zawierająca znak do wysłania
		TX_IDX_BUSY++;
		if (TX_IDX_BUSY >= TX_BF_length) TX_IDX_BUSY = 0;
		HAL_UART_Transmit_IT(&huart2, &tmp, 1);											// Transmisja elementu
	} else TX_IDX_EMPTY = idx;
	__enable_irq();																		// Włączenie zapytań przerwań
}//fsend



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2)
	{
		if (TX_IDX_EMPTY != TX_IDX_BUSY)
		{
			uint8_t tmp = TX_BF[TX_IDX_BUSY];
			TX_IDX_BUSY++;
			if (TX_IDX_BUSY >= TX_BF_length) TX_IDX_BUSY = 0;
			HAL_UART_Transmit_IT(&huart2, &tmp, 1);
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2)
	{
		RX_IDX_EMPTY++;
		if (RX_IDX_EMPTY >= RX_BF_length)
		{
			RX_IDX_EMPTY = 0;
		}
		HAL_UART_Receive_IT(&huart2, &RX_BF[RX_IDX_EMPTY], 1);
	}
}

// __________________________________ USART FUNCTIONS END __________________________________


// ====================================================== SEARCH FRAME FUNCTION START ======================================================

#define MAX_SIZE 218											// Zdefiniowanie max. długości ramki
#define MIN_SIZE 28												// Zdefiniowanie min. długość ramki

int16_t frameLength = 0;										// Zmianna obliczająca długość ramki
int8_t detectionFlag = 0;										// Flaga detekcji ramki.
char frame[MAX_SIZE];											// Tablica ramki

void searchFrame(){

	// Oczekiwanie na znak rozpoczęcia
	int16_t character = USART_getChar();
	if (character < 0) return;

	USART_fsend("%c", character);							// Podobno nie powinno sie wyświetlać wysłanych znaków (? Zaliczenie ?)

	// Jeśli znaleziono znak rozpoczęcia rozpoczyna odczyt
	if (character == '@') {
	    frameLength = 0;
	    detectionFlag = 1;
	}

	if (detectionFlag == 1) {
	    frame[frameLength++] = character;
	    // Jeśli znaleziono znak końca ramki
	    if (character == '$') {
	        frame[frameLength] = '\0';
	        // Reset funkcji jeżeli ramka jest za krótka
	        if (frameLength < 28) {
	    		detectionFlag = 0;
	    		return;
	    	}
	        readFrame();
	        detectionFlag = 0;
	    }
	    // Jeśli ramka przekroczyła maksymalny rozmiar
	    if (frameLength > MAX_SIZE){
	        detectionFlag = 0;
	        return;
	    }
	}
}

// ====================================================== SEARCH FRAME FUNCTION END ======================================================

// ====================================================== READ FRAME FUNCTION START ======================================================

#define SENDER 1													// Odczyt nadawcy z ramki
#define RECEIVER 2													// Odczyt adresata z ramki
#define COMMAND_LENGTH 3											// Odczyt długości komendy
#define COMMAND 4													// Odczyt sekcji komendy
#define CHECK_SUM 5													// Odczyt sumy kontrolnej
#define EXECUTE 6													// Wykonanie komendy

#define startCharacter '@'											// Znak rozpoczęcia ramki
#define endCharacter '$'											// Znak końca ramki
#define separator ':'												// Separator
#define senAdr "PC1"												// ID nadawcy
#define recAdr "STM"												// ID odbiorcy

// Dane odczytane z ramki
char senderAdr[4];													// Tablica z danymi nadawcy ( 3 znaki + '\0' )
char receiverAdr[4];												// Tablica z danymi odbiorcy ( 3 znaki + '\0' )
int16_t maxCmdLen = 0;												// Długość komendy
char receivedData[MAX_SIZE];										// Tablica z przysłanymi danymi
char commandType[4];												// Tablica z danymi o typie komendy ( 3 znaki + '\0' )
char command[15];													// Tablica z komendą
char commandArg[MAX_SIZE];											// Tablica z danymi argumentu komendy
int32_t frameCRC = 0;												// Zmienna zapisująca sumę CRC podaną w ramce
unsigned int crc_value;												// Obliczone CRC przez program
int numArguments[3];												// Argumenty liczbowe

// Zmienne pomocnicze
char tmpArr[MAX_SIZE];												// Tablica pomocnicza przy obróbce danych
int errorFlag = 0;													// Flaga błędu. 0 - Oznacza ze nie ma błędu, 1 - Oznacza ze nie ma błędu.



/* 	 ******************************************************* TODO LIST *******************************************************

								   - Zmienić i sprawdzić czy DMA prawidłowo ndapisuje buffory w połowie i na końcu.
					* Po zmianie testy efektów nie wykazały żadnych błędów (przerwań w wypełnieniu / pominięcia nadpisania itp.)

						     - Sprwadzić poprawność warunku w searchFrame() sprawdzającego min. długość ramki.
						     	 	 	 	 * Prawidłowo program nie dopuszcza ramki do analizy

							- Zmiana sprawdzania poprawności długości sekcji danych z strlen na coś innego
						* Usunąłem ponieważ za sprawdzanie długość sekcji danych odpowiada funckja getSection

 	 ************************************************************************************************************************* */


int getSection(int8_t dlugoscSekcji, int8_t sekcjaID, int16_t globalIndex, char oczekiwanyZnak){
	// Pętla odpowiedzialna za pobieranie znaków do tablicy tymczasowej
	for (int localIndex = 0; localIndex < dlugoscSekcji + 1; localIndex++){
		if (errorFlag == 1) return 0;
		// Sprawdzenie czy w sekcji nie znajdują się separatory np. @PC:
		if (localIndex < dlugoscSekcji){
			// Sprawdzenie czy znak to '\0'
			if (frame[globalIndex] == '\0') frameRecall(1003, sekcjaID);
			else if (frame[globalIndex] == separator) frameRecall(1001, sekcjaID);
			else tmpArr[localIndex] = frame[globalIndex];
		} else {
			// Jeżeli w sekcji nie było separatorów sprawdza czy ':' lub '$' znajduje się za nią.
			if (frame[globalIndex] != oczekiwanyZnak) frameRecall(1002, sekcjaID);
			else tmpArr[localIndex] = '\0';
		}
		globalIndex++;
	}
	return globalIndex; // Zwraca wartość indeksu w celu odczytu dalszej części ramki
}

int isNUM(int8_t numberLength, int16_t maxValue, int8_t minValue, int8_t section){
	// Sprawdzenie czy znaki są liczbami systemu dziesiętnego

	for (int i = 0; i < numberLength; i++){
		if ((tmpArr[i] < '0' || tmpArr[i] > '9') && errorFlag == 0) frameRecall(2001, section);
	}
	if (errorFlag == 0){
		int number = atoi(tmpArr);
		// Sprawdzenie czy liczba mieści się w dopuszczalnym zakresie.
		if (number < minValue || number > maxValue) frameRecall(2002, section);
		else return number;
	}
} // Jeżeli nie było błędów funkcja zwraca liczbe typu INT

void readFrame(){
	int16_t globalIndex = 1;					// Indeks odczytu przysłanej ramki
	int8_t section = 0;							// Flaga sekcji
	errorFlag = 0;								// Zerowanie flagi błędu

	// Pętla wykona się tyle razy ile jest sekcji + sekcja anonimowa EXECUTE ( Tylko widoczna dla programu )
	for (section; section <= EXECUTE; section){
		// Sprawdzenie czy nie było błędu w poprzedniej sekcji.
		if (errorFlag == 0) section++;
		else return;

		switch (section){
		case SENDER:
			globalIndex = getSection(3, section, globalIndex, separator);
			if (errorFlag == 0) strcpy(senderAdr, tmpArr);
			// Sprawdzenie ID nadawcy
			if (strcmp(senderAdr, senAdr) != 0 && errorFlag == 0) frameRecall(4001, section);
			break;
		case RECEIVER:
			globalIndex = getSection(3, section, globalIndex, separator);
			if (errorFlag == 0) strcpy(receiverAdr, tmpArr);
			// Sprawdzenie ID adresata
			if (strcmp(receiverAdr, recAdr) != 0 && errorFlag == 0) frameRecall(4002, section);
			break;
		case COMMAND_LENGTH:
			globalIndex = getSection(3, section, globalIndex, separator);
			// Konwersja tablicy char -> int
			if (errorFlag == 0) maxCmdLen = isNUM(3, 200, 10, section);
			break;
		case COMMAND:
			globalIndex = getSection(maxCmdLen, section, globalIndex, separator);
			if (errorFlag == 0) strcpy(receivedData, tmpArr);
			break;
		case CHECK_SUM:
			globalIndex = getSection(3, section, globalIndex, endCharacter);
			if (errorFlag == 0) frameCRC = isNUM(3, 999, 0, section);
			break;
		case EXECUTE:
			// Sprawdzenie sumy kontrolnej
			checkCRC();
			// Sprawdzenie sekcji danych ( czy argument / dłgość komendy się zgadzają)
			if (errorFlag == 0) checkData();
			// Pobarnie komendy z sekcji danych
			if (errorFlag == 0) getCommand();
			// Próba wykonania komendy
			if (errorFlag == 0) executeCommand();
			break;
		}
	}
	return;
}

void checkCRC(){
	// Sprawdzenie CRC wynikowe z wysłanym w ramce
	crc_value = crc(receivedData, maxCmdLen) % 999;
	if(crc_value != frameCRC) frameRecall(4003, CHECK_SUM);
}

unsigned int crc(char *data, unsigned int length) {
    unsigned int crc = 0xFFFF;
    for (unsigned int i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crc_table[(crc ^ data[i]) & 0xFF];
    }
    return crc & 0xFFFF;
}

// Sprawdzenie poprawności sekcji danych
void checkData(){

//  Sprawdzenie długości komendy
//	if (strlen(receivedData) != maxCmdLen){
//		frameRecall(4004, COMMAND);
//		return;
//	}

	// Sprawdzenie czy separatory argumentu są na odpowiednich miejscach ( - 1 bo tablica ma jeszcze miejsce dla '\0')
	if (receivedData[8] != '(' || receivedData[maxCmdLen - 1] != ')') {
		frameRecall(4005, COMMAND);
		return;
	}
	// Sprawdzenie czy separatory nie występują w nieoczekiwanych miejscach.
	for (int i = 0; i < maxCmdLen; i++) {
		if (i != 8 && i != maxCmdLen - 1 && (receivedData[i] == '(' || receivedData[i] == ')')) {
			frameRecall(4006, COMMAND);
			return;
		}
	}
}

// Pobranie typu komendy, komendy i argumentu.
void getCommand(){
	// Pobranie typu komendy z sekcji danych
	for (int i = 0; i < 3; i++) {
		commandType[i] = receivedData[i];
	}
	commandType[3] = '\0';

	// Pobranie komendy z sekcji danych
	for (int i = 0; i < 5; i++) {
		command[i] = receivedData[i+3];
	}
	command[5] = '\0';

	// Pobranie argumentu z sekcji danych (typ + komenda + () = 10)
	for (int i = 0; i < maxCmdLen - 10; i++) {
		commandArg[i] = receivedData[i+9];
	}
	// Zamienienie ostatniego znaku argumentu komendy na znak końca stringa
	commandArg[maxCmdLen - 10] = '\0';
	return;
}

// Funkcja konwertująca argument typu STR -> INT
void getNumArgument(){

	// Zerowanie wartości z ostatniej ramki
	for (int i = 0; i < 3; i++) numArguments[i] = 0;

	int numberIndex = 0;                                // Indeks aktualnie odczytywanej liczby
	int argumentNr = 0;                                 // Licznik ilości argumentów

	for (int i = 0; i <= strlen(commandArg); i++){
		tmpArr[numberIndex] = '\0';
		// Jeżeli jest więcej argumentów niż 3 zwraca błąd
        if (argumentNr > 2){
			frameRecall(5007, COMMAND);
			return;
		}
        // Jeżeli znak nie jest separatorem argumentu zapisze znak w tablicy pomocniczej
		if (commandArg[i] != ',' && i != strlen(commandArg)){
			tmpArr[numberIndex++] = commandArg[i];
			tmpArr[numberIndex] = '\0';
		}
        // W przeciwnym razie przejdzie do sprawdzenia argumentu jeżeli jest poprawny odczytuje kolejny
        else {
			numArguments[argumentNr++] = isNUM(numberIndex, 999, 0, COMMAND);
			if (errorFlag == 1) return;
			numberIndex = 0;
		}
	}
}

// Wykonywanie komend
void executeCommand(){
	// Sprawdzenie czy komenda jest typu TXT
	if (strcmp(commandType, "TXT") == 0){
		if (strcmp(command, "COLOR") == 0) COLOR();
		else if (strcmp(command, "EFFCT") == 0) EFFCT();
		else if (strcmp(command, "RGBST") == 0) RGBST();
		else if (strcmp(command, "GETST") == 0) GETST();
        // Jeżeli podana komenda nie istnieje zwórci błąd
		else frameRecall(5001, EXECUTE);
	}
	else if (strcmp(commandType, "NUM") == 0){
		getNumArgument();
		if (errorFlag == 1) return;
		if (strcmp(command, "BRGHT") == 0) BRGHT(0);
		else if (strcmp(command, "STCLR") == 0) STCLR();
        // Jeżeli podana komenda nie istnieje zwróci błąd
		else frameRecall(5002, EXECUTE);
	}
    // Jeżeli podany typ komendy nie istnieje zwórci błąd
    else frameRecall(5003, EXECUTE);
	return;
}

// ====================================================== READ FRAME FUNCTION END ======================================================

// ====================================================== WYKONYWANIE KOMEND START ======================================================
// Deklaracja tablic wykorzystywanych przez DMA

uint32_t redDMAValues_ACTUAL[512];									// Główna tablica DMA używana przez &htim4, TIM_CHANNEL_1 (R)
uint32_t greenDMAValues_ACTUAL[512];								// Główna tablica DMA używana przez &htim4, TIM_CHANNEL_2 (G)
uint32_t blueDMAValues_ACTUAL[512];									// Główna tablica DMA używana przez &htim4, TIM_CHANNEL_3 (B)

uint32_t redDMAValues_NEW[512];                                  	// Tablica DMA podmieniana podczas końca transmisji PWM ( callback )
uint32_t greenDMAValues_NEW[512];                                	// Tablica DMA podmieniana podczas końca transmisji PWM ( callback )
uint32_t blueDMAValues_NEW[512];                                    // Tablica DMA podmieniana podczas końca transmisji PWM ( callback )

uint32_t redDMAValues_SPECIAL[512];                                 // Tablica specjalna do efektów które potrzebują więcej niż  1 callback
uint32_t greenDMAValues_SPECIAL[512];                               // Tablica specjalna do efektów które potrzebują więcej niż  1 callback
uint32_t blueDMABalues_SPECIAL[512];                                // Tablica specjalna do efektów które potrzebują więcej niż  1 callback

int is_half_callbackOne_end = 0;									// Zmienna pilnująca kolejność wypełniania buffora DMA
int is_half_callbackTwo_end = 0;									// Zmienna pilnująca kolejność wypełniania buffora DMA
#define END 1

char actualEffect[20] = {"NONE"};									// Tablica zawierające nazwe aktualnego efektu

#define ON 1
#define OFF -1
#define RESET 0

int ledState = ON;                                                  // Stan diody wł./wył.
float brightness = 100;                                             // Zmienna określająca jasność diody
int prescaler = 35;													// Zmienna zmieniająca częstotliwość

int effectLoop = 1;													// Flaga włączająca i wyłączająca pętle działania efektu
int callbackOne = 0;												// Flaga zmiany efektu
int callbackTwo = 0;												// Dodatkowa flaga zmiany efektu

// ====================================== FUNKCJE POMOCNICZE ======================================

// Inicjalizacja ustawień PWM
void TIM4_change_prescaler(int prescaler) {
    // Ustawienie prescalera w celu zmiany częstotliwości sygnału
	__HAL_TIM_SET_PRESCALER(&htim4, prescaler);
}

// Wypełnianie pierwszej połowy DMA w połowie bufora
void HALF_Transmit_Callback(DMA_HandleTypeDef *htim){
	// Jeżeli zaszła jakaś zmiana efektu diody zostaje przepisana do tablic używanych przez DMA
	if (callbackOne){
		// Podmienienie wartości DMA na nowe
		for (int i = 0; i < 256; i++){
			redDMAValues_ACTUAL[i] = redDMAValues_NEW[i];
			greenDMAValues_ACTUAL[i] = greenDMAValues_NEW[i];
			blueDMAValues_ACTUAL[i] = blueDMAValues_NEW[i];
		}
		// Flaga zakończenia zapełniania pierwszej połowy DMA
		is_half_callbackOne_end = END;
	}
	// Jeżeli jakiś efekt wymagał 2 stopniowej zmiany tablic, wykona się poniższy kod...
	// ...(np. zmiana jasności płynna zmiana(NEW) -> jasność wynikowa (SPECIAL)
	else if (callbackTwo){
		// Podmienienie wartości DMA na nowe
		for (int i = 0; i < 256; i++){
			redDMAValues_ACTUAL[i] = redDMAValues_SPECIAL[i];
			greenDMAValues_ACTUAL[i] = greenDMAValues_SPECIAL[i];
			blueDMAValues_ACTUAL[i] = blueDMABalues_SPECIAL[i];
			// Nadpisanie tablic _NEW zapobiega błędu zmiany jasności -> wył. diody -> wł. diody
			redDMAValues_NEW[i] = redDMAValues_SPECIAL[i];
			greenDMAValues_NEW[i] = greenDMAValues_SPECIAL[i];
			blueDMAValues_NEW[i] = blueDMABalues_SPECIAL[i];

		}
		// Flaga zakończenia zapełniania pierwszej połowy DMA.
		is_half_callbackTwo_end = END;
	}
};

// Wypełnienie drugiej częsci DMA na końcu bufora
void END_Transmit_Callback(DMA_HandleTypeDef *htim)
{
	// Jeżeli zaszła jakaś zmiana efektu diody zostaje przepisana do tablic używanych przez DMA
	if (callbackOne && is_half_callbackOne_end == END){
		// Ustawienie prędkości efektu
		TIM4_change_prescaler(prescaler);
		// Podmienienie wartości DMA na nowe
		for (int i = 256; i < 512; i++){
			redDMAValues_ACTUAL[i] = redDMAValues_NEW[i];
			greenDMAValues_ACTUAL[i] = greenDMAValues_NEW[i];
			blueDMAValues_ACTUAL[i] = blueDMAValues_NEW[i];
		}
		// Reset flag wykonujących kod przerwania
		callbackOne = 0;
		is_half_callbackOne_end = 0;
	}
	// Jeżeli jakiś efekt wymagał 2 stopniowej zmiany tablic, wykona się poniższy kod...
	// ...(np. zmiana jasności: płynna zmiana(NEW) -> jasność wynikowa (SPECIAL)
	else if (callbackTwo && is_half_callbackTwo_end == END){
		// Ustawienie prędkości efektu
		TIM4_change_prescaler(prescaler);
		// Podmienienie wartości DMA na nowe
		for (int i = 256; i < 512; i++){
			redDMAValues_ACTUAL[i] = redDMAValues_SPECIAL[i];
			greenDMAValues_ACTUAL[i] = greenDMAValues_SPECIAL[i];
			blueDMAValues_ACTUAL[i] = blueDMABalues_SPECIAL[i];
			// Nadpisanie tablic _NEW zapobiega błędu zmiany jasności -> wył. diody -> wł. diody
			redDMAValues_NEW[i] = redDMAValues_SPECIAL[i];
			greenDMAValues_NEW[i] = greenDMAValues_SPECIAL[i];
			blueDMAValues_NEW[i] = blueDMABalues_SPECIAL[i];
		}
		// Reset flag wykonujących kod przerwania
		callbackTwo = 0;
		is_half_callbackTwo_end = 0;
	}
}


void start_PWM_DMA(){
	// Inicjalizacja transmisji 3 KANAŁÓW PWM'a z użyciem DMA
	HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_1, (uint32_t *)redDMAValues_ACTUAL, 512);
	HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_2, (uint32_t *)greenDMAValues_ACTUAL, 512);
	HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_3, (uint32_t *)blueDMAValues_ACTUAL, 512);

	// Ustawienie wskaźnika callback w połowie transmisji z DMA do peryferium
	htim4.hdma[TIM_DMA_ID_CC1]->XferHalfCpltCallback = HALF_Transmit_Callback;

	// Ustawienie wskaźnika callback na końcu transmisji z DMA do peryferium.
	htim4.hdma[TIM_DMA_ID_CC1]->XferCpltCallback = END_Transmit_Callback;
}

// Funkcja odpowiedzialna za wyłączanie i włączanie diody (Podmiana starych wartości na 0 lub odwrotnie)
void switch_PWM_DMA(uint32_t *nullDMARed, uint32_t *nullDMAGreen, uint32_t *nullDMABlue, int ledFlag){
	// Zmienne przechowujące stare wartości DMA
	static uint32_t oldRedDMA[512];
	static uint32_t oldGreenDMA[512];
	static uint32_t oldBlueDMA[512];
	// Wyłączenie polega na podmienieniu wartości tablic DMA na zerowe
	if (ledFlag == OFF){
		for (int i = 0; i < 512; i++){
			// Zapamiętanie ostatniego stanu.
			oldRedDMA[i] = redDMAValues_NEW[i];
			oldGreenDMA[i] = greenDMAValues_NEW[i];
			oldBlueDMA[i] = blueDMAValues_NEW[i];
			// Wyłączenie diody
			redDMAValues_NEW[i] = 0;
			greenDMAValues_NEW[i] = 0;
			blueDMAValues_NEW[i] = 0;
			ledState = OFF;
			callbackOne = 1;
		}
	// Włączenie polega na podmienienu wartości tablic DMA na poprzednie
	} else if (ledFlag == ON){
		for (int i = 0; i < 512; i++){
			// Przywrócenie starej jasności
			redDMAValues_NEW[i] = oldRedDMA[i];
			greenDMAValues_NEW[i] = oldGreenDMA[i];
			blueDMAValues_NEW[i] = oldBlueDMA[i];
			oldRedDMA[i] = 0;
			oldGreenDMA[i] = 0;
			oldBlueDMA[i] = 0;
			ledState = ON;
			callbackOne = 1;
		}
	// Dodatkowa opcja która podmiena stare wartości na nowe podczas zmiany efektu.
	} else if (ledFlag == RESET){
		for (int i = 0; i < 512; i++){
			// Pobranie nowego efektu. ( Unikniknięcie pominięcia zmian w sytuacji: Efekt -> Wył. -> Zmiana efektu -> Wł.diody )
			oldRedDMA[i] = redDMAValues_NEW[i];
			oldGreenDMA[i] = greenDMAValues_NEW[i];
			oldBlueDMA[i] = blueDMAValues_NEW[i];
		}
	}
}

// ====================================== FUNKCJE EFEKTÓW ======================================

// Funkcja odpowiadająca za zmianę efektu diody
void EFFCT(){
	if (strcmp(commandArg, "POLICE") == 0){
		strcpy(actualEffect, commandArg);
		prescaler = 35;
		generatePoliceLights(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW);
	}
	else if (strcmp(commandArg, "DISCO") == 0){
		prescaler = 35;
		strcpy(actualEffect, commandArg);
		generateDiscoEffect(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW);

	}
	else if (strcmp(commandArg, "BREATH") == 0){
		prescaler = 550;
		strcpy(actualEffect, commandArg);
		generateBreathingEffect(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW);

	}
	else if (strcmp(commandArg, "HEART") == 0){
		strcpy(actualEffect, commandArg);
		prescaler = 35;
		generateHeartBeatEffect(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW);

	}
	else if (strcmp(commandArg, "BLINKING") == 0){
		prescaler = 150;
		strcpy(actualEffect, commandArg);
		generateBlinkingEffect(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW);
	}
	else if (strcmp(commandArg, "RANDOM") == 0){
		prescaler = 175;
		strcpy(actualEffect, commandArg);
		generaterandomBlinkingEffect(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW);
	} else {
		frameRecall(5005, EXECUTE);
		return;
	}
	// Flaga infomrująca ze nastąpiła zmiana efektu diody ( callback )
	callbackOne = 1;
	// Resetowanie jasności diody
	BRGHT(1);
	// Wysłanie komunikatu zwrotnego.
	frameRecall(9002, EXECUTE);
	// Pobranie nowego efektu w celu przechowywania podczas wyłączonej diody.
	switch_PWM_DMA(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW, RESET);
}

void generatePoliceLights(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues) {
	for (int i = 0; i < 512; i++){
		// Wypełnienie tablicy koloru czerwonego w 50%
		if (i < 256){
			bluePWMValues[i] = 0;
			redPWMValues[i] = 899;
			greenPWMValues[i] = 0;
		}
		// Wypełnienie tablicy koloru niebieskiego w 50%
		else {
			bluePWMValues[i] = 899;
			redPWMValues[i] = 0;
			greenPWMValues[i] = 0;
		}
	}
}

void generateDiscoEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues) {
	// Pętla przygotowująca wartości dla bufora DMA
	for (int i = 0; i < 512; i++) {
        redPWMValues[i] = (uint32_t)((sin(i * 2 * M_PI / 512) + 1) * 0.5 * 999);
        greenPWMValues[i] = (uint32_t)((sin(i * 2 * M_PI / 512 + 2 * M_PI / 3) + 1) * 0.5 * 999);
        bluePWMValues[i] = (uint32_t)((sin(i * 2 * M_PI / 512 + 4 * M_PI / 3) + 1) * 0.5 * 999);
    }
}

void generateBreathingEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues) {
	// Paleta kolorów dla wartości RGB w systemie <0, 255> mnożnik * 3,917647058823529.
	uint32_t colorPalette[16][3] = {
			{999,0,0},		{600,0,0},		{999,501,0},	{999,999,0},
			{501,999,0},	{0,999,0}, 		{0,999,501},	{0,999,999},
			{0,501,999},	{0,0,999},		{501,0,999},	{298,0,600},
			{999,0,999},	{600,0,600},	{999,0,498},	{600,0,298}
	};
	int nextColor = 32;
	int paletteIndex = 0;
    // Pętla przygotowująca wartości dla bufora DMA ( takie aby generowały efekt oddychania dla każdego koloru )
	for (uint32_t i = 0; i < 512; i++) {
    	if (i < nextColor){
    		redPWMValues[i] = (uint32_t)((sin(i * 1 * M_PI / 16) + 1) * 0.5 * colorPalette[paletteIndex][0]);
    		greenPWMValues[i] = (uint32_t)((sin(i * 1 * M_PI / 16) + 1) * 0.5 * colorPalette[paletteIndex][1]);
    		bluePWMValues[i] = (uint32_t)((sin(i * 1 * M_PI / 16) + 1) * 0.5 * colorPalette[paletteIndex][2]);
    	}
    	else {
    		paletteIndex++;
    		nextColor += 32;

    	}
    }
}

void generateHeartBeatEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues) {
	// Przebieg zmieniający wartości wypełnienia okresu PWM OUT ( W tym przypadku jasność danej składowej R/G/B )
	for (int i = 0; i < 512; i++) {
		if (i < 256) {
			redPWMValues[i] = 999 - (i * 3);
			greenPWMValues[i] = 0;
			bluePWMValues[i] = 0;
		}
		else
		{
			// Przytrzymanie wypełnienia na 999 - (256 * 3) = 231 dla lepszego efektu bicia serca.
			redPWMValues[i] = 231;
			greenPWMValues[i] = 0;
			bluePWMValues[i] = 0;
		}
	}
}

void generateBlinkingEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues){
	// Paleta kolorów dla wartości RGB w systemie <0, 255> mnożnik * 3,917647058823529.
	uint32_t colorPalette[16][3] = {
			{999,0,0},		{600,0,0},		{999,501,0},	{999,999,0},
			{501,999,0},	{0,999,0}, 		{0,999,501},	{0,999,999},
			{0,501,999},	{0,0,999},		{501,0,999},	{298,0,600},
			{999,0,999},	{600,0,600},	{999,0,498},	{600,0,298}
	};

	int nextColor = 32;
	int paletteIndex = 0;
	// Pętla przygotowująca wartości dla bufora DMA ( co 32 słowa zmiana koloru )
	for (int i = 0; i < 512; i++){
		if (i < nextColor){
			redPWMValues[i] = colorPalette[paletteIndex][0];
			greenPWMValues[i] = colorPalette[paletteIndex][1];
			bluePWMValues[i] = colorPalette[paletteIndex][2];
		} else {
			paletteIndex++;
			nextColor += 32;
		}
	}
}

void generaterandomBlinkingEffect(uint32_t *redPWMValues, uint32_t *greenPWMValues, uint32_t *bluePWMValues){
	int nextColor = 16;
	srand(HAL_GetTick());
	int randomR = rand() % 1001;
	int randomG = rand() % 1001;
	int randomB = rand() % 1001;
	for (int i = 0; i < 512; i++){
		if (i <= nextColor){
		redPWMValues[i] = randomR;
		greenPWMValues[i] = randomG;
		bluePWMValues[i] = randomB;
		} else {
			int tmp = 0;
			while (tmp++ < 100) if(USART_kbhit()) searchFrame();
			randomR = rand() % 1001;
			randomG = rand() % 1001;
			randomB = rand() % 1001;
			nextColor += nextColor;
		}
	}
}

// ====================================== FUNKCJE ZMIANY KOLORÓW ======================================

void STCLR(){
	strcpy(actualEffect, command);
	for (int i = 0; i < 512; i++){
		redDMAValues_NEW[i] = numArguments[0];
		greenDMAValues_NEW[i] = numArguments[1];
		blueDMAValues_NEW[i] = numArguments[2];
	}
	// Ustawienie częstotliowsci TIMERA
	prescaler = 35;
	// Informacja że powinna nastąpić zmiana efektu diody RGB ( callback )
	callbackOne = 1;
	// Resetowanie jasności
	BRGHT(1);
	// Pobranie nowego efektu w celu przechowywania podczas wyłączonej diody.
	switch_PWM_DMA(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW, RESET);
	// Wysłanie komunikatu zwrotnego
	frameRecall(9001, EXECUTE);
}

void COLOR(){
	// Pobranie nazwy efektu w celu wyświetlenia callback'a
	strcpy(actualEffect, command);
	// Ustawia diodę na wybrany kolor.
	int redValue = 0, greenValue = 0, blueValue = 0;
		if (strcmp(commandArg, "RED") == 0) {
			redValue = 999;
		} else if (strcmp(commandArg, "GREEN") == 0) {
			greenValue = 999;
		} else if (strcmp(commandArg, "BLUE") == 0) {
			blueValue = 999;
		} else if (strcmp(commandArg, "YELLOW") == 0) {
			redValue = 999;
			greenValue = 999;
		} else if (strcmp(commandArg, "ORANGE") == 0) {
			redValue = 999;
			greenValue = 501;
		} else if (strcmp(commandArg, "PINK") == 0) {
			redValue = 999;
			greenValue = 200;
			blueValue = 600;
		} else if (strcmp(commandArg, "PURPLE") == 0) {
			redValue = 600;
			blueValue = 600;
		} else if (strcmp(commandArg, "CYAN") == 0) {
			redValue = 200;
			greenValue = 999;
			blueValue = 999;
		} else {
			frameRecall(5006, EXECUTE);
			return;
		}
		// Przypisywanie wybranych wartości do buforów
		for (int i = 0; i < 512; i++) {
			redDMAValues_NEW[i] = redValue;
			greenDMAValues_NEW[i] = greenValue;
			blueDMAValues_NEW[i] = blueValue;
		}
		// Ustawienie częstotliowsci TIMERA
		prescaler = 35;
		// Informacja że powinna nastąpić zmiana efektu diody RGB ( callback )
		callbackOne = 1;
		// Resetowanie jasności diody
		BRGHT(1);
		// Wysłanie komunikatu zwrotnego
		frameRecall(9006, EXECUTE);
		// Pobranie nowego efektu w celu przechowywania podczas wyłączonej diody.
		switch_PWM_DMA(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW, RESET);
		return;
}

// ====================================== FUNKCJE ZMIANY STANU DIODY / ODCZYT STANU DIODY ======================================


void BRGHT(int stockBrigthness){
	// Bufory DMA z wartościami 100%.
	static uint32_t redStockBrigthnes[512];
	static uint32_t greenStockBrigthnes[512];
	static uint32_t blueStockBrigthnes[512];
	static float oldBrightness = 100;							// Stara jasność
	// Jeżeli funkcja przyjeła flage stockBrigthness == 1, zapisuje podstawowe intensywności składowych RGB
	if (stockBrigthness){
		// Zapis jasności 100% ( od niej zawsze jest odliczana jasność )
		brightness = (float)100;
		for (int i = 0; i < 512; i++){
			redStockBrigthnes[i] = redDMAValues_NEW[i];
			greenStockBrigthnes[i] = greenDMAValues_NEW[i];
			blueStockBrigthnes[i] = blueDMAValues_NEW[i];
		}
	}
	// Jeżeli flaga stockBrigthness != 1 oznacza że użytkownik chce zmienić jasnośc diody
	else {
		// Zapis nowej jasności
		oldBrightness = brightness;
		brightness = (float)numArguments[0];
		// Błąd zakresu liczby
		if (brightness > 100){
			frameRecall(2002, EXECUTE);
			return;
		}
		float t;
		// Pętla odpowiedzialna za wyliczenie płynnej zmiany jasności
		for (int i = 0; i < 512; i++){
			if (i != 0) t = (float) i / 512;
			int32_t currentBrightness = oldBrightness + (brightness - oldBrightness) * t;
			redDMAValues_NEW[i] = (uint32_t)(redStockBrigthnes[i] * (currentBrightness / 100.0));
			greenDMAValues_NEW[i] = (uint32_t)(greenStockBrigthnes[i] * (currentBrightness  / 100.0));
			blueDMAValues_NEW[i] = (uint32_t)(blueStockBrigthnes[i] * (currentBrightness  / 100.0));
		}
		// Pętla odpowiedzialna za wyliczenie wynikowej jasności diody (zmiana wypełnienia)
		for (int i = 0; i < 512; i++){
			redDMAValues_SPECIAL[i] = (uint32_t)round(redStockBrigthnes[i] * (brightness / 100));
			greenDMAValues_SPECIAL[i] = (uint32_t)round(greenStockBrigthnes[i] * (brightness / 100));
			blueDMABalues_SPECIAL[i] = (uint32_t)round(blueStockBrigthnes[i] * (brightness / 100));
		}
		callbackOne = 1;					// Wywołanie płynnej zmiany jasności
		callbackTwo = 1;					// Wywołanie statycznej jasności
		frameRecall(9005, EXECUTE);
	}
	return;
}

void RGBST(){
	// Zmienia stan diody na ON/OFF w zależności od argumentu funkcji.
	if (strcmp(commandArg, "ON") == 0) switch_PWM_DMA(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW, ON);
	else if (strcmp(commandArg, "OFF") == 0) switch_PWM_DMA(redDMAValues_NEW, greenDMAValues_NEW, blueDMAValues_NEW, OFF);
	else frameRecall(5008, EXECUTE);

	if (errorFlag == 0) frameRecall(9003, EXECUTE);
}

void GETST(){
	frameRecall(9004, EXECUTE);
	return;
}

// ====================================================== WYKONYWANIE KOMEND END ======================================================

// ====================================================== KOMUNIKATY ZWROTNE START ======================================================

void frameRecall(int ID, int sectionID){
    // Nazwy sekcji w której program zakończył działanie
	const char* sections[] = {"OutOfFrame\0","SENDER\0", "RECEIVER\0", "COMMAND_LENGTH\0", "COMMAND\0", "CHECK_SUM\0", "EXECUTE\0"};

    // Przypisanie nazwy sekcji poprzez odebrany identyfikator sekcji (przy wywołaniu funkcji)
    const char* section = sections[sectionID];

    // Zmienna zawierająca aktualny stan diody wł./wył. etc
	const char* ledStateInf[10];

	// brightness to INT
	int brightnessINT = (int)brightness;

    // Tablica zawierająca znaki które mają zostać wysłane
	char dataToSend[100];

    // Jeżeli identyfikator komunikatu pobrany podczas wywołania funkcji jest < 9000 jest to ID błędu
	if (ID < 9000) errorFlag = 1;

    // Switch dopisujący dane do komunikatu zwrotnego
	switch(ID){
	case 1001:
		sprintf(commandArg, "(1001 - Błąd sekcji %s, wykryto nieoczekiwane użycie separatora!)", section);
		break;
	case 1002:
		sprintf(commandArg, "(1002 - Błąd sekcji %s, nie wykryto separatora w oczekiwanym miejscu!)", section);
		break;
// +++++++++++++++++++++++++++++++++++++++++ DO SPRAWDZENIA +++++++++++++++++++++++++++++++++++++++++
	case 1003:
		sprintf(commandArg, "(1003 - Błąd sekcji %s, wykryto znak bitowego zera \\0 - przerwanie ciągu znaków!)", section);
		break;
// ----------------------------------------- DO SPRAWDZENIA -----------------------------------------
	case 2001:
		sprintf(commandArg, "(2001 - Błąd liczbowy w %s, odczytany znak nie jest cyfrą systemu dziesiętnego!)", section);
		break;
	case 2002:
		sprintf(commandArg, "(2002 - Błąd liczbowy w %s, wprowadzona wartość jest poza dopuszczalnym zakresem!)", section);
		break;
	case 4001:
		strcpy(commandArg, "(4001 - Błąd ramki, nieznany nadawca!)\0");
		break;
	case 4002:
		strcpy(commandArg, "(4002 - Błąd ramki, nieznany odbiorca!)\0");
		break;
	case 4003:
		strcpy(commandArg, "(4003 - Błąd ramki, wyliczona suma kontrolna z danych wynikowych jest inna od podanej!)\0");
		break;
	case 4004:
		strcpy(commandArg, "(4004 - Błąd ramki, długość otrzymanej komendy jest niezgodna z podaną!)\0");
		break;
	case 4005:
        strcpy(commandArg, "(4005 - Błąd ramki, nie wykryto separatorów argumentu w oczekiwanym miejscu!)\0");
		break;
	case 4006:
		strcpy(commandArg, "(4006 - Błąd ramki, wykryto separator argumentu w nieoczekiwanym miejscu!)\0");
		break;
	case 5001:
		strcpy(commandArg, "(5001 - Błąd komendy tekstowej, nieznana komenda!)\0");
		break;
	case 5002:
		strcpy(commandArg, "(5002 - Błąd komendy liczbowej, nieznana komenda!)\0");
		break;
	case 5003:
		strcpy(commandArg, "(5003 - Błąd typu komendy, nieznany typ komendy!)\0");
		break;
	case 5005:
		strcpy(commandArg, "(5005 - Nie znaleziono takiego efektu!)\0");
		break;
	case 5006:
		strcpy(commandArg, "(5006 - Nie znaleziono koloru!)\0");
		break;
	case 5007:
		strcpy(commandArg, "(5007 - Błąd komendy liczbowej, wykryto liczbe argumentów większą od 3!)\0");
		break;
	case 5008:
		strcpy(commandArg, "(5008 - Błąd komendy, nieprawidłowy argument!)\0");
		break;
	case 9001:
		strcpy(commandArg, "(9001 -Pomyślnie zmieniono kolor diody!)\0");
		break;
	case 9002:
		sprintf(commandArg, "(9002 - Pomyślnie zmieniono efekt diody na %s!)", actualEffect);
		break;
	case 9003:
		if (ledState == ON) strcpy(ledStateInf, "ON");
		else if (ledState == OFF) strcpy(ledStateInf, "OFF");
		sprintf(commandArg, "(9003 - Pomyślnie zmieniono stan diody na %s!)", ledStateInf);
		break;
	case 9004:
		if (ledState == ON) strcpy(ledStateInf, "ON");
		if (ledState == OFF) strcpy(ledStateInf, "OFF");
		sprintf(commandArg, "(9004 - Stan: %s,Jasność: %d%%,Efekt: %s!)",ledStateInf, brightnessINT, actualEffect);
		break;
	case 9005:
		sprintf(commandArg, "(9005 - Pomyślnie zmieniono jasność diody na %d %%!)", brightnessINT);
		break;
	default:
        // Na potrzeby wersji DEBUG
		strcpy(commandArg, "Niezidentyfikowany\0");
	}

    // Przypisane odpowiedniego nagłówka komunikatu zwrotnego poprzez sprawdzenie jego ID
	if (ID < 2000) strcpy(dataToSend, "TXTWRNSEC\0");                                           // Błędy sekcji
	else if (ID > 2000 && ID < 3000) strcpy(dataToSend, "TXTWRNNUM\0");                         // Błędy liczbowe
	else if (ID > 4000 && ID < 5000) strcpy(dataToSend, "TXTWRNFRM\0");                         // Błędy w ramce
	else if (ID > 5000 && ID < 6000) strcpy(dataToSend, "TXTWRNCMD\0");                         // Błędy w komendach (zła komenda etc.)
	else if (ID > 9000 && ID < 10000) strcpy(dataToSend, "TXTCMDCRR\0");                        // Komunikat o prawidłowej obsłudze ramki + komendy
	else strcpy(dataToSend, "UNKNOWN\0");                                                       // Na potrzeby wersji DEBUG

	// Połączenie wiadomości do wysłania i obliczenia sumy kontrolnej
	strcat(dataToSend, commandArg);
	maxCmdLen = strlen(dataToSend);                                                             // Wyliczenie długości danych
	crc_value = crc(dataToSend, maxCmdLen) % 999;                                               // Wyliczenie sumy CRC
    // Wysłanie komunikatu
	USART_fsend("\n\r@%s:%s:%d:%s:%d$\n\r\0", recAdr, senAdr, maxCmdLen, dataToSend, crc_value);
	return;
}

// ====================================================== KOMUNIKATY ZWROTNE END ======================================================

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  // Inicjalizacja odbioru USART
  HAL_UART_Receive_IT(&huart2,&RX_BF[0],1);
  // Inicjalizacja PWM + DMA
  start_PWM_DMA();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* USER CODE END WHILE */
	  /* USER CODE BEGIN 3 */
	  searchFrame();										// Szukanie ramki
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_TIM34;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Tim34ClockSelection = RCC_TIM34CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 35;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
