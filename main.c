#define _WIN32_WINNT 0x0700 // Windows 7

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <dwmapi.h>

#include "resource.h"
#include "city.h"
#include "list.h"
#include "map.h"
#include "road.h"
#include "mario.h"

// Converteer bijvoorbeeld '00000010' naar 2
unsigned char CharArrayToSignal(const char* signalin) {
	unsigned char result = 0;
	for(int i = 0;i < 8;i++) {
		if(signalin[7-i] == '1') {
			result += pow(2,i);
		}
	}
	return result;
}

// Converteer bijvoorbeeld 2 naar '00000010'
void SignalToCharArray(unsigned char signalin,char* chararrayout) {
	unsigned char p;
	for(int i = 7;i >=0;i--) {
		p = pow(2,i);
		if(p <= signalin) {
			chararrayout[7-i] = '1';
			signalin -= p;
		}
	}
}

// Herleid de output van printf naar het tekstveld i.p.v. de console
void StdOut(const char* format,...) {
	int len = SendMessage(Logveld,WM_GETTEXTLENGTH,0,0);
	if(len >= 20000) { // Zorg ervoor dat het ding niet volloopt
		SendMessage(Logveld,WM_SETTEXT,0,(LPARAM)"");
		len = 0;
	}
	char* text = calloc(len + 300,1);

	va_list parameters;
	va_start(parameters,format);
	vsprintf(text,format,parameters);
	va_end(parameters);
	
	SendMessage(Logveld,EM_SETSEL,(WPARAM)len,(LPARAM)len);
	SendMessage(Logveld,EM_REPLACESEL,0,(LPARAM)text);
	
	free(text);
}

// Herleid de output van printf naar het tekstveld i.p.v. de console,
// inclusief errorbeep.
void StdErr(const char* format,...) {
	int len = SendMessage(Logveld,WM_GETTEXTLENGTH,0,0);
	if(len >= 20000) { // Zorg ervoor dat het ding niet volloopt
		SendMessage(Logveld,WM_SETTEXT,0,(LPARAM)"");
		len = 0;
	}
	char* text = calloc(len + 300,1);

	va_list parameters;
	va_start(parameters,format);
	vsprintf(text,format,parameters);
	va_end(parameters);
	
	SendMessage(Logveld,EM_SETSEL,(WPARAM)len,(LPARAM)len);
	SendMessage(Logveld,EM_REPLACESEL,0,(LPARAM)text);
	
	free(text);

	MessageBeep(MB_ICONERROR);
}

