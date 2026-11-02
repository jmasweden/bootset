/***************************************************************************/
/* THIS CODE IS FREEWARE AND AS SUCH YOU ARE PERMITTED TO DO WHAT YOU WISH */
/* WITH IT. THESE PROGRAMS ARE PROVIDED AS IS WITHOUT ANY WARRANTY,        */
/* EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO FITNESS FOR ANY      */
/* PARTICULAR PURPOSE.                                                     */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/*   Modul:      BOOTSET.CPP, cpp-sorce for customized boot                */
/*                                                                         */
/*   Time:       960203, 261028, 34h                                       */
/*                                                                         */
/*  ---------------------------------------------------------------------  */
/*   960203:     Stole most from QL.CPP. (MA,1h)                           */
/*  ---------------------------------------------------------------------  */
/*   960204:     *A LOT*. (MA,4h)                                          */
/*  ---------------------------------------------------------------------  */
/*   960205:     Added a help screen and fixed so that a timeout of 0      */
/*               would not start the timeout thread. (MA,2h)               */
/*  ---------------------------------------------------------------------  */
/*   960221:     Changed to 16 bits Vio/Kbd and gave timeout thread a      */
/*               larger stack. (MA,2h)                                     */
/*  ---------------------------------------------------------------------  */
/*   960308:     Added all future fields to .INI file (MA, 1h).            */
/*  ---------------------------------------------------------------------  */
/*   960309:     Cursor now hidden during program exec (MA, 1h).           */
/*  ---------------------------------------------------------------------  */
/*   970504:     Added support for shareware version and fixed a few       */
/*               problems with redraws. (MA, 2h).                          */
/*  ---------------------------------------------------------------------  */
/*   970613:     Tiny waitscreen was added and color support. (MA, 2h)     */
/*  ---------------------------------------------------------------------  */
/*   970624:     Fixed some (layout) bugs. (MA, 2h)                        */
/*  ---------------------------------------------------------------------  */
/*   980325:     Moved to STVIO library. (MA, 5h)                          */
/*  ---------------------------------------------------------------------  */
/*   980325:     Hide cursor after help window. (MA, 1h)                   */
/*  ---------------------------------------------------------------------  */
/*   980715:     Just a recompile. (MA, 1h)                                */
/*  ---------------------------------------------------------------------  */
/*   980729:     Moved all texts to english.h. (MA, 2h)                    */
/*  ---------------------------------------------------------------------  */
/*   981012:     Fixed a bug in ShowUnregistered. (MA, 1h)                 */
/*  ---------------------------------------------------------------------  */
/*   260916:     Moved to gcc. (MA, 3h)                                    */
/*  ---------------------------------------------------------------------  */
/*   261028:     Moved to openwatcom. (MA, 4h)                             */
/*  ---------------------------------------------------------------------  */
/***************************************************************************/
#define INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <process.h>

#include "../stlib2/src/st_vio.h"
#include "../stlib2/src/st_std.h"

#include "bootset.h"
#include "english.h"

// Global variables
//------------------
APPLICATION     app;


void debugLog(char *szFormat, ...);

