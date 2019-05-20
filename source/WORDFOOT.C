// Versuch der Umsetzung von Wortfu·ball

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#define WinPosX 100    // Position und Grî·e des Fensters
#define WinPosY 40
#define WinLenX 471
#define WinLenY 390


#define PLX 202       // Anfangskoordinaten der Figuren
#define PLY 156
#define LANGX 85        // LÑnge und Breite zum Abdecken der Figuren
#define LANGY 20

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os2.h>
#include "wordfoot.h"

// Globale Variablen fÅr Verwaltung der Wîrter

int      k,l,wahr;   

// Speicherung der Wîrter

struct  ein
  {
        char e_aufg[40];
        char e_such[50];
        char e_antw[30];
  } eintrag[1000];

// Variablen fÅr Datei

PVOID   speicher;
PCHAR   datei;

POINTL  text1_pos = {40, 55},          // Position der Spielstandtexte
        text2_pos = {350, 55},
        spieler = {PLX,PLY};          // Anfangskoordinaten der Spieler

// Bereich der Spiele und Ball

RECTL  bereich = {PLX,PLY,PLX + LANGX, PLY + LANGY};     

int     lentext1 = {13},                //LÑnge der Spielstandtexte
        lentext2 = {13},
        wahr = {0},                     //Animation vor oder zurÅck
        zufall={0},                          // Algorithmus
        punkte_comp = {0},              // Spielstand
        punkte_play = {0},
        spiel_akt = {2};                // Position der Spieler

HAB     hab_t;

// Hilfetext fÅr HilfemenÅ

char HText[800]={"\nSorry that there is a short introduction only. You have to select 'Open' from the 'File' menu to be able to play. There you have to open a file and then you can start.\n\nWith the 'Option' dialogue you can select whether the words are chosen by chance or by the sequence of the file. The options are not saved."};

// Fensterprozeduren

