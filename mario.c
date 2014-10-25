#include <stdio.h>

#include "resource.h"
#include "city.h"
#include "list.h"
#include "map.h"
#include "road.h"

void MarioIgnoreUntilLine() {
	StdOut("Waiting until line...\r\n");
	
	MarioFlush();
	
	unsigned char readbyte = 0;
	int bytesread = 0;

	while(1) {
		
		// Byte ontvangen?
		bytesread = ReadByte(&readbyte);
		if(bytesread) {
	
			char signalcopy[9] = "00000000";
			SignalToCharArray(readbyte,signalcopy);
			StdOut("Received '%s' (%3d)...\r\n",signalcopy,readbyte);

			if(/*readbyte == 0 || readbyte == 1 || */readbyte == 5) {
				break;
			}
		}
		Sleep(1);
	}
}

// Saai naar voren: gewoon doorrijden zonder commands door te geven, wat
// voorkomt dat de robot even 'hikt' bij mario aanzetten.
void MarioNoSignalForward(unsigned int ms) {
	StdOut("Doing nothing\r\n");
	
	unsigned char readbyte = 0;
	int bytesread = 0;

	DWORD oldtime = GetTickCount();
	while(oldtime + ms > GetTickCount()) {
		
		// Byte ontvangen?
		bytesread = ReadByte(&readbyte);
		if(bytesread) {
				
			char signalcopy[9] = "00000000";
			SignalToCharArray(readbyte,signalcopy);
			StdOut("Received '%s' (%3d)...\r\n",signalcopy,readbyte);
			
			if(readbyte >= 8) {
				
				// Mijn gevonden
				StdOut("Found mine!\r\n");
			
				UpdateMapMine();
				return;
			}
		}
		Sleep(1);
	}
	
	MarioFlush();
	MessageBeep(MB_ICONWARNING);
}

void MarioFlush() {
	StdOut("Flushing buffer...\r\n");
	
	unsigned char* readbyte = malloc(1000);
	DWORD bytes = 0;
	
	if(ComHandle != INVALID_HANDLE_VALUE) {
		ReadFile(ComHandle,&readbyte,1000,&bytes,NULL);
	}
	
	free(readbyte);
}

// Volledige bocht naar links
void MarioLeft() {
	StdOut("Taking left turn\r\n");
	
//	mciSendString("play sfx\\linksaf.wma",NULL,0,NULL);
	
	// Een stukje naar voren
	mario = TRUE;
	leftarrow = FALSE;
	rightarrow = FALSE;
	uparrow = TRUE;
	downarrow = FALSE;
	UpdateSignal();
	Sleep(280);

	// Een stukje draaien
	mario = TRUE;
	leftarrow = TRUE;
	rightarrow = FALSE;
	uparrow = FALSE;
	downarrow = FALSE;
	UpdateSignal();
	Sleep(1000);
	
	// Dan draaien totdat een lijn wordt gevonden
	MarioIgnoreUntilLine();
	MarioDisable();

	// en lang de sensor negeren
	MarioNoSignalForward(1000);
}

// Volledige bocht naar rechts
void MarioRight() {
	StdOut("Taking right turn\r\n");
	
//	mciSendString("play sfx\\rechtsaf.wma",NULL,0,NULL);
	
	// Een stukje naar voren
	mario = TRUE;
	leftarrow = FALSE;
	rightarrow = FALSE;
	uparrow = TRUE;
	downarrow = FALSE;
	UpdateSignal();
	Sleep(280);

	// Een stukje draaien
	mario = TRUE;
	leftarrow = FALSE;
	rightarrow = TRUE;
	uparrow = FALSE;
	downarrow = FALSE;
	UpdateSignal();
	Sleep(1000);
	
	// Dan draaien totdat een lijn wordt gevonden
	MarioIgnoreUntilLine();
	MarioDisable();

	// en lang de sensor negeren
	MarioNoSignalForward(1000);
}

// Ga weer een lijn volgen
void MarioDisable() {
	if(mario) {
		StdOut("Disabling Mario\r\n");
		mario = FALSE;
		UpdateSignal();
	}
}

// Draai om voor een mijn, moet nog gefixt worden
void MarioTurnAround(BOOL ismine) {
	StdOut("Taking 180 deg turn\r\n");
	
//	mciSendString("play sfx\\keerom.wma",NULL,0,NULL);
	
	// Een stukje vooruit
	mario = TRUE;
	leftarrow = FALSE;
	rightarrow = FALSE;
	uparrow = TRUE;
	downarrow = FALSE;
	UpdateSignal();
	Sleep(200);

	// Een stukje draaien
	mario = TRUE;
	leftarrow = TRUE;
	rightarrow = FALSE;
	uparrow = FALSE;
	downarrow = FALSE;
	UpdateSignal();
	Sleep(1900);
	
	// Dan draaien totdat een lijn wordt gevonden
	MarioIgnoreUntilLine();
	
	if(ismine) {
		
		// Een stukje achteruit
		mario = TRUE;
		leftarrow = FALSE;
		rightarrow = FALSE;
		uparrow = FALSE;
		downarrow = TRUE;
		UpdateSignal();
		Sleep(800);
	}
	MarioDisable();

	// en lang de sensor negeren
	MarioNoSignalForward(300);	
}

// Ga stilstaan
void MarioStop() {
	StdOut("Stopping\r\n");

	mario = TRUE;
	leftarrow = FALSE;
	rightarrow = FALSE;
	uparrow = FALSE;
	downarrow = FALSE;
	UpdateSignal();
}

void MarioUpdate(int olddir,int newdir,BOOL ismine) {
	switch(olddir) {
		case 0: {
			switch(newdir) {
				case 0: {
					MarioNoSignalForward(800);
					break;
				}
				case 1: {
					MarioRight();
					break;
				}
				case 2: {
					MarioTurnAround(ismine);
					break;
				}
				case 3: {
					MarioLeft();
					break;
				}

			}
			break;
		}
		case 1: {
			switch(newdir) {
				case 0: {
					MarioLeft();
					break;
				}
				case 1: {
					MarioNoSignalForward(800);
					break;
				}
				case 2: {
					MarioRight();
					break;
				}
				case 3: {
					MarioTurnAround(ismine);
					break;
				}
			}
			break;
		}
		case 2: {
			switch(newdir) {
				case 0: {
					MarioTurnAround(ismine);
					break;
				}
				case 1: {
					MarioLeft();
					break;
				}
				case 2: {
					MarioNoSignalForward(800);
					break;
				}
				case 3: {
					MarioRight();
					break;
				}
			}
			break;
		}
		case 3: {
			switch(newdir) {
				case 0: {
					MarioRight();
					break;
				}
				case 1: {
					MarioTurnAround(ismine);
					break;
				}
				case 2: {
					MarioLeft();
					break;
				}
				case 3: {
					MarioNoSignalForward(800);
					break;
				}
			}
			break;
		}
	}
}