/*-----------------------------------------------------------------------------
   main: Entrypoint into this program.
-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
    {
    // Check if file can be opened
    // --------------------------------------------
    if (argc == 2)
       {
       if ( !(app.stream = fopen(argv[1], "r")) )
          {
          printf ("\n\n%s - %s", IDS_BOOTSETVER, IDS_VERSION);
          printf (IDS_SETNOTFOUND, argv[1]);
          printf (IDS_PRESSANYKEY);
          VioInkey(TRUE, NULL);
          return (1);
          }
       }
    else
       {
       if ( !(app.stream = fopen(IDS_INIFILE, "r")) )
          {
          printf("\n\n%s - %s", IDS_BOOTSETVER, IDS_VERSION);
          printf(IDS_SETMUSTSPEC);
          printf(IDS_PRESSANYKEY);
          VioInkey(TRUE, NULL);
          return (1);
          }
       }


    // Find out where .exe was run from and set files
    // ---------------------------------------------
    sprintf(app.szBatchFile, "%s%s", FileGetPath(argv[0]), IDS_BATCHFILE);
    sprintf(app.szLogFile, "%s%s", FileGetPath(argv[0]), IDS_LOGFILE);

    // Save CommandLine screen and clear the screen.
    // ---------------------------------------------
    SelectList(argv[1]);
    app.bShowTime = FALSE;
    app.bExitProgram = FALSE;
    DosWaitThread(&app.tid, DCWW_WAIT);
    InitApplication(1);
    return 0;
    }


/*-----------------------------------------------------------------------------
   SelectList: .
-----------------------------------------------------------------------------*/
void SelectList(char *szINIfile)
     {
     char              szString[255];

     STVIOLISTBOX       lb;
     BOOL               bQuit = FALSE;

     PCHAR              *pszSelectList;
     PFILESETTYPE       set;

     int               iRet = 0, iCounter, iInner;

     FILE               *batch;

     HFILE              hIOCTL;
     unsigned long     ulAction;

     char               szProduct[12];
     char               cOne, cTwo;

     PSTVIO             pstv;
     PSTVIOWINDOW       pwnd, hwnd;
     int                aiRet[2];


     // Waitscreen, start with one line
     fgets(szString, 255, app.stream);
     app.iWait = atoi(szString);

     // Foreground color
     fgets(szString, 255, app.stream);
     app.iForeColor = atoi(szString);

     // Background color
     fgets(szString, 255, app.stream);
     app.iBackColor = atoi(szString);

     // Timeout
     fgets(szString, 255, app.stream);
     app.iTimeOut = atoi(szString);

     // Future expansion
     fgets(szString, 255, app.stream);
     fgets(szString, 255, app.stream);
     fgets(szString, 255, app.stream);
     fgets(szString, 255, app.stream);
     fgets(szString, 255, app.stream);
     fgets(szString, 255, app.stream);

     // Save screen
     InitApplication(0);

     // Check for one-line keypress
     if (app.iWait == 1)
		{
		app.iOneLineRow = VioQueryCursorRow();
		app.pbOneLine = VioGetWindow(app.iOneLineRow, 1, app.iOneLineRow, 80);

		app.iOneLineCol = IDI_PRESSBFORCOL; 
		VioFill(app.iOneLineRow, 1, 80, app.iForeColor, app.iBackColor);
		sprintf(szString, IDS_PRESSBFOR, IDS_BOOTSET);
		VioPrintString (app.iOneLineRow, 2, szString);

		VioSetCursorPosition(app.iOneLineRow, 9);

		app.bExitProgram = TRUE;
		app.bShowTime = TRUE;

		// Start the thread.
		// --------------------------------------------
		if (app.iTimeOut > 0)
			app.tid = _beginthread(ExitProgram, NULL, 16738, NULL);

		iRet = VioInkey(TRUE, NULL);
		VioPutWindow(app.pbOneLine, app.iOneLineRow, 1, app.iOneLineRow, 80);

        switch (iRet)
                {
                case 'B':
                case 'b':
                         app.bExitProgram = FALSE;
			 app.bShowTime = FALSE;
                         break;

                default:
                         return;
                }
        }

     // Number of menu items
     fgets(szString, 255, app.stream);
     app.iTotalSet = atoi(szString);

     set = (PFILESETTYPE)calloc(app.iTotalSet, sizeof(FILESETTYPE));
     pszSelectList = (PCHAR*)calloc(app.iTotalSet, sizeof(PCHAR));

     for (iCounter = 0; iCounter < app.iTotalSet; iCounter++)
         {
         fgets(szString, 255, app.stream);
         szString[strlen(szString) - 1] = '\0';
         sprintf(set[iCounter].szTitle, " %s ", szString);

         pszSelectList[iCounter] = set[iCounter].szTitle;

         fgets(szString, 255, app.stream);
         set[iCounter].bReboot = atoi(szString);

         fgets(szString, 255, app.stream);
         set[iCounter].iCommand = atoi(szString);

         for (iInner = 0; iInner < set[iCounter].iCommand; iInner++)
             {
             fgets(szString, 255, app.stream);
             sprintf(set[iCounter].szCommand[iInner], szString);
             }
         }

     // Draw the window for BootSet fullscreen
     pstv = VioInit(-1, -1);

     VioSetGlobal(pstv, TRUE, 0, TRUE, TRUE, 30,
                  app.iForeColor, app.iBackColor,
                  app.iForeColor, app.iBackColor,
                  app.iForeColor, app.iBackColor,
                  app.iBackColor, app.iForeColor);

     sprintf(szString, "%s - %s", IDS_BOOTSETVER, IDS_VERSION);
     VioDesktop(pstv, szString, 177, TRUE);

     VioStatusPrint(pstv, 2, 1, FALSE, IDS_STATUSKEYS);

     sprintf(szString, "%s", IDS_BOOTSETVER);

     app.pmainWnd = VioOpenStandardWindow(pstv, 3, 20, 20, 40, szString);

     VioPrint(app.pmainWnd, 18, 3, IDS_WINDOWKEYS);
     VioFrame(app.pmainWnd, "", 2, 2, 15, 35, 0);

     // If we went strait into the fullscreen
     if (app.iWait == 0)
        {
        app.bExitProgram = TRUE;
        app.bShowTime = TRUE;

        // Start the thread.
        // --------------------------------------------
        if (app.iTimeOut > 0)
           app.tid = _beginthread(ExitProgram, NULL, 16738, NULL);
        }

     VioSetCursor(app.pmainWnd, 0, 0, FALSE);

     // Prepare for the listbox.
     // --------------------------------------------
     memset(&lb, 0, sizeof(STVIOLISTBOX));

     lb.row1 = 3;
     lb.col1 = 3;
     lb.rowTo = 14;
     lb.colTo = 34;
     lb.llen = app.iTotalSet;
     lb.startat = 0;
     lb.mark = FALSE;

     VioListBox(app.pmainWnd, &lb, 0, pszSelectList, aiRet, 0);

     bQuit = FALSE;
     while (bQuit == FALSE)
           {
           iRet = VioListBox(app.pmainWnd, &lb, 1, pszSelectList, aiRet, 0);
           app.bExitProgram = FALSE;
           app.bShowTime = FALSE;
           VioPrint(app.pmainWnd, 1, 3, "                                 ");
           switch (iRet)
                  {
                  case KEY_ENTER:
                       batch = fopen(app.szBatchFile, "w");
                       for (iCounter = 0; iCounter < set[lb.curpos].iCommand; iCounter++)
                           fprintf(batch, set[lb.curpos].szCommand[iCounter]);
                       fclose(batch);

                       VioCls(COLOR_WHITE, COLOR_BLACK);

                       iRet = system(app.szBatchFile);

                       if (iRet != -1)
                          {
                          // Remove Batch file
                          remove(app.szBatchFile);

                          // Do we want to reboot
                          if (set[lb.curpos].bReboot == 1)
                             {
                             // Open handle to DOS$
                             DosOpen((PCSZ)"\\DEV\\DOS$", &hIOCTL, &ulAction, 0L, FILE_NORMAL, FILE_OPEN, OPEN_SHARE_DENYNONE, 0L);

                             // Shutdown
                             DosShutdown(0);

                             // Reboot machine to use new files
                             DosDevIOCtl(hIOCTL, 0xD5, 0xAB, NULL, 0, NULL, NULL, 0, NULL);
                             }
                          }

                       bQuit = TRUE;
                       break;

                  case KEY_ESC:
                       bQuit = TRUE;
                       break;

                  case KEY_F1:
                       ShowFileHelp(pstv);
                       VioSetCursor(app.pmainWnd, 0, 0, FALSE);
                       break;
                 }
           }
     VioListBox(app.pmainWnd, &lb, 2, NULL, NULL, 0);
     free (pszSelectList);

     VioCloseWindow(app.pmainWnd);
     VioExit(pstv);
     return;
     }