MRESULT EXPENTRY ClientWndProc(HWND hwnd,ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY OptionProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

int main (VOID)
{
        HAB     hab;
        HMQ     hmq;
        QMSG    qmsg;
        HWND    hwndClientFrame,
                  hwndClient;
        ULONG   ClientStyleFlags = FCF_TITLEBAR |
                        FCF_SYSMENU |
                        FCF_BORDER |
                        FCF_MINBUTTON |
                        FCF_TASKLIST |
                        FCF_ICON |
                        FCF_MENU |
			FCF_ACCELTABLE;

// Initialisierung

hab = WinInitialize (0); 
hmq = WinCreateMsgQueue (hab,0);

// Anmeldung und Erzeugen des Fensters

WinRegisterClass (hab,
                "WordFootball",
                ClientWndProc,
                CS_SIZEREDRAW,
                0);
hwndClientFrame = WinCreateStdWindow (HWND_DESKTOP,
                        WS_VISIBLE,
                        &ClientStyleFlags,
                        "WordFootball",
                        "WordFootball",
                        WS_VISIBLE,
                        (HMODULE)0,
                        ID_ClientWindow,
                        &hwndClient);

// Position und Grî·e des Fensters

WinSetWindowPos (hwndClientFrame,
                HWND_TOP,
                WinPosX, WinPosY,
                WinLenX, WinLenY,
                SWP_MOVE | SWP_SIZE | SWP_ACTIVATE);

// Nachrichten empfangen

while (WinGetMsg (hab,&qmsg,0L,0,0))
        WinDispatchMsg (hab,&qmsg);

// Fenster abmelden und entfernen

WinDestroyWindow (hwndClientFrame);
WinDestroyMsgQueue (hmq);
WinTerminate (hab);
return 0;
}

// Fensterprozedur

MRESULT EXPENTRY ClientWndProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
        HPS     hps;
        static HWND    hwndFrame, hwndMenu;
        static  HBITMAP  hbm;
        POINTL  koord;
        ULONG   dlg_res, option_res;
        int     it1, it2,i,j;                     //ZÑhler fÅr Timer(it) + ZÑhler fÅr Datei(i,j,k)
        static char filename[CCHMAXPATH];

        HFILE   hfile;
        ULONG   aktion, laenge;
        PCHAR  pointer, zwischen;
        char stand[50]={""};
        koord.x = 20;                    //fÅr Koordinaten der Bitmap
        koord.y = 20;
        
        switch (msg)
        {
             case WM_CREATE:
                // Handle zum Rahmenfenster holen
                hwndFrame = WinQueryWindow (hwnd, QW_PARENT);
                // Handle zum MenÅ holen
                hwndMenu = WinWindowFromID (hwndFrame, FID_MENU);
                // Laden der Bitmap
                hps = WinGetPS(hwnd);
                hbm = GpiLoadBitmap(hps,NULLHANDLE,IDB_Platz,0L,0L);
                WinReleasePS(hps);
                break;

            case WM_SIZE:
                break;

            case WM_PAINT:
                // Zeichenen der Bitmap
                hps = WinBeginPaint (hwnd,NULLHANDLE,NULL);
                GpiErase (hps);
                WinDrawBitmap(hps,hbm,NULL,&koord,0L,0L,DBM_NORMAL);
                UpdateStand(hps);
                UpdateSpieler(hps);
                WinEndPaint (hps);
                break;

            case WM_COMMAND:
                switch (SHORT1FROMMP(mp1))
                   {
                   case IDM_Play:
                        // Aufruf des Rate-Dialogs
                        dlg_res = WinDlgBox (HWND_DESKTOP, hwnd, DlgProc,
                                (HMODULE) 0, ID_Dialog, NULL);
                        if (dlg_res == DID_CANCEL)
                           {
                           dlg_res = WinMessageBox(HWND_DESKTOP, hwnd,
                                "Do you really want to quit?","Play",
                                0, MB_YESNO | MB_ICONQUESTION);
                           if (dlg_res == MBID_NO) {
                                WinSendMsg(hwnd,WM_COMMAND,
                                        MPFROMSHORT(IDM_Play),NULL);                        
                                } 
                                else {
                                       sprintf (stand, "\nThe score was: Player %01d - Computer %01d", punkte_play, punkte_comp);
                                       WinMessageBox (HWND_DESKTOP, hwnd,                  
                                          stand,                        
                                          "Game quit!",                                         
                                          0, MB_OK | MB_INFORMATION);                     
                                        spieler.x = PLX;
                                        spieler.y = PLY;
                                        punkte_comp=0;
                                        punkte_play=0;
                                        l=0;
                                        WinInvalidateRect(hwnd,NULL,TRUE);
                                }
                           }
                           else { WinSendMsg(hwnd,WM_COMMAND,MPFROMSHORT(ID_Ani),NULL);
                              }
                        break;
                     
                   case IDM_Options:
                        // Aufruf des Optionen-Dialogs
                        option_res = WinDlgBox (HWND_DESKTOP, hwnd, OptionProc,
                                (HMODULE) 0, ID_Option, NULL);
                        if (option_res == DID_OK)
                           {
                            spieler.x = PLX;
                            spieler.y = PLY;
                            punkte_comp=0;
                            punkte_play=0;
                            l=0;
                          WinInvalidateRect(hwnd,NULL,TRUE);
                           }
                        break;
                
                   case ID_Ani:
                        WinStartTimer(hab_t,hwnd,ID_Timer1,200);
                        WinStartTimer(hab_t,hwnd,ID_Timer2,1000);
                        break;
                 
                   case IDM_Choose:
                         for (i=0; i<=100 ; i++) {        
                            strcpy(eintrag[i].e_aufg, "");
                            strcpy(eintrag[i].e_such, "");
                            strcpy(eintrag[i].e_antw, "");
                         } /* endfor */
                         Dateiliste(hwnd, "Choose file", "*.dta", filename);
                         if (DosAllocMem(&speicher, 4096, PAG_WRITE | PAG_READ | PAG_COMMIT) == 0)
                            {
                            datei=speicher;
                            if (DosOpen(filename, &hfile, &aktion, 0, 0L, OPEN_ACTION_OPEN_IF_EXISTS,
                               OPEN_SHARE_DENYWRITE, 0L) ==0)
                            {
                               DosRead(hfile, datei, 4096,&laenge);
                               DosClose(hfile); 
                               pointer=datei;
			                zwischen=pointer;
                               k=0; j=-1;
                           // Schleife bis zum Ende der Datei
         			           for (i=0; i<=laenge; i++) {
                                  pointer++;
                           // Wort bis zum Zeichen |        
                                  if ((CHAR)*pointer==124) {
				                 strncat(eintrag[k].e_aufg,zwischen,i-j);
                                     *pointer=0;
                                     pointer++;
                                      zwischen=pointer;
                                     j=i;
                                     }
                           // Wort von Zeichen | bis #
                                  if ((CHAR)*pointer==35) {
                                     strncat(eintrag[k].e_such,zwischen,i-j);
                                     *pointer=0;
                                     pointer++;
                                     zwischen=pointer;
                                     j=i;
                                     } 
                           // Wort von Zeichen # bis \r\n
                                  if ((CHAR)*pointer==13) {
                                     strncat(eintrag[k].e_antw,zwischen,i-j);
                                     *pointer=0;
                                     pointer+=2;
                                     zwischen=pointer;
                                     j=i;
                                     k++;
                                     } 
                                  } 
                               WinSendMsg(hwndMenu,MM_SETITEMATTR,
                                                   MPFROM2SHORT(IDM_Play,TRUE),
                                                   MPFROM2SHORT(MIA_DISABLED,0)); 
                            } else {
                                 WinMessageBox (HWND_DESKTOP, hwnd,                                         
                                   "File could not be opened.\nCheck file attributes or if file is already in use.",              
                                   "Error",                                                        
                                   0, MB_OK | MB_ERROR);                                        
                            } 
                         } else {
                              WinMessageBox (HWND_DESKTOP, hwnd,                                         
                                 "Memory could not be allocated.\nCheck if there is enough memory.",              
                                 "Error",                                                        
                                 0, MB_OK | MB_ERROR);                                        
                         } 
                         DosFreeMem(speicher);
                         l=0;
                         break;
                      
                   case IDM_Exit:
                        // Programm durch Message beenden
                        WinPostMsg (hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
                        break;
                     
                   case IDM_About:
                        WinMessageBox (HWND_DESKTOP, hwnd, 
                                "\n          WordFootball V 1.0 by Torsten Schulz",
                                "About...",
                                0, MB_OK | MB_INFORMATION);
                        break;
                   }
                break;

            case WM_CLOSE:
                WinPostMsg (hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
                break;

            case WM_DESTROY:
                // Lîschen der geladenen Bitmap
                GpiDeleteBitmap(hbm);
                break;

            case WM_HELP:
                WinMessageBox (HWND_DESKTOP, hwnd,
                        HText,
                        "Help",
                        0, MB_OK | MB_INFORMATION);
                break;

            case WM_TIMER:
                switch (SHORT1FROMMP(mp1)) {
                     case ID_Timer1:
                        if (wahr==0) {
                                for (it1 = 0; it1 < 5; it1++) {
                                        bereich.xLeft = spieler.x - 5;
                                        bereich.yBottom = spieler.y - 5;
                                        bereich.xRight = spieler.x + LANGX;
                                        bereich.yTop = spieler.y + LANGY;
                                        spieler.x = spieler.x + it1;
                                        WinInvalidateRect(hwnd,&bereich,TRUE);
                                        }
                                }
                            else {
                                for (it1 = 0; it1 < 5; it1++) {
                                        bereich.xLeft = spieler.x - 10;
                                        bereich.yBottom = spieler.y - 5;
                                        bereich.xRight = spieler.x + LANGX;
                                        bereich.yTop = spieler.y + LANGY;
                                        spieler.x = spieler.x - it1;
                                        WinInvalidateRect(hwnd,&bereich,TRUE);
                                        }
                          }
                        break;
                     
                     case ID_Timer2:
                        for (it2 = 0; it2 < 3 ; it2++) {
                        WinStopTimer(hab_t,hwnd,ID_Timer1+it2);
                        } 
                        if (spieler.x < 88) {
                                punkte_play++;
                                spieler.x = PLX;
                                spieler.y = PLY;
                                WinInvalidateRect(hwnd,NULL,TRUE);
                                WinMessageBox (HWND_DESKTOP, hwnd,
                                        "Wow, you scored!!",
                                        "",
                                        0, MB_OK | MB_WARNING);
                                }
                        if (spieler.x > 288) {
                                punkte_comp++;
                                spieler.x = PLX;
                                spieler.y = PLY;
                                WinInvalidateRect(hwnd,NULL,TRUE);
                                WinMessageBox (HWND_DESKTOP, hwnd,
                                        "Computer scored!",
                                        "",
                                        0, MB_OK | MB_WARNING);
                                }
                        if (zufall==1){
                          l++;
                          if (l==k) {
                            sprintf (stand, "\nThe score was: Player %01d - Computer %01d", punkte_play, punkte_comp);
                            WinMessageBox (HWND_DESKTOP, hwnd,                  
                                   stand,                        
                                   "Game over!!!",                                         
                                   0, MB_OK | MB_INFORMATION);                     
                            spieler.x = PLX;
                            spieler.y = PLY;
                            punkte_comp=0;
                            punkte_play=0;
                            l=0;
                            WinInvalidateRect(hwnd,NULL,TRUE);
                             }
                             else {
                               WinSendMsg(hwnd,WM_COMMAND,MPFROMSHORT(IDM_Play),NULL);
                               }
                           } else {
                             l=k-rand()*(k-1)/32767;
                             WinSendMsg(hwnd,WM_COMMAND,MPFROMSHORT(IDM_Play),NULL);
                           }
                        break;
                } 

            default:
                return WinDefWindowProc (hwnd,msg,mp1,mp2);
                break;
        }
return (MRESULT)FALSE;
}


MRESULT EXPENTRY DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
HWND hwndAufg, hwndWort, hwndEing;

char   eing[30]; 

switch(msg) 
   {
      case WM_INITDLG:
        hwndAufg = WinWindowFromID (hwnd, ID_Aufg);                
        WinSetWindowText (hwndAufg, eintrag[l].e_aufg);      
        hwndWort = WinWindowFromID (hwnd, ID_Wort);          
        WinSetWindowText (hwndWort, eintrag[l].e_such);      
        break;
     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1)) 
        {
        case DID_OK:            
          hwndEing = WinWindowFromID (hwnd, ID_Eing);          
          WinQueryWindowText (hwndEing, sizeof(eing),eing);   
          WinDismissDlg (hwnd, DID_OK);
          if (strcmp(eing, eintrag[l].e_antw)) wahr=0; 
	    else wahr=1; 
          break;
        case DID_CANCEL:
           WinDismissDlg (hwnd, DID_CANCEL); 
            break;
         }
        break;           

      default:
                return WinDefDlgProc(hwnd, msg, mp1, mp2);
   }