// Werk de vensters die de status bijhouden bij
void UpdateInfo() {
	char buffer[200] = "";
	if(connected) {
		sprintf(buffer,"Robotbesturing - Connected");
	} else {
		sprintf(buffer,"Robotbesturing - Not connected");
	}
	SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)buffer);
	
	if(mario) {
		SendMessage(MarioText,WM_SETTEXT,0,(LPARAM)"Mario is ON");
	} else {
		SendMessage(MarioText,WM_SETTEXT,0,(LPARAM)"Mario is OFF");
	}
	
	if(!leftarrow && !uparrow && !downarrow && !rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Reset");
	else if(!leftarrow && !uparrow && !downarrow &&  rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Right rotate");
	else if(!leftarrow && !uparrow &&  downarrow && !rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Reverse");
	else if(!leftarrow && !uparrow &&  downarrow &&  rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Right reverse turn");
	else if(!leftarrow &&  uparrow && !downarrow && !rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Forward");
	else if(!leftarrow &&  uparrow && !downarrow &&  rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Right turn");
	else if(!leftarrow &&  uparrow &&  downarrow && !rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"ERROR");
	else if(!leftarrow &&  uparrow &&  downarrow &&  rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"ERROR");
	else if( leftarrow && !uparrow && !downarrow && !rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Left rotate");
	else if( leftarrow && !uparrow && !downarrow &&  rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"ERROR");
	else if( leftarrow && !uparrow &&  downarrow && !rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Left reverse turn");
	else if( leftarrow && !uparrow &&  downarrow &&  rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"ERROR");
	else if( leftarrow &&  uparrow && !downarrow && !rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"Left turn");
	else if( leftarrow &&  uparrow && !downarrow &&  rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"ERROR");
	else if( leftarrow &&  uparrow &&  downarrow && !rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"ERROR");
	else if( leftarrow &&  uparrow &&  downarrow &&  rightarrow)
		SendMessage(TurnText,WM_SETTEXT,0,(LPARAM)"ERROR");
}

// Lees 1 byte van COM
DWORD ReadByte(unsigned char* byte) {
	if(ComHandle != INVALID_HANDLE_VALUE) {
		DWORD bytes = 0;
		if(ReadFile(ComHandle,byte,1,&bytes,NULL)) {
			return bytes;
		} else {
			return FALSE;			
		}
	} else {
		return FALSE;
	}
}

// Schrijf 1 byte naar COM
BOOL WriteByte(const unsigned char byte) {
	if(ComHandle != INVALID_HANDLE_VALUE) {
		DWORD bytes = 0;
		if(WriteFile(ComHandle,&byte,1,&bytes,NULL)) {
			StdOut("Success!\r\n");
			return TRUE;
		} else {
			StdErr("Error writing byte\r\n");
			return FALSE;
		}
	} else {
		StdErr("Can't send data to broken COM!\r\n");
		return FALSE;
	}
}

void UpdateMapMine() {
	if(next) {
	
		StdOut("City %s has mine!\r\n",next->name);
		next->hasmine = TRUE;
		
		// Schakel ook de weg aan de overkant van de mijn uit!
		List* wegenwalker = next->roads;
		while(wegenwalker != NULL) {

			Road* thisroad = wegenwalker->data;
			City* destination = thisroad->destination;
			
			// En vind alle wegen die naar andere steden dan current linken
			if(destination != current) {
				thisroad->disabled = TRUE;
				
				// Nu hebben we gevonden wat de stad aan de overkant is
				// Zet dan de weg de andere kant op ook uit!
				List* wegenwalker2 = destination->roads;
				while(wegenwalker2 != NULL) {
		
					Road* thisroad2 = wegenwalker2->data;
					City* destination2 = thisroad2->destination;
					
					// En vind de weg die naar de mijn gaat
					if(destination2 == next) {
						thisroad2->disabled = TRUE; // UIT!
						break;
					}
					
					wegenwalker2 = wegenwalker2->next;
				}
				
				break;
			}
			
			wegenwalker = wegenwalker->next;
		}
	
		ResetMap();
		FindShortestRoute(next,currentto);
		
		int olddir = 0; // 0 == up, 1 == right, 2 == down, 3 == left
		int newdir = 0;
	
		// vind de huidige richting uit
		if(next->x == current->x) {
			if(next->y > current->y) {
				olddir = 2;
			} else if(next->y < current->y) {
				olddir = 0;
			}
		} else if(next->y == current->y) {
			if(next->x > current->x) {
				olddir = 1;
			} else if(next->x < current->x) {
				olddir = 3;
			}
		}
		
		City* copy = next;
		next = current;
		current = copy;
		
		// vind de nieuwe richting uit
		if(next->x == current->x) {
			if(next->y > current->y) {
				newdir = 2;
			} else if(next->y < current->y) {
				newdir = 0;
			}
		} else if(next->y == current->y) {
			if(next->x > current->x) {
				newdir = 1;
			} else if(next->x < current->x) {
				newdir = 3;
			}
		}
				
		// We hebben een nieuwe stad gevonden, dus opnieuw tekenen
		InvalidateRgn(hwnd,NULL,TRUE);
		
		// En stuur commando's naar de robot
		MarioUpdate(olddir,newdir,TRUE);
	}
}

// Nadat we een stad hebben gevonden, moeten we de route bijwerken...
void UpdateRoute(BOOL flipcities) {
	
	// Als we nog niet klaar zijn met de route
	if(next) {
		
		// Kijken of we bij een S-stad terecht zijn gekomen
		if(next == to1) {
			ResetMap(); // oude route weggooien
			FindShortestRoute(to1,to2); // Nieuwe berekenen
		} else if(next == to2) {
			ResetMap();
			FindShortestRoute(to2,to3);
		} else if(next == to3) {

			current = next;
			next = NULL; // definitie van klaar
			
			// Zet de robot uit
			MarioStop();
			
			InvalidateRgn(hwnd,NULL,FALSE);
			
			mciSendString("play sfx\\einde.wma",NULL,0,NULL);
			mciSendString("play sfx\\music5.mid",NULL,0,NULL);
			return;
		}
		
		int olddir = 0; // 0 == up, 1 == right, 2 == down, 3 == left
		int newdir = 0;
	
		// vind de huidige richting uit
		if(next->x == current->x) {
			if(next->y > current->y) {
				olddir = 2;
			} else if(next->y < current->y) {
				olddir = 0;
			}
		} else if(next->y == current->y) {
			if(next->x > current->x) {
				olddir = 1;
			} else if(next->x < current->x) {
				olddir = 3;
			}
		}
		
		// Zet de stedenpointers een stap verder
		if(flipcities) {
			City* copy = next;
			next = current;
			current = copy;				
		} else {
		
			// Wandel de wegenlist af van de stad waar we aankomen
			List* wegenwalker = next->roads;
			while(wegenwalker != NULL) {
	
				Road* thisroad = wegenwalker->data;
				City* destination = thisroad->destination;
				
				// En vind uit waar we heenmoeten
				if(destination->highlighted && 
				   destination != current &&
				   destination->totaldistance == next->totaldistance + 1) {
					
					current = next;
					next = destination;
					
					break;
				}
				
				wegenwalker = wegenwalker->next;
			}
		}

		// vind de nieuwe richting uit
		if(next->x == current->x) {
			if(next->y > current->y) {
				newdir = 2;
			} else if(next->y < current->y) {
				newdir = 0;
			}
		} else if(next->y == current->y) {
			if(next->x > current->x) {
				newdir = 1;
			} else if(next->x < current->x) {
				newdir = 3;
			}
		}
				
		// We hebben een nieuwe stad gevonden, dus opnieuw tekenen
		InvalidateRgn(hwnd,NULL,TRUE);
		
		// En stuur commando's naar de robot
		MarioUpdate(olddir,newdir,FALSE);
	}
}

// Converteer de pijltjesstates naar een signaal, en verstuur
void UpdateSignal() {
	char signal[9] = "00000000";
	if(mario) {
		signal[0] = '1';
	}
	
	if(!leftarrow &&  uparrow && !downarrow &&  !rightarrow)
		strcpy(&signal[4],"0000"); // Reset
	if(!leftarrow &&  uparrow && !downarrow &&  !rightarrow)
		strcpy(&signal[4],"0001"); // Forward
	else if( leftarrow && !uparrow && !downarrow && !rightarrow)
		strcpy(&signal[4],"0010"); // Left rotate
	else if(!leftarrow && !uparrow && !downarrow &&  rightarrow)
		strcpy(&signal[4],"0011"); // Right rotate
	else if( leftarrow &&  uparrow && !downarrow && !rightarrow)
		strcpy(&signal[4],"0100"); // Left turn
	else if(!leftarrow &&  uparrow && !downarrow &&  rightarrow)
		strcpy(&signal[4],"0101"); // Right turn
	else if( leftarrow && !uparrow &&  downarrow && !rightarrow)
		strcpy(&signal[4],"0110"); // Left reverse turn
	else if(!leftarrow && !uparrow &&  downarrow &&  rightarrow)
		strcpy(&signal[4],"0111"); // Right reverse turn
	else if(!leftarrow && !uparrow &&  downarrow && !rightarrow)
		strcpy(&signal[4],"1000"); // Reverse

	// Converteer de chararray naar een unsigned char
	StdOut("Sending  '%s' (%3d)... ",signal,CharArrayToSignal(signal));
	
	// En verstuur
	WriteByte(CharArrayToSignal(signal));
	UpdateInfo();
}

// Hier is één thread niks anders aan het doen dan COM lezen en schrijven
void ComReaderThread() {
	unsigned char readbyte = 0;
	int bytesread = 0;

	// Eeuwig doorgaan
	while(1) {
		
		// Byte ontvangen?
		bytesread = ReadByte(&readbyte);
		if(bytesread) {
				
			// Converteren naar array
			char signalcopy[9] = "00000000";
			SignalToCharArray(readbyte,signalcopy);
			StdOut("Received '%s' (%3d)...\r\n",signalcopy,readbyte);
			
			// Alleen de controle overnemen als we een stad vinden
			if((readbyte == 0 || readbyte == 8) && next) {
				if(next != to3) {
					StdOut("Found city!\r\n");

					// Route bijwerken
					UpdateRoute(FALSE);
				}
			} else if((readbyte == 7 || readbyte == 15) && next) {
				if(next->name[0] == 'S'){
					StdOut("Found S city!\r\n");

					// Route bijwerken
					UpdateRoute(TRUE);
				}
			} else if(readbyte >= 8) {
				// Mijn gevonden
				StdOut("Found mine!\r\n");
				
				UpdateMapMine();
			}
		}
		
		// Niet blijven doen, maar heel even pauzeren
		Sleep(1);
	}
}

// Stel de eigenschappen van de COM-verbinding in
BOOL InitInterface(const HANDLE hSerial) {

	COMMTIMEOUTS timeouts;
	DCB serialparams;

	serialparams.DCBlength = sizeof(serialparams);

	if(!GetCommState(hSerial,&serialparams)) {
		StdErr("Error getting state!\r\n");
		return FALSE;
	}

	char buffer[20] = "";
	SendMessage(Baud,WM_GETTEXT,sizeof(buffer),(LPARAM)&buffer);
	int result = 0;
	sscanf(buffer,"%d",&result);
	StdOut(" using baud rate %d... ",result);

	serialparams.BaudRate = result;
	serialparams.ByteSize = 8;
	serialparams.StopBits = ONESTOPBIT;
	serialparams.Parity = NOPARITY;

	if(!SetCommState(hSerial,&serialparams)) {
		StdErr("Error setting state!\r\n");
		return FALSE;
	}

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;

	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if(!SetCommTimeouts(hSerial, &timeouts)) {
		StdErr("Error setting timeout state!\r\n");
		return FALSE;
	}
	
	StdOut("Succes!\r\n");
	return TRUE;
}

// Lees de baudrate en COM-port uit en open de COM-poortverbinding
void ConnectToRobot() {
	char port[30] = "\\\\.\\COM";
	SendMessage(Com,WM_GETTEXT,sizeof(port),(LPARAM)&port[strlen(port)]);
	StdOut("Connecting to %s... ",port);
	ComHandle = CreateFile(port,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(ComHandle == INVALID_HANDLE_VALUE) {
		if(GetLastError()== ERROR_FILE_NOT_FOUND) {
			StdErr("Port does not exist\r\n");
		} else {
			StdErr("Error\r\n");
		}
	} else {
		StdOut("Succes!\r\nConfiguring port... ");
		connected = InitInterface(ComHandle);
	}
}

// Teken de kaart als het veld waar de kaart staat opnieuw getekend moet
// worden of als de route is veranderd.
void PaintMap(HDC dc) {
	int border = 15;
	
	// Eerst is uitvinden hoe groot het veld is waarop we gaan tekenen
	RECT rc;
	GetClientRect(hwnd,&rc); // Hoe groot is het tekenbare vensterdeel?
	rc.bottom -= border + 220; // log
	rc.top += border;
	rc.left += border;
	rc.right -= border + 150; // rechterbalk
	
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	
	int squarelen = (height - height%10)/10;
	squarelen = min(squarelen,(width - width%10)/10);

	if(glassenabled) {
		SetTextColor(dc,RGB(255,255,255));
	} else {
		SetTextColor(dc,RGB(0,0,0));
	}
	
	// Met pennen teken je randjes van vlakken
	HPEN minepen = CreatePen(PS_SOLID,2,RGB(255,0,0));	
	HPEN roadhighlightpen = CreatePen(PS_SOLID,1,RGB(0,255,0));	
	HPEN cityhighlightpen = CreatePen(PS_SOLID,5,RGB(255,128,0));
	HPEN currenthighlightpen = CreatePen(PS_SOLID,5,RGB(34,100,221));
	
	// Met een kwast vul je een vlak in (paint bucket-idee)
	HBRUSH roadhighlightbrush = CreateSolidBrush(RGB(0,255,0));
	
	// Maak een transparante kwast aan
	LOGBRUSH brush;
	memset(&brush,0,sizeof(LOGBRUSH));
	brush.lbStyle = BS_HOLLOW;
	brush.lbColor = RGB(255,128,0);
	HBRUSH cityhighlightbrush = CreateBrushIndirect(&brush);

	// Selecteer de goeie pennen en kwasten
	SelectObject(dc,GetStockObject(BLACK_PEN));
	SelectObject(dc,GetStockObject(BLACK_BRUSH));
	SelectObject(dc,staticfont);
	
	// Tekst heeft een transparante achtergrond
	SetBkMode(dc,TRANSPARENT);

	// Wandel de stedenlist af (linked list)
	List* stedenwalker = map;
	while(stedenwalker != NULL) {
		City* thiscity = (City*)stedenwalker->data;
		
		// Wandel de wegenlist af van deze stad (linked list)
		List* wegenwalker = thiscity->roads;
		while(wegenwalker != NULL) {
			
			Road* thisroad = wegenwalker->data;
			City* destination = thisroad->destination;

			// De wegen van/naar deze stad kleurtje geven
			if(thiscity->highlighted && destination->highlighted) {
				SelectObject(dc,roadhighlightpen);
				SelectObject(dc,roadhighlightbrush);
			}
			
			// Teken de weg als rechthoek (met de geselecteerde Pen en Brush)
			RECT offset;
			if(thiscity->x < destination->x) { // naar rechts
				offset.left = 2;
				offset.top = 2;
				offset.right = -3;
				offset.bottom = -3;
			} else if(thiscity->x > destination->x) { // naar links
				offset.left = -3;
				offset.top = -3;
				offset.right = 2;
				offset.bottom = 2;
			} else if(thiscity->y < destination->y) {  // naar beneden
				offset.left = 2;
				offset.top = 2;
				offset.right = -3;
				offset.bottom = -3;
			} else if(thiscity->y > destination->y) { // naar boven
				offset.left = -3;
				offset.top = -3;
				offset.right = 2;
				offset.bottom = 2;
			}
			if(!thisroad->disabled)
				Rectangle(dc,
					border + thiscity->x*squarelen + offset.left,
					border + thiscity->y*squarelen + offset.top,
					border + destination->x*squarelen + offset.right,
					border + destination->y*squarelen + offset.bottom);
		
			// En weer de standaardpen en -brush selecteren
			SelectObject(dc,GetStockObject(BLACK_PEN));
			SelectObject(dc,GetStockObject(BLACK_BRUSH));
			
			wegenwalker = wegenwalker->next;
		}
		
		// Kleurtje van de stad zelf
		if(thiscity->highlighted) {
			SelectObject(dc,roadhighlightpen);
			SelectObject(dc,roadhighlightbrush);	
		}
		
		// Teken het stadblokje zelf
		Rectangle(dc,
			border + thiscity->x*squarelen - 3,
			border + thiscity->y*squarelen - 3,
			border + thiscity->x*squarelen + 2,
			border + thiscity->y*squarelen + 2);
		
		// Teken de mijnplek
		if(thiscity->name[0] == 'M') {
			Ellipse(dc,
				border + thiscity->x*squarelen - 7,
				border + thiscity->y*squarelen - 7,
				border + thiscity->x*squarelen + 6,
				border + thiscity->y*squarelen + 6);
			if(thiscity->hasmine) {
				
				// Teken de mijn
				
				// We gebruiken een speciale rode pen
				SelectObject(dc,minepen);
				
				// MoveTo zet een imaginaire cursor op een pixel neer
				MoveToEx(dc,
					border + thiscity->x*squarelen - 5,
					border + thiscity->y*squarelen - 5,NULL);
					
				// LineTo tekent vanaf dat punt naar een ander punt een lijntje met Pen
				LineTo(dc,
					border + thiscity->x*squarelen + 5,
					border + thiscity->y*squarelen + 5);
				MoveToEx(dc,
					border + thiscity->x*squarelen + 5,
					border + thiscity->y*squarelen - 5,NULL);
				LineTo(dc,
					border + thiscity->x*squarelen - 5,
					border + thiscity->y*squarelen + 5);
					
				// En pak de standaardpen er weer bij
				SelectObject(dc,GetStockObject(BLACK_PEN));	
			}
		} else {

			// Teken de stadnaam
			TextOut(dc,
				border + thiscity->x*squarelen + 8,
				border + thiscity->y*squarelen + 6,
				thiscity->name,strlen(thiscity->name));
		}
		
		// Teken een rondje van de huidige positie
		if(thiscity == current) {
			
			SelectObject(dc,currenthighlightpen);
			SelectObject(dc,cityhighlightbrush);

			Ellipse(dc,
				border + thiscity->x*squarelen - 9,
				border + thiscity->y*squarelen - 9,
				border + thiscity->x*squarelen + 8,
				border + thiscity->y*squarelen + 8);
		} else if(thiscity == from ||
		   thiscity == to1 ||
		   thiscity == to2 ||
		   thiscity == to3) {
				
			SelectObject(dc,cityhighlightpen);
			SelectObject(dc,cityhighlightbrush);
			
			Ellipse(dc,
				border + thiscity->x*squarelen - 9,
				border + thiscity->y*squarelen - 9,
				border + thiscity->x*squarelen + 8,
				border + thiscity->y*squarelen + 8);
		}

		// En weer de standaardpen en -brush selecteren
		SelectObject(dc,GetStockObject(BLACK_PEN));
		SelectObject(dc,GetStockObject(BLACK_BRUSH));
	
		stedenwalker = stedenwalker->next;
	}
	
	// En ruim de gemaakte troep op
	DeleteObject(minepen);
	DeleteObject(roadhighlightpen);
	DeleteObject(cityhighlightpen);
	DeleteObject(currenthighlightpen);
	
	DeleteObject(roadhighlightbrush);
	DeleteObject(cityhighlightbrush);
}

// Deze functie zal voor de City* thiscity de afstanden tot de buursteden
// berekenen en zal daarna zichzelf aanroepen voor de volgende stad.
// Hiervoor wordt Dijkstra's algoritme gebruikt.
void ConsiderNeighbors(City* thiscity) {

	// Wandel alle wegen van deze stad af
	List* wegenwalker = thiscity->roads;
	while(wegenwalker != NULL) {
		
		Road* thisroad = wegenwalker->data;
		City* destination = thisroad->destination;
		
		// Mijnen en al bezochte steden worden genegeerd
		if(!destination->visited && !destination->hasmine && !thisroad->disabled) {
			destination->totaldistance = min(destination->totaldistance,thiscity->totaldistance + 1);
		}
		
		wegenwalker = wegenwalker->next;
	}
	
	// Deze stad is klaar!
	thiscity->visited = TRUE;
}

void ResetMap() {
	List* stedenwalker = map;
	while(stedenwalker != NULL) {
		City* thiscity = (City*)stedenwalker->data;
		
		thiscity->totaldistance = 0xFFFFFFFF;
		thiscity->highlighted = FALSE;
		thiscity->visited = FALSE;

		stedenwalker = stedenwalker->next;
	}
}

void FindShortestRoute(City* from,City* to) {
	
	BOOL foundroute = FALSE;
	
	from->totaldistance = 0;
	from->highlighted = TRUE;
	
	City* currentcity = from;	
	while(currentcity) {
		
		ConsiderNeighbors(currentcity);
		
		if(currentcity == to) {
			foundroute = TRUE;
			break; // KLAAR
		}
		
		// Neem even aan dat we klaar zijn
		currentcity = NULL;
		unsigned int leastdistance = 0xFFFFFFFF - 1;
	
		List* stedenwalker = map;
		while(stedenwalker != NULL) {
			
			City* thiscity = (City*)stedenwalker->data;
			
			if(!thiscity->visited && thiscity->totaldistance < leastdistance) {
				leastdistance = thiscity->totaldistance;
				currentcity = thiscity;
			}
			
			stedenwalker = stedenwalker->next;
		}
	}
	
	if(foundroute) {
		
		// Vind dan waar we heengegaan zijn
		City* backtrace = to;
		while(backtrace->totaldistance > 0) {
			
			backtrace->highlighted = TRUE;
			
			List* wegenwalker = backtrace->roads;
			while(wegenwalker != NULL) {
				Road* thisroad = wegenwalker->data;
				City* destination = thisroad->destination;
				
				if(destination->visited) {
					if(destination->totaldistance + 1 == backtrace->totaldistance) {
						backtrace = destination;
						break;
					}
				}
	
				wegenwalker = wegenwalker->next;
			}
		}
	}
		
	currentto = to;
}

BOOL CALLBACK FocusProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_KEYDOWN: {
			switch(wParam) {
				case VK_LEFT: {
					if(!leftarrow) {
						leftarrow = TRUE;
						UpdateSignal();
					}
					break;
				}
				case VK_RIGHT: {
					if(!rightarrow) {
						rightarrow = TRUE;
						UpdateSignal();
					}
					break;
				}
				case VK_DOWN: {
					if(!downarrow) {
						downarrow = TRUE;
						UpdateSignal();
					}
					break;
				}
				case VK_UP: {
					if(!uparrow) {
						uparrow = TRUE;
						UpdateSignal();
					}
					break;
				}
				case 'M': {
					mario = !mario;
					UpdateSignal();
					break;
				}
			}
			break;
		}
		case WM_KEYUP: {
			switch(wParam) {
				case VK_LEFT: {
					leftarrow = FALSE;
					UpdateSignal();
					break;
				}
				case VK_RIGHT: {
					rightarrow = FALSE;
					UpdateSignal();
					break;
				}
				case VK_DOWN: {
					downarrow = FALSE;
					UpdateSignal();
					break;
				}
				case VK_UP: {
					uparrow = FALSE;
					UpdateSignal();
					break;
				}
			}
			break;
		}
		default:
			return CallWindowProc(OldFocusProc,hwnd,Message,wParam,lParam);
	}
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_COMMAND: {
			if(HIWORD(wParam) == EN_CHANGE) {
				switch(LOWORD(wParam)) {
					case ID_CITYFROM: {
						recalcroute = TRUE;
						InvalidateRgn(hwnd,NULL,TRUE);
						break;
					}
					case ID_CITY1TO: {
						recalcroute = TRUE;
						InvalidateRgn(hwnd,NULL,TRUE);
						break;
					}
					case ID_CITY2TO: {
						recalcroute = TRUE;
						InvalidateRgn(hwnd,NULL,TRUE);
						break;
					}
					case ID_CITY3TO: {
						recalcroute = TRUE;
						InvalidateRgn(hwnd,NULL,TRUE);
						break;
					}
				}
			}
			switch(wParam) {
				case ID_UPDATE: {
					CloseHandle(ComHandle);
					ConnectToRobot();
					break;
				}
			}
			break;
		}
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hwnd,&ps);

			if(recalcroute) {
				
				current = NULL;
				next = NULL;
				
				from = NULL;
				to1 = NULL;
				to2 = NULL;
				to3 = NULL;
			
				char buffer[CITYNAMEMAXLEN + 1];
				
				SendMessage(CityFrom,WM_GETTEXT,(WPARAM)CITYNAMEMAXLEN,(LPARAM)buffer);
				from = find_city(map,buffer);
				SendMessage(City1To,WM_GETTEXT,(WPARAM)CITYNAMEMAXLEN,(LPARAM)buffer);
				to1 = find_city(map,buffer);
				SendMessage(City2To,WM_GETTEXT,(WPARAM)CITYNAMEMAXLEN,(LPARAM)buffer);
				to2 = find_city(map,buffer);
				SendMessage(City3To,WM_GETTEXT,(WPARAM)CITYNAMEMAXLEN,(LPARAM)buffer);
				to3 = find_city(map,buffer);
				
				ResetMap();
				
				if(from && to1) {
					
					current = from;
					
					FindShortestRoute(from,to1);
					
					List* wegenwalker = current->roads;
					while(wegenwalker != NULL) {
				
						Road* thisroad = wegenwalker->data;
						City* destination = thisroad->destination;
				
						if(destination->highlighted) {	
							next = destination;
							break;
						}
						wegenwalker = wegenwalker->next;
					}
				}
			}
			
			// Teken de map
			PaintMap(dc);
			
			EndPaint(hwnd,&ps);
			
			recalcroute = FALSE;
			break;
		}
		case WM_CREATE: {
			
			// Alle vensters binnen het hoofdvenster aanmaken
			Logveld = CreateWindow("EDIT","",WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_VSCROLL,0,0,0,0,hwnd,(HMENU)ID_LOG,GetModuleHandle(NULL),NULL);
			
			MarioText = CreateWindow("STATIC","Mario is OFF",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,(HMENU)ID_MARIO,GetModuleHandle(NULL),NULL);	
			TurnText = CreateWindow("STATIC","Forward",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,(HMENU)ID_TURN,GetModuleHandle(NULL),NULL);
			
			Focus = CreateWindow("BUTTON","Focus!",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,(HMENU)ID_FOCUS,GetModuleHandle(NULL),NULL);
			OldFocusProc = (WNDPROC)GetWindowLongPtr(Focus,GWLP_WNDPROC);
			SetWindowLongPtr(Focus,GWLP_WNDPROC,(LONG_PTR)FocusProc);

			ComText = CreateWindow("STATIC","COM port",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,0,GetModuleHandle(NULL),NULL);
			Com = CreateWindow("EDIT","17",WS_BORDER|WS_CHILD|WS_VISIBLE|ES_NUMBER,0,0,0,0,hwnd,(HMENU)ID_BAUD,GetModuleHandle(NULL),NULL);		
			
			BaudText = CreateWindow("STATIC","Baud rate",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,0,GetModuleHandle(NULL),NULL);
			Baud = CreateWindow("EDIT","9600",WS_BORDER|WS_CHILD|WS_VISIBLE|ES_NUMBER,0,0,0,0,hwnd,(HMENU)ID_BAUD,GetModuleHandle(NULL),NULL);
			
			Update = CreateWindow("BUTTON","Reconnect",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,(HMENU)ID_UPDATE,GetModuleHandle(NULL),NULL);
			
			CityFromText = CreateWindow("STATIC","From city",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,0,GetModuleHandle(NULL),NULL);
			CityFrom = CreateWindow("EDIT","S09",WS_BORDER|WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,(HMENU)ID_CITYFROM,GetModuleHandle(NULL),NULL);

			City1ToText = CreateWindow("STATIC","To city 1",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,0,GetModuleHandle(NULL),NULL);
			City1To = CreateWindow("EDIT","S08",WS_BORDER|WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,(HMENU)ID_CITY1TO,GetModuleHandle(NULL),NULL);

			City2ToText = CreateWindow("STATIC","To city 2",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,0,GetModuleHandle(NULL),NULL);
			City2To = CreateWindow("EDIT","S07",WS_BORDER|WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,(HMENU)ID_CITY2TO,GetModuleHandle(NULL),NULL);
			
			City3ToText = CreateWindow("STATIC","To city 3",WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,0,GetModuleHandle(NULL),NULL);
			City3To = CreateWindow("EDIT","S06",WS_BORDER|WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,(HMENU)ID_CITY3TO,GetModuleHandle(NULL),NULL);
			
			// Mooi fontje
			HDC dc = GetDC(hwnd);
			logfont = CreateFont(-MulDiv(12,GetDeviceCaps(dc, LOGPIXELSY),72),0,0,0,0,0,0,0,0,0,0,0,0,"Consolas");
			staticfont = CreateFont(-MulDiv(10,GetDeviceCaps(dc, LOGPIXELSY),72),0,0,0,0,0,0,0,0,0,0,0,0,"MS Sans Serif");
			interfacefont = CreateFont(-MulDiv(10,GetDeviceCaps(dc, LOGPIXELSY),72),0,0,0,0,0,0,0,0,0,0,0,0,"Segoe UI");
			ReleaseDC(hwnd,dc);
			
			SendMessage(Logveld,      WM_SETFONT,(WPARAM)logfont,0);
			SendMessage(MarioText,    WM_SETFONT,(WPARAM)staticfont,0);
			SendMessage(TurnText,     WM_SETFONT,(WPARAM)staticfont,0);
			SendMessage(Focus,        WM_SETFONT,(WPARAM)interfacefont,0);
			SendMessage(BaudText,     WM_SETFONT,(WPARAM)staticfont,0);
			SendMessage(Baud,         WM_SETFONT,(WPARAM)interfacefont,0);
			SendMessage(ComText,      WM_SETFONT,(WPARAM)staticfont,0);
			SendMessage(Com,          WM_SETFONT,(WPARAM)interfacefont,0);
			SendMessage(CityFromText, WM_SETFONT,(WPARAM)staticfont,0);
			SendMessage(CityFrom,     WM_SETFONT,(WPARAM)interfacefont,0);
			SendMessage(City1ToText,  WM_SETFONT,(WPARAM)staticfont,0);
			SendMessage(City1To,      WM_SETFONT,(WPARAM)interfacefont,0);
			SendMessage(City2ToText,  WM_SETFONT,(WPARAM)staticfont,0);
			SendMessage(City2To,      WM_SETFONT,(WPARAM)interfacefont,0);
			SendMessage(City3ToText,  WM_SETFONT,(WPARAM)staticfont,0);
			SendMessage(City3To,      WM_SETFONT,(WPARAM)interfacefont,0);
			SendMessage(Update,       WM_SETFONT,(WPARAM)interfacefont,0);

			// Kijken of we Vista en Aero gebruiken (of nieuwer)
			OSVERSIONINFO osvi;
			memset(&osvi,0,sizeof(osvi));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx(&osvi);
			if(osvi.dwMajorVersion >= 6) {
				DwmIsCompositionEnabled(&glassenabled);
			}
			
			if(glassenabled) {
				tkey = RGB(250,250,251);
				transparentbrush = CreateSolidBrush(tkey);
				SetLayeredWindowAttributes(hwnd,tkey,0,LWA_COLORKEY);
				MARGINS margins = {-1,-1,-1,-1};
				DwmExtendFrameIntoClientArea(hwnd,&margins);
			} else {
				SetLayeredWindowAttributes(hwnd,tkey,255,LWA_ALPHA);	
			}
			
			// Open de kaart/plattegrond met naam dikzak met City en Road
			char filename[] = "dikzak";
			StdOut("Opening map file '%s'... ",filename);
			FILE* mapfile = fopen(filename,"r");
			if(mapfile) {
				StdOut("Success!\r\n");
				
				StdOut("Processing map file '%s'... ",filename);
				map = create_map(mapfile);
				if(map) {
					StdOut("Success!\r\n");
				} else {
					StdErr("No map data found\n");
				}
			} else {
				StdErr("Failed\r\n");
			}
			fclose(mapfile);
			
			// Probeer COM te lezen
			ConnectToRobot();
			
			// Maak leesthread
			ComReader = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ComReaderThread,NULL,0,NULL);
			break;
		}
		case WM_CTLCOLORSTATIC: {
			if(glassenabled) {
				SetTextColor((HDC)wParam,RGB(255,255,255));
				SetBkColor((HDC)wParam,tkey);
			   	return (INT_PTR)transparentbrush;
			} else {
				return DefWindowProc(hwnd, Message, wParam, lParam);
			}
		}
		case WM_ERASEBKGND: {
			if(glassenabled) {
				RECT rc;
				GetClientRect(hwnd, &rc);
				FillRect((HDC)wParam, &rc, transparentbrush);
				return 1;
			} else {
				return DefWindowProc(hwnd, Message, wParam, lParam);
			}
		}
		case WM_SIZE: {
			int height = HIWORD(lParam);
			int width = LOWORD(lParam);

			SetWindowPos(Logveld,NULL,5,height-200,width-127,195,SWP_NOZORDER);
			
			SetWindowPos(MarioText,NULL,    width-115, 5,110,25,SWP_NOZORDER);
			SetWindowPos(TurnText,NULL,     width-115, 30,110,25,SWP_NOZORDER);
			
			SetWindowPos(Focus,NULL,        width-115,55,110,height-465,SWP_NOZORDER);

			SetWindowPos(ComText,NULL,      width-115,height-405,110,25,SWP_NOZORDER);
			SetWindowPos(Com,NULL,          width-115,height-380,110,25,SWP_NOZORDER);
			SetWindowPos(BaudText,NULL,     width-115,height-345,110,25,SWP_NOZORDER);
			SetWindowPos(Baud,NULL,         width-115,height-320,110,25,SWP_NOZORDER);
			SetWindowPos(Update,NULL,       width-115,height-280,110,35,SWP_NOZORDER);
			
			SetWindowPos(CityFromText,NULL, width-115,height-235,110,25,SWP_NOZORDER);
			SetWindowPos(CityFrom,NULL,     width-115,height-210,110,25,SWP_NOZORDER);
			
			SetWindowPos(City1ToText,NULL,  width-115,height-175,110,25,SWP_NOZORDER);
			SetWindowPos(City1To,NULL,      width-115,height-150,110,25,SWP_NOZORDER);

			SetWindowPos(City2ToText,NULL,  width-115,height-115,110,25,SWP_NOZORDER);
			SetWindowPos(City2To,NULL,      width-115,height-90, 110,25,SWP_NOZORDER);
			
			SetWindowPos(City3ToText,NULL,  width-115,height-55, 110,25,SWP_NOZORDER);
			SetWindowPos(City3To,NULL,      width-115,height-30, 110,25,SWP_NOZORDER);

			InvalidateRgn(hwnd,NULL,FALSE);
			break;
		}
		case WM_CLOSE: {
			CloseHandle(ComHandle);
			DeleteObject(logfont);
			DeleteObject(staticfont);
			DeleteObject(interfacefont);
			DeleteObject(transparentbrush);
			
			// Map verwijderen
			if(map != NULL)
				delete_map(map);

			DestroyWindow(hwnd);
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

// Hieronder wordt het venster geregistreerd en gemaakt...
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	MSG Msg;

	// Maak een struct met eigenschappen van het venster aan
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(IDI_MAINICON),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	wc.hIconSm		 = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(IDI_MAINICON),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL,"Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	// Maak het venster aan
	hwnd = CreateWindowEx(WS_EX_LAYERED,"WindowClass","Robotbesturing - Not connected",WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		900,
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL,"Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	UpdateSignal();
	
	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