/*-----------------------------------------------------------------------------
   ExitProgram: Counts down and exits program if it times out.
-----------------------------------------------------------------------------*/
void ExitProgram(void *arg)
     {
	 char               szString[255];
	 int                iAntal;

	 iAntal = app.iTimeOut;
	 while (iAntal > 0 )
            {
			if (app.bShowTime == TRUE)
			   {
			   if (app.iWait == 0)
				  {
				  sprintf(szString, IDS_TIMEOUTWINDOW, iAntal);
				  VioPrint(app.pmainWnd, 1, 3, szString);
				  }
			   else
				  {
				  sprintf(szString, IDS_TIMEOUTONELINE, iAntal);
				  VioPrintString (app.iOneLineRow, app.iOneLineCol, szString);
				  }
			   }

			  if (app.bExitProgram == FALSE)
				 {
				 return;
				 }

			  DosSleep(1000);
			  if (app.bExitProgram == FALSE)
				 {
				 return;
				 }
			  iAntal--;
			  }

      InitApplication(1);
      exit(0);
      }


/*-----------------------------------------------------------------------------
   InitApplication: Sets colors/clears screen etc.
-----------------------------------------------------------------------------*/
void InitApplication(USHORT usFlag)
    {
    static char       *pbCmdWindow;
    static int        iMaxRows, iMaxCols;
    static USHORT     usCmdRow, usCmdCol;


    if (usFlag == 0)
       {
       usCmdRow = VioQueryCursorRow();
       usCmdCol = VioQueryCursorColumn();
       VioGetInfo(&iMaxRows, &iMaxCols, NULL);
       pbCmdWindow = VioGetWindow(1, 1, iMaxRows, iMaxCols);
       }
    else
       {
       VioSetScreenSize(iMaxRows, iMaxCols);
       VioSetCursorPosition(usCmdRow, usCmdCol);
       VioShowCursor(TRUE);
       VioPutWindow(pbCmdWindow, 1, 1, iMaxRows, iMaxCols);
       }
    }