return (MRESULT)FALSE;
}


MRESULT EXPENTRY OptionProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
switch(msg) 
   {
      case WM_INITDLG:
        if (zufall==0) {
          WinSendDlgItemMsg(hwnd,ID_Zufall,BM_SETCHECK, (MPARAM) 1,NULL);
        } else { 
          WinSendDlgItemMsg(hwnd,ID_Reihe,BM_SETCHECK, (MPARAM) 1,NULL);
        } /* endif */
        break;
     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1)) 
        {
        case DID_OK:            
          zufall=(USHORT) WinSendDlgItemMsg(hwnd,ID_Zufall,BM_QUERYCHECKINDEX,NULL,NULL);
          WinDismissDlg (hwnd, DID_OK);
          break;
        case DID_CANCEL:
           WinDismissDlg (hwnd, DID_CANCEL); 
            break;
         }
        break;           

      default:
                return WinDefDlgProc(hwnd, msg, mp1, mp2);
   }
return (MRESULT)FALSE;
}

VOID UpdateStand(HPS hps)
{
    char buff[13];
// Texte fÅr Spielstand
sprintf (buff, "Computer: %01d  ", punkte_comp);
GpiCharStringAt(hps,&text1_pos, lentext1, buff);
sprintf (buff, "Player: %01d    ", punkte_play);
GpiCharStringAt(hps,&text2_pos, lentext2, buff);
}

