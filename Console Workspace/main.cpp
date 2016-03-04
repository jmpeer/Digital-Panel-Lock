#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

#define LINE_MAX_SIZE 256
#define BUFFER_MAX_SIZE 256
#define PORT_NAME_MAX_SIZE 256
#define CODE_SIZE 11
#define NUM_GUESTS 10
#define NUM_CODES NUM_GUESTS+1

int SetPort (HANDLE &portHandle, char *portName) {

	if (portHandle) CloseHandle(portHandle);
	portHandle= CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (portHandle== INVALID_HANDLE_VALUE) return 0;

	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(portHandle, &dcbSerialParams)) return 0;

	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(portHandle, &dcbSerialParams)) return 0;

	COMMTIMEOUTS timeouts = {0};
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(portHandle, &timeouts)) return 0;

	return 1;

}

// Copy a string from the stream to the buffer.
// The copying ends upon one of the following conditions:
//   1.) a newline character has been detected
//   2.) an end-of-file character has been detected
//   3.) (bufferSize-1) characters have been copied
// A null character is then added to the end of the string.
// Returns a pointer to buffer if successful, null if not.
// Note: The newline and end-of-file characters will not be included in the string.
char* GetString (char *buffer, int bufferSize, FILE *stream) {
	int length;
	buffer = fgets(buffer, bufferSize, stream);
	if (!buffer) return buffer;
	length = strlen(buffer);
	if (buffer[length-1] == '\n') buffer[length-1] = '\0';
	return buffer;
}

// Returns 1 if str begins with pre, otherwise, 0.
int StringBeginsWith (const char *str, const char *pre) {
	unsigned int prelen = strlen(pre);
	if (prelen <= strlen(str) && strncmp(pre, str, prelen) == 0) return 1;
	else return 0;
}

// Sends numBytes from the buffer to the USB device.
// numBytesSent is set to the number of bytes successfully sent.
// Returns 1 if numBytesSent == numBytes, otherwise, 0.
// Note: numBytesSent can be used for error handling.
int SetBuffer (char *buffer, int numBytes, int *numBytesSent, HANDLE portHandle) {
	WriteFile(portHandle, buffer, numBytes, (PDWORD) numBytesSent, NULL);
	return (*numBytesSent == numBytes);
}

// Waits to receive numBytes from the USB device to put in buffer.
// numBytesReceived is set to the number of bytes successfully received.
// Returns 1 if numBytesReceived == numBytes, otherwise, 0.
// Note: numBytesReceived can be used for error handling.
int GetBuffer (char *buffer, int numBytes, int *numBytesReceived, HANDLE portHandle) {
	ReadFile(portHandle, buffer, numBytes, (PDWORD) numBytesReceived, NULL);
	return (*numBytesReceived == numBytes);
}

int OpenConfig (char code [NUM_CODES][CODE_SIZE], int &numGuests) {

	FILE *file;
	char line [LINE_MAX_SIZE];
	int i;

	file = fopen("config.txt", "r");
	if (!file) return 0;

	GetString(line, LINE_MAX_SIZE, file);
	strcpy(code[0], line);
	numGuests = 0;
	for (i = 1; i <= NUM_GUESTS; i++) {
		if (!GetString(line, LINE_MAX_SIZE, file)) break;
		strcpy(code[i], line);
		numGuests++;
	}

	fclose(file);
	return 1;

}

int main () {

	int debug = 0;

	HANDLE portHandle = 0;
	char portName [PORT_NAME_MAX_SIZE];
	int isPortSet = 0;
	char line [LINE_MAX_SIZE];
	char buffer [BUFFER_MAX_SIZE];
	int numBytesSent, numBytesReceived;
	char code [NUM_CODES][CODE_SIZE];
	int numGuests;
	int i, match;
	int isLocked = 0, isActive = 0;

	if (!OpenConfig(code, numGuests)) {
		printf("Error: couldn't open config.txt. \n");
		return 0;
	}

	if (debug) {
		printf("Active: %d, Locked: %d\n", isActive, isLocked);
		for (i = 0; i < numGuests+1; i++) {
			printf("Code %d: %s\n", i, code[i]);
		}
	}

	while (!isPortSet) {
		printf("Port name: ");
		GetString(portName, PORT_NAME_MAX_SIZE, stdin);
		isPortSet = SetPort(portHandle, portName);
		if (!isPortSet) printf("Error: couldn't connect to port. \n");
	}

	// set active
	isActive = 1;
	buffer[0] = 1;
	buffer[1] = 2;
	SetBuffer(buffer, 2, &numBytesSent, portHandle);

	// set locked
	isLocked = 1;
	buffer[0] = 1;
	buffer[1] = 0;
	SetBuffer(buffer, 2, &numBytesSent, portHandle);

	while (1) {
		if (_kbhit()) {
			GetString (line, LINE_MAX_SIZE, stdin);
			if (strcmp(line, "status") == 0) {
				printf("Active: %d, Locked: %d\n", isActive, isLocked);
			} else if (strcmp(line, "locked") == 0) {
				isLocked = 1;
				// send: set locked
				buffer[0] = 1;
				buffer[1] = 0;
				SetBuffer(buffer, 2, &numBytesSent, portHandle);
			} else if (strcmp(line, "unlocked") == 0) {
				isLocked = 0;
				// send: set unlocked
				buffer[0] = 1;
				buffer[1] = 1;
				SetBuffer(buffer, 2, &numBytesSent, portHandle);
			} else if (strcmp(line, "active") == 0) {
				isActive = 1;
				// send: set active
				buffer[0] = 1;
				buffer[1] = 2;
				SetBuffer(buffer, 2, &numBytesSent, portHandle);
			} else if (strcmp(line, "inactive") == 0) {
				isActive = 0;
				// send: set inactive
				buffer[0] = 1;
				buffer[1] = 3;
				SetBuffer(buffer, 2, &numBytesSent, portHandle);
			} else if (strcmp(line, "quit") == 0) {
				break;
			} else {
				printf("Error: unrecognized command.\n");
			}
		} else if (GetBuffer(buffer, 1, &numBytesReceived, portHandle)) {
			GetBuffer(&buffer[1], (unsigned char) buffer[0], &numBytesReceived, portHandle);
			// receieve: set locked (by user while active and unlocked)
			if (buffer[1] == 0) {
				isLocked = 1;
				printf("Locked by the user.\n");
			// receieve: request set unlocked (by user while active and locked)
			} else if (buffer[1] == 1) {
				match = 0;
				for (i = 0; i < numGuests+1; i++) {
					if (strcmp(&buffer[2], code[i]) == 0) {
						match = 1;
					}
				}
				if (match) {
					isLocked = 0;
					printf("%s was entered. Unlocking.\n", &buffer[2]);
					// send: accept set unlocked
					buffer[0] = 1;
					buffer[1] = 4;
					SetBuffer(buffer, 2, &numBytesSent, portHandle);
				} else {
					printf("%s was entered. Remaining locked.\n", &buffer[2]);
					// send: reject set unlocked
					buffer[0] = 1;
					buffer[1] = 5;
					SetBuffer(buffer, 2, &numBytesSent, portHandle);
				}
			} else {
				// incorrect command sent from board
			}
		} else {
			// optional delay between polling for keyboard and USB input
			Sleep(50);
		}
	}

	if (portHandle != 0) CloseHandle(portHandle);

    return 0;
}