/*-----------------------------------------------------------------------------
   ShowFileHelp: Shows help screen for file mode.
-----------------------------------------------------------------------------*/
void ShowFileHelp(PSTVIO pstv)
     {
     char               szTmp[17][70];
     int                iCounter;
     PSTVIOWINDOW       pwnd;


     pwnd = VioOpenStandardWindow(pstv, 3, 5, 18, 69, IDS_WNDHELP);

     sprintf(szTmp[0],  IDS_HELP01);
     sprintf(szTmp[1],  IDS_HELP02);
     sprintf(szTmp[2],  IDS_HELP03);
     sprintf(szTmp[3],  IDS_HELP04);
     sprintf(szTmp[4],  IDS_HELP05);
     sprintf(szTmp[5],  IDS_HELP06);
     sprintf(szTmp[6],  IDS_HELP07);
     sprintf(szTmp[7],  IDS_HELP08);
     sprintf(szTmp[8],  IDS_HELP09);
     sprintf(szTmp[9],  IDS_HELP10);
     sprintf(szTmp[10], IDS_HELP11);
     sprintf(szTmp[11], IDS_HELP12);
     sprintf(szTmp[12], IDS_HELP13);
     sprintf(szTmp[13], IDS_HELP14);

     for (iCounter = 0; iCounter < 14; iCounter++)
         VioPrint(pwnd, iCounter + 2, 1, szTmp[iCounter]);

     VioGetKey(pwnd, TRUE, FALSE);

     VioCloseWindow(pwnd);
     }

/*-----------------------------------------------------------------------------
  debugLog: Skickar information till debuglog-filen.
-----------------------------------------------------------------------------*/
void debugLog(char *szFormat, ...)
     {
     FILE               *hLogFile;
     time_t             ltime;
     struct tm        *curtime;

     char              szData[2048];
     va_list            argptr;


     va_start(argptr, szFormat);
     vsprintf(szData, szFormat, argptr);
     va_end(argptr);

     time(&ltime);
     curtime = localtime(&ltime);

     hLogFile = fopen(app.szLogFile, "a");
     fprintf(hLogFile, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d\t%s\n",
             1900 + curtime->tm_year, curtime->tm_mon + 1, curtime->tm_mday,
             curtime->tm_hour, curtime->tm_min + 1, curtime->tm_sec, szData);
     fclose(hLogFile);
     }