VOID UpdateSpieler(HPS hps)
{
    POINTL buff;
    ARCPARAMS kreis = {0,0,0,0};
// Ausgabe von ComputermÑnnchens
GpiSetColor(hps,CLR_RED);
GpiSetLineWidthGeom(hps,7);
GpiBeginPath(hps, 1L);
GpiMove(hps,&spieler);
buff.x = spieler.x;
buff.y = spieler.y + 15;
GpiLine(hps, &buff);
GpiEndPath(hps);
GpiSetLineEnd(hps,LINEEND_ROUND);
GpiStrokePath(hps,1L, 0L);

// Ausgabe des SpielermÑnnchens
GpiSetColor(hps,CLR_BLUE);
GpiSetLineWidthGeom(hps,7);
GpiBeginPath(hps, 1L);
buff.x = spieler.x + 70;
buff.y = spieler.y;
GpiMove(hps,&buff);
buff.x = spieler.x + 70;
buff.y = spieler.y + 15;
GpiLine(hps, &buff);
GpiEndPath(hps);
GpiSetLineEnd(hps,LINEEND_ROUND);
GpiStrokePath(hps,1L, 0L);

// Ausgabe des Balls
GpiSetColor(hps,CLR_BLACK);
GpiSetBackColor(hps, CLR_WHITE);
GpiSetPattern(hps,PATSYM_DENSE7);
GpiSetBackMix(hps,BM_OR);
buff.x = spieler.x +35;
buff.y = spieler.y + 2;
GpiMove(hps,&buff);
kreis.lP = 5;
kreis.lQ = 5;
//kreis. 0, 0}
GpiSetArcParams(hps,&kreis);
GpiFullArc(hps,DRO_OUTLINEFILL, MAKEFIXED(1,0));

// ZurÅcksetzen der Farbe
GpiSetColor(hps,CLR_DEFAULT);
}

VOID Dateiliste(HWND hwnd, PSZ Titel, PSZ Filter, PSZ Auswahl)
{
        FILEDLG FileDlg;
        HWND    hwndDlg;
memset(&FileDlg,0,sizeof(FILEDLG));
FileDlg.cbSize = sizeof(FILEDLG);
FileDlg.fl = FDS_CENTER | FDS_OPEN_DIALOG;
FileDlg.pszTitle = Titel;
FileDlg.pszOKButton = (PSZ) "~Open";
strcpy(FileDlg.szFullFile, Filter);
hwndDlg = WinFileDlg(HWND_DESKTOP, hwnd, &FileDlg);
if ((hwndDlg != NULLHANDLE) && (FileDlg.lReturn == DID_OK)) 
	strcpy(Auswahl,FileDlg.szFullFile);
return;
}
