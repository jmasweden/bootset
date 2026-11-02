/***************************************************************************/
/* THIS CODE IS FREEWARE AND AS SUCH YOU ARE PERMITTED TO DO WHAT YOU WISH */
/* WITH IT. THESE PROGRAMS ARE PROVIDED AS IS WITHOUT ANY WARRANTY,        */
/* EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO FITNESS FOR ANY      */
/* PARTICULAR PURPOSE.                                                     */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/*   Modul:      BSEDIT.CPP, cpp-sorce for editor for custimized boot      */
/*                                                                         */
/*   Time:       980325, 260916, 29h                                       */
/*                                                                         */
/*  ---------------------------------------------------------------------  */
/*   980325:     Stole most from BOOTSET.CPP. (MA,1h)                      */
/*  ---------------------------------------------------------------------  */
/*   980331:     Lots more. (MA,11h)                                       */
/*  ---------------------------------------------------------------------  */
/*   980715:     Just a recompile. (MA,1h)                                 */
/*  ---------------------------------------------------------------------  */
/*   980729:     Every text moved to english.h. (MA,2h)                    */
/*  ---------------------------------------------------------------------  */
/*   981012:     Recompile to support .SET name. (MA,1h)                   */
/*  ---------------------------------------------------------------------  */
/*   260916:     Moved to gcc. (MA,4h)                                     */
/*  ---------------------------------------------------------------------  */
/*   261029:     Moved to openwatcom. (MA,2h)                              */
/*  ---------------------------------------------------------------------  */
/***************************************************************************/
#define VERSION   "0.106 - Sep 16, 2026"

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


BOOL LoadSettingsFile(void);
BOOL SaveSettingsFile(void);
BOOL CreateSettingsFile(void);
int MainSettings(PSTVIO pstv, int iStartField);
int CommandSettings(PSTVIO pstv, int iStartField, FILESETTYPE *pfst);
void ShowMainHelp(PSTVIO pstv);
void ShowSettingsHelp(PSTVIO pstv);
void ShowCommandHelp(PSTVIO pstv);
char *ReadLine(FILE *stream);
void debugLog(char *szFormat, ...);

/*-----------------------------------------------------------------------------
   main: Entrypoint into this program.
-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
    {
    char               szString[255];

    STVIOLISTBOX       lb;
    BOOL               bQuit = FALSE;

    PCHAR              *pszSelectList;
    PFILESETTYPE       set;

    int                iRet = 0, iCounter, iInner;

    FILE               *batch;

    HFILE              hIOCTL;
    unsigned long      ulAction;

    char               szProduct[12];
    char               cOne, cTwo;

    PSTVIO             pstv;
    PSTVIOWINDOW       pwnd, hwnd;
    int                aiRet[2];


    // Find out where .exe was run from and set log
    // ---------------------------------------------
    sprintf(app.szLogFile, "%s%s", FileGetPath(argv[0]), IDS_LOGFILE);


    // Allocate
    app.set = (PFILESETTYPE)calloc(MAX_ITEMS, sizeof(FILESETTYPE));
    app.pszSelectList = (PCHAR*)calloc(MAX_ITEMS, sizeof(PCHAR));

    // Draw the window for BootSet fullscreen
    pstv = VioInit(-1, -1);

     VioSetGlobal(pstv, TRUE, 255, TRUE, TRUE, 30,
                  COLOR_BLUE, COLOR_WHITE,
                  COLOR_BRBLUE, COLOR_WHITE,
                  COLOR_BLUE, COLOR_WHITE,
                  COLOR_WHITE, COLOR_RED
                  );

	sprintf(szString, "%s - %s", IDS_BSEDITVER, IDS_VERSION);
    VioDesktop(pstv, szString, 177, TRUE);

    VioStatusPrint(pstv, 2, 1, FALSE, IDS_BSESTATUSKEYS);


    // Check if file can be opened
    // --------------------------------------------
    if (argc == 2)
       {
       if ( !(app.stream = fopen(argv[1], "r")) )
          {
          VioMessageBox(pstv, 0, IDS_BSEERROR, "", IDS_BSESETNOTFOUND, argv[1]);
          VioExit(pstv);
          return (1);
          }
       else
          {
          strcpy(app.szIniFile, argv[1]);
          }
       }
    else
       {
       strcpy(app.szIniFile, IDS_INIFILE);
       if ( !(app.stream = fopen(app.szIniFile, "r")) )
          {
          if (VioMessageBox(pstv, 1, IDS_BSEWARNING, IDS_BSENOSETSPEC, IDS_BSEBSCREATE, IDS_BSEAREYOUSURE) == TRUE)
              {
              if ( CreateSettingsFile() == TRUE)
                  {
				  app.stream = fopen(app.szIniFile, "r");
                  }
              else
                  {
                  VioMessageBox(pstv, 0, IDS_BSEERROR, "", IDS_BSESETNOTCREATE, app.szIniFile);
                  VioExit(pstv);
                  return (1);
                  }
              }
          else
              {
              VioExit(pstv);
              return (1);
              }
          }
       }

    LoadSettingsFile();
    fclose(app.stream);

    sprintf(szString, "%s", IDS_BSEDITVER);
    app.pmainWnd = VioOpenStandardWindow(pstv, 3, 5, 20, 70, szString);
    VioFrame(app.pmainWnd, "", 2, 2, 15, 35, 255); // -1);

    VioPrint(app.pmainWnd, 3, 46, IDS_BSSETTINGSSTAR);

    sprintf(szString, IDS_BSFILENAME, app.szIniFile);
    VioPrint(app.pmainWnd, 5, 42, szString);

    if (app.iWait == 0)
       VioPrint(app.pmainWnd, 7, 43, IDS_BSSTARTFS);
    else
       VioPrint(app.pmainWnd, 7, 43, IDS_BSSTARTOL);

    sprintf(szString, "%s %3d %s", IDS_BSTIMEOUT, app.iTimeOut, IDS_BSSEC);
    VioPrint(app.pmainWnd, 11, 45, szString);

    VioPrint(app.pmainWnd, 13, 46, IDS_BSCOLORSET);
    VioColorPrint(app.pmainWnd, 14, 45, app.iForeColor, app.iBackColor, " Current colors ");

    VioPrint(app.pmainWnd, 18, 3, IDS_BSENTEREDIT);
    VioPrint(app.pmainWnd, 18, 16, IDS_BSESCEXIT);
    VioPrint(app.pmainWnd, 18, 27, IDS_BSINSADD);
    VioPrint(app.pmainWnd, 18, 37, IDS_BSDELREM);
    VioPrint(app.pmainWnd, 18, 50, IDS_BSTABEDITSET);

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

    VioListBox(app.pmainWnd, &lb, 0, app.pszSelectList, aiRet, 0);

    bQuit = FALSE;
    while (bQuit == FALSE)
          {
          iRet = VioListBox(app.pmainWnd, &lb, 1, app.pszSelectList, aiRet, 0);
          switch (iRet)
                 {
                 case KEY_ENTER:
                      if (app.set[lb.curpos].iCommand > 10)
                         VioMessageBox(pstv, 0, IDS_BSEWARNING, IDS_BSTOMANYLINES, IDS_BSDATALOST, "");
                      if (CommandSettings(pstv, 1, &app.set[lb.curpos]) == TRUE)
                         lb.redraw = TRUE;
                      VioSetCursor(app.pmainWnd, 0, 0, FALSE);
                      break;

                 case KEY_INS:
                      if (app.iTotalSet < MAX_ITEMS)
                         {
                         for (iCounter = app.iTotalSet; iCounter > lb.curpos; iCounter--)
                             {
                             memcpy(&app.set[iCounter], &app.set[iCounter - 1], sizeof(FILESETTYPE));
                             app.pszSelectList[iCounter] = app.set[iCounter].szTitle;
                             }

                         lb.curpos++;
                         strcpy(app.set[lb.curpos].szTitle, IDS_BSNEWITEM);
                         app.pszSelectList[lb.curpos] = app.set[lb.curpos].szTitle;
                         app.set[lb.curpos].bReboot = FALSE;
                         app.set[lb.curpos].iCommand = 2;
                         strcpy(app.set[lb.curpos].szCommand[0], IDS_BSREM1);
                         strcpy(app.set[lb.curpos].szCommand[1], IDS_BSREM2);

                         app.iTotalSet++;
                         lb.llen++;
                         lb.lvisible++;
                         lb.redraw = TRUE;
                         }
                      else
                         {
                         VioMessageBox(pstv, 0, IDS_BSEERROR, IDS_BSTOMANYITEMS, "", IDS_BSCANNOTADD);
                         }
                      VioSetCursor(app.pmainWnd, 0, 0, FALSE);
                      break;

                 case KEY_DEL:
                      if (app.iTotalSet > 1)
                         {
                         if (VioMessageBox(pstv, 2, IDS_BSEWARNING, IDS_BSDELETEITEM, app.pszSelectList[lb.curpos], IDS_BSEAREYOUSURE) == TRUE)
                            {
                            for (iCounter = lb.curpos; iCounter < (app.iTotalSet - 1); iCounter++)
                                {
                                memcpy(&app.set[iCounter], &app.set[iCounter + 1], sizeof(FILESETTYPE));
                                app.pszSelectList[iCounter] = app.set[iCounter].szTitle;
                                }
                            app.pszSelectList[app.iTotalSet] = NULL;
                            if (app.iTotalSet - 1 == lb.curRow)
                               lb.curRow--;
                            app.iTotalSet--;
                            lb.llen--;
                            lb.redraw = TRUE;
                            }
                         }
                      else
                         {
                         VioMessageBox(pstv, 0, IDS_BSEERROR, "", IDS_BSCANNOTREMOVE, "");
                         }
                      VioSetCursor(app.pmainWnd, 0, 0, FALSE);
                      break;

                 case KEY_F1:
                      ShowMainHelp(pstv);
                      VioSetCursor(app.pmainWnd, 0, 0, FALSE);
                      break;

                 case KEY_ESC:
                      if (VioMessageBox(pstv, 1, IDS_BSEXITPROGRAM, "", IDS_BSCHANGESET, "") == TRUE)
                         {
                         if (SaveSettingsFile() == FALSE)
                            VioMessageBox(pstv, 0, IDS_BSEERROR, "", IDS_BSCANNOTWRITE, app.szIniFile);
                         else
                            bQuit = TRUE;
                         }
                      else
                         bQuit = TRUE;
                      break;

                 case KEY_TAB:
                      if (MainSettings(pstv, 1) == TRUE)
                         {
                         if (app.iWait == 0)
                            VioPrint(app.pmainWnd, 7, 43, IDS_BSSTARTFS);
                         else
                            VioPrint(app.pmainWnd, 7, 43, IDS_BSSTARTOL);

                         sprintf(szString, "%s %3d sec", IDS_BSTIMEOUT, app.iTimeOut);
                         VioPrint(app.pmainWnd, 11, 45, szString);

                         VioPrint(app.pmainWnd, 13, 46, IDS_BSCOLORSET);
                         VioColorPrint(app.pmainWnd, 14, 45, app.iForeColor, app.iBackColor, IDS_BSCOLORCUR);
                         }
                         VioSetCursor(app.pmainWnd, 0, 0, FALSE);
                      break;
                 }
          }
    VioListBox(app.pmainWnd, &lb, 2, NULL, NULL, 0);
    free (app.pszSelectList);

    VioCloseWindow(app.pmainWnd);
    VioExit(pstv);
    return(0);
    }


// ---------------------------------------------------------------------
//  LoadSettingsFile: Loads an INI file.                     1998-03-26
// ---------------------------------------------------------------------
BOOL LoadSettingsFile(void)
     {
     char               *szString;
     int                iCounter, iInner;


      // Waitscreen
     szString = ReadLine(app.stream);
     app.iWait = atoi(szString);

     // Foreground color
     szString = ReadLine(app.stream);
     app.iForeColor = atoi(szString);

     // Background color
     szString = ReadLine(app.stream);
     app.iBackColor = atoi(szString);

     // Timeout
     szString = ReadLine(app.stream);
     app.iTimeOut = atoi(szString);

     // Future expansion
     szString = ReadLine(app.stream);
     szString = ReadLine(app.stream);
     szString = ReadLine(app.stream);
     szString = ReadLine(app.stream);
     szString = ReadLine(app.stream);
     szString = ReadLine(app.stream);

     // Number of menu items
     szString = ReadLine(app.stream);
     app.iTotalSet = atoi(szString);

     app.set = (PFILESETTYPE)calloc(MAX_ITEMS, sizeof(FILESETTYPE));
     app.pszSelectList = (PCHAR*)calloc(MAX_ITEMS, sizeof(PCHAR));

     for (iCounter = 0; iCounter < app.iTotalSet; iCounter++)
         {
         szString = ReadLine(app.stream);
         strcpy(app.set[iCounter].szTitle, szString);

         app.pszSelectList[iCounter] = app.set[iCounter].szTitle;

         szString = ReadLine(app.stream);
         app.set[iCounter].bReboot = atoi(szString);

         szString = ReadLine(app.stream);
         app.set[iCounter].iCommand = atoi(szString);

         for (iInner = 0; iInner < app.set[iCounter].iCommand; iInner++)
             {
             szString = ReadLine(app.stream);
             sprintf(app.set[iCounter].szCommand[iInner], szString);
             }
         }
     return(TRUE);
     }

// ---------------------------------------------------------------------
//  SaveSettingsFile: Saves an INI file.                     1998-03-26
// ---------------------------------------------------------------------
BOOL SaveSettingsFile(void)
     {
     int        iCounter, iInner;


     if ( (app.stream = fopen(app.szIniFile, "w")) != NULL)
        {
        // Waitscreen
        fprintf(app.stream, "%d\n", app.iWait);

        // Foreground color
        fprintf(app.stream, "%d\n", app.iForeColor);

        // Background color
        fprintf(app.stream, "%d\n", app.iBackColor);

        // Timeout
        fprintf(app.stream, "%d\n", app.iTimeOut);

        // Future expansion
        fprintf(app.stream, "0\n");
        fprintf(app.stream, "0\n");
        fprintf(app.stream, "0\n");
        fprintf(app.stream, "0\n");
        fprintf(app.stream, "0\n");
        fprintf(app.stream, "0\n");

        // Total items
        fprintf(app.stream, "%d\n", app.iTotalSet);

        for (iCounter = 0; iCounter < app.iTotalSet; iCounter++)
            {
            fprintf(app.stream, "%s\n", app.set[iCounter].szTitle);
            fprintf(app.stream, "%d\n", app.set[iCounter].bReboot);
            fprintf(app.stream, "%d\n", app.set[iCounter].iCommand);
            for (iInner = 0; iInner < app.set[iCounter].iCommand; iInner++)
                fprintf(app.stream, "%s\n", app.set[iCounter].szCommand[iInner]);
            }

        return(TRUE);
        }
     else
        {
        return(FALSE);
        }
     }

// ---------------------------------------------------------------------
//  CreateSettingsFile: Creates an INI file.                 1998-03-31
// ---------------------------------------------------------------------
BOOL CreateSettingsFile(void)
     {
     int        iCounter, iInner;
     FILE       *stream;


     if ( (stream = fopen(app.szIniFile, "w")) != NULL)
        {
        // Waitscreen
        app.iWait = 1;
        fprintf(stream, "%d\n", app.iWait);

        // Foreground color
        app.iForeColor = 7;
        fprintf(stream, "%d\n", app.iForeColor);

        // Background color
        app.iBackColor = 1;
        fprintf(stream, "%d\n", app.iBackColor);

        // Timeout
        app.iTimeOut = 5;
        fprintf(stream, "%d\n", app.iTimeOut);

        // Future expansion
        fprintf(stream, "0\n");
        fprintf(stream, "0\n");
        fprintf(stream, "0\n");
        fprintf(stream, "0\n");
        fprintf(stream, "0\n");
        fprintf(stream, "0\n");

        // Total items
        app.iTotalSet = 1;
        fprintf(stream, "%d\n", app.iTotalSet);

        strcpy(app.set[0].szTitle, IDS_BSEXAMPLE);
        fprintf(stream, "%s\n", app.set[0].szTitle);
        app.pszSelectList[0] = app.set[0].szTitle;

        app.set[0].bReboot = FALSE;
        fprintf(stream, "%d\n", app.set[0].bReboot);
        app.set[0].iCommand = 2;
        fprintf(stream, "%d\n", app.set[0].iCommand);

        strcpy(app.set[0].szCommand[0], IDS_BSREM1);
        fprintf(stream, "%s\n", app.set[0].szCommand[0]);
        strcpy(app.set[0].szCommand[1], IDS_BSREM2);
        fprintf(stream, "%s\n", app.set[0].szCommand[1]);

        fclose(stream);

        return(TRUE);
        }
     else
        {
        return(FALSE);
        }
     }

// ---------------------------------------------------------------------
//  MainSettings: Window created by WinScreen.               1998-03-26
// ---------------------------------------------------------------------
int MainSettings(PSTVIO pstv, int iCurField)
    {
    PSTVIOWINDOW     pwnd;
    int             aiRet[8];
    int             iKey, iRet, iLastField;
    BOOL             bQuit = FALSE;


    APPLICATION      tapp;


    memcpy(&tapp, &app, sizeof(APPLICATION));

    pwnd = VioOpenStandardWindow(pstv,  5,  14,  13,  45, IDS_BSSETTINGS);

    VioPrint(pwnd, 4, 7, IDS_BSFCOLOR);
    VioPrint(pwnd, 5, 7, IDS_BSBCOLOR);
    VioPrint(pwnd, 7, 7, IDS_BSTIMEOUT);
    VioPrint(pwnd, 10, 3, IDS_BSLINE01);

    VioFieldPrint(pwnd, 2, 6, VioFormatString(IDFS_CHECK, IDS_BSSTARTONE, (void*) &tapp.iWait));
    VioEditPrint(pwnd, 4, 27, VioFormatString(IDFS_INTEGER, "02", (void *) &tapp.iForeColor));
    VioEditPrint(pwnd, 5, 27, VioFormatString(IDFS_INTEGER, "02", (void *) &tapp.iBackColor));
    VioEditPrint(pwnd, 7, 27, VioFormatString(IDFS_INTEGER, "003", (void *) &tapp.iTimeOut));
    VioFieldPrint(pwnd, 11, 7, VioFormatString(IDFS_BUTTON, IDS_BSF2SAVE, NULL));
    VioFieldPrint(pwnd, 11, 23, VioFormatString(IDFS_BUTTON, IDS_BSESCCANVEL, NULL));

    VioColorPrint(pwnd, 5, 30, tapp.iForeColor, tapp.iBackColor, IDS_BSCOLORTEST);

    if (iCurField == 0)
       iCurField = 1;

    iLastField=7;

    aiRet[0] = KEY_F2;
    aiRet[1] = KEY_ESC;
    aiRet[2] = KEY_ENTER;
    aiRet[3] = KEY_TAB;
    aiRet[4] = KEY_BTAB;
    aiRet[5] = KEY_DARROW;
    aiRet[6] = KEY_UARROW;
    aiRet[7] = KEY_F1;

    bQuit = FALSE;
    while (bQuit == FALSE)
          {
          switch (iCurField)
                 {
                 case 1 :
                      iKey = VioEditCheck(pwnd, &tapp.iWait, 2, 6, IDS_BSSTARTONE, aiRet, 8);
                      break;
                 case 2 :
                      iKey = VioEditInt(pwnd, &tapp.iForeColor, 4, 27, 2, aiRet, 8);
                      VioColorPrint(pwnd, 5, 30, tapp.iForeColor, tapp.iBackColor, IDS_BSCOLORTEST);
                      break;
                 case 3 :
                      iKey = VioEditInt(pwnd, &tapp.iBackColor, 5, 27, 2, aiRet, 8);
                      VioColorPrint(pwnd, 5, 30, tapp.iForeColor, tapp.iBackColor, IDS_BSCOLORTEST);
                      break;
                 case 4 :
                      iKey = VioEditInt(pwnd, &tapp.iTimeOut, 7, 27, 3, aiRet, 8);
                      break;
                 case 5 :
                      iKey = VioEditButton(pwnd, IDS_BSF2SAVE, 11, 7, KEY_F2, aiRet, 8);
                      break;
                 case 6 :
                      iKey = VioEditButton(pwnd, IDS_BSESCCANVEL, 11, 23, KEY_ESC, aiRet, 8);
                      break;
                 }

          switch (iKey)
                 {
                 case KEY_TAB:
                 case KEY_DARROW:
                 case KEY_ENTER:
                      iCurField++;
                      break;

                 case KEY_BTAB:
                 case KEY_UARROW:
                      iCurField--;
                      break;

                 case KEY_F2:
                      memcpy(&app, &tapp, sizeof(APPLICATION));
                      iRet = TRUE;
                      bQuit = TRUE;
                      break;

                 case KEY_F1:
                      ShowSettingsHelp(pstv);
                      break;

                 case KEY_ESC:
                      iRet = FALSE;
                      bQuit = TRUE;
                      break;
                 }

          if (iCurField > iLastField)
             iCurField = 1;
          if (iCurField < 1)
             iCurField = iLastField;
          }
    VioCloseWindow(pwnd);
    return(iRet);
    }


// ---------------------------------------------------------------------
//  CommandSettings: Window created by WinScreen.            1998-03-26
// ---------------------------------------------------------------------
int CommandSettings(PSTVIO pstv, int iCurField, FILESETTYPE *pfst)
    {
    PSTVIOWINDOW     pwnd;
    int              aiRet[8];
    int              iKey, iRet, iLastField;
    int              iCounter;
    BOOL             bQuit = FALSE;

    FILESETTYPE      tfst;


    memcpy(&tfst, pfst, sizeof(FILESETTYPE));

    pwnd = VioOpenStandardWindow(pstv,  5,  7,  20,  70, IDS_BSEDITITEM);

    VioPrint(pwnd, 2, 3, IDS_BSITEMTITLE);
    VioPrint(pwnd, 5, 3, IDS_BSCOMMAND);
    VioPrint(pwnd, 17, 3, IDS_BSLINE02);

    VioEditPrint(pwnd, 2, 15, VioFormatString(IDFS_STRING, "32", (void *) tfst.szTitle));
    VioFieldPrint(pwnd, 3, 15, VioFormatString(IDFS_CHECK, IDS_BSREBOOTAFTER, (void*) &tfst.bReboot));
    for (iCounter = 0; iCounter < 10; iCounter++)
        {
        if (iCounter < tfst.iCommand)
           VioEditPrint(pwnd, 6 + iCounter, 3, VioFormatString(IDFS_STRING, "64", (void *) tfst.szCommand[iCounter]));
        else
           VioEditPrint(pwnd, 6 + iCounter, 3, VioFormatString(IDFS_STRING, "64", (void *) ""));
        }
    VioFieldPrint(pwnd, 18, 18, VioFormatString(IDFS_BUTTON, IDS_BSF2SAVE, NULL));
    VioFieldPrint(pwnd, 18, 34, VioFormatString(IDFS_BUTTON, IDS_BSESCCANVEL, NULL));

    if (iCurField == 0)
       iCurField = 1;

    iLastField = 14;

    aiRet[0] = KEY_F2;
    aiRet[1] = KEY_ESC;
    aiRet[2] = KEY_ENTER;
    aiRet[3] = KEY_TAB;
    aiRet[4] = KEY_BTAB;
    aiRet[5] = KEY_DARROW;
    aiRet[6] = KEY_UARROW;
    aiRet[7] = KEY_F1;

    bQuit = FALSE;
    while (bQuit == FALSE)
          {
          switch (iCurField)
                 {
                 case 1 :
                      iKey = VioEditString(pwnd, tfst.szTitle, 2, 15, 32, aiRet, 8, IDH_NULL);
                      break;
                 case 2 :
                      iKey = VioEditCheck(pwnd, &tfst.bReboot, 3, 15, IDS_BSREBOOTAFTER, aiRet, 8);
                      break;
                 case 3 :
                      iKey = VioEditString(pwnd, tfst.szCommand[0], 6, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 4 :
                      iKey = VioEditString(pwnd, tfst.szCommand[1], 7, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 5 :
                      iKey = VioEditString(pwnd, tfst.szCommand[2], 8, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 6 :
                      iKey = VioEditString(pwnd, tfst.szCommand[3], 9, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 7 :
                      iKey = VioEditString(pwnd, tfst.szCommand[4], 10, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 8 :
                      iKey = VioEditString(pwnd, tfst.szCommand[5], 11, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 9 :
                      iKey = VioEditString(pwnd, tfst.szCommand[6], 12, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 10:
                      iKey = VioEditString(pwnd, tfst.szCommand[7], 13, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 11:
                      iKey = VioEditString(pwnd, tfst.szCommand[8], 14, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 12:
                      iKey = VioEditString(pwnd, tfst.szCommand[9], 15, 3, 64, aiRet, 8, IDH_NULL);
                      break;
                 case 13:
                      iKey = VioEditButton(pwnd,IDS_BSF2SAVE, 18, 18, KEY_F2, aiRet, 8);
                      break;
                 case 14:
                      iKey = VioEditButton(pwnd,IDS_BSESCCANVEL, 18, 34, KEY_ESC, aiRet, 8);
                      break;
                 }

          switch (iKey)
                 {
                 case KEY_TAB:
                 case KEY_DARROW:
                 case KEY_ENTER:
                      iCurField++;
                      break;

                 case KEY_BTAB:
                 case KEY_UARROW:
                      iCurField--;
                      break;

                 case KEY_F2:
                      tfst.iCommand = 0;
                      for (iCounter = 0; iCounter < 10; iCounter++)
                          {
                          strtrim(tfst.szCommand[iCounter]);
                          if ( strlen(tfst.szCommand[iCounter]) == 0)
                             {
                             tfst.iCommand = iCounter - 1;
                             break;
                             }
                          }

                      if (tfst.iCommand < 0)
                         {
                         VioMessageBox(pstv, 0, IDS_BSEERROR, "", IDS_BSCMDNOTEMPTY, "");
                         break;
                         }

                      tfst.iCommand++;

                      strtrim(tfst.szTitle);
                      if ( strlen(tfst.szTitle) == 0 )
                         {
                         VioMessageBox(pstv, 0, IDS_BSEERROR, "", IDS_BSTITLENOTEMP, "");
                         break;
                         }

                      memcpy(pfst, &tfst, sizeof(FILESETTYPE));
                      iRet = TRUE;
                      bQuit = TRUE;
                      break;

                 case KEY_F1:
                      ShowCommandHelp(pstv);
                      break;

                 case KEY_ESC:
                      iRet = FALSE;
                      bQuit = TRUE;
                      break;
                 }

          if (iCurField > iLastField)
             iCurField = 1;
          if (iCurField < 1)
             iCurField = iLastField;
          }
    VioCloseWindow(pwnd);
    return(iRet);
    }


/*-----------------------------------------------------------------------------
   ShowMainHelp: Shows help screen for main page.
-----------------------------------------------------------------------------*/
void ShowMainHelp(PSTVIO pstv)
     {
     char               szTmp[17][70];
     int                iCounter;
     PSTVIOWINDOW       pwnd;


     pwnd = VioOpenStandardWindow(pstv, 6, 8, 18, 69, IDS_BSWNDHELP);

     sprintf(szTmp[0],  IDS_BSHELP01);
     sprintf(szTmp[1],  IDS_BSHELP02);
     sprintf(szTmp[2],  IDS_BSHELP03);
     sprintf(szTmp[3],  IDS_BSHELP04);
     sprintf(szTmp[4],  IDS_BSHELP05);
     sprintf(szTmp[5],  IDS_BSHELP06);
     sprintf(szTmp[6],  IDS_BSHELP07);
     sprintf(szTmp[7],  IDS_BSHELP08);
     sprintf(szTmp[8],  IDS_BSHELP09);
     sprintf(szTmp[9],  IDS_BSHELP10);
     sprintf(szTmp[10], IDS_BSHELP11);
     sprintf(szTmp[11], IDS_BSHELP12);
     sprintf(szTmp[12], IDS_BSHELP13);
     sprintf(szTmp[13], IDS_BSHELP14);

     for (iCounter = 0; iCounter < 14; iCounter++)
         VioPrint(pwnd, iCounter + 2, 1, szTmp[iCounter]);

     VioGetKey(pwnd, TRUE, FALSE);

     VioCloseWindow(pwnd);
     }


/*-----------------------------------------------------------------------------
   ShowSettingsHelp: Shows help screen for settings dialog.
-----------------------------------------------------------------------------*/
void ShowSettingsHelp(PSTVIO pstv)
     {
     char               szTmp[17][70];
     char               szColor[2];
     int                iCounter, iColor;
     PSTVIOWINDOW       pwnd;


     pwnd = VioOpenStandardWindow(pstv, 3, 5, 19, 69, IDS_BSWNDHELPSET);

     sprintf(szTmp[0],  IDS_BSHELPS01);
     sprintf(szTmp[1],  IDS_BSHELPS02);
     sprintf(szTmp[2],  IDS_BSHELPS03);
     sprintf(szTmp[3],  IDS_BSHELPS04);
     sprintf(szTmp[4],  IDS_BSHELPS05);
     sprintf(szTmp[5],  IDS_BSHELPS06);
     sprintf(szTmp[6],  IDS_BSHELPS07);
     sprintf(szTmp[7],  IDS_BSHELPS08);
     sprintf(szTmp[8],  IDS_BSHELPS09);
     sprintf(szTmp[9],  IDS_BSHELPS10);
     sprintf(szTmp[10], IDS_BSHELPS11);
     sprintf(szTmp[11], IDS_BSHELPS12);
     sprintf(szTmp[12], IDS_BSHELPS13);
     sprintf(szTmp[13], IDS_BSHELPS14);
     sprintf(szTmp[14], IDS_BSHELPS15);

     for (iCounter = 0; iCounter < 15; iCounter++)
         VioPrint(pwnd, iCounter + 1, 1, szTmp[iCounter]);

     iColor = 0;
     for (iCounter = 14; iCounter < 69; iCounter += 3)
         {
         sprintf(szColor, "%-2d", iColor);
         VioColorPrint(pwnd, 8, iCounter, iColor, COLOR_WHITE, szColor);
         VioColorPrint(pwnd, 9, iCounter, COLOR_BRBLUE, iColor, "  ");
         iColor++;
         }

     VioGetKey(pwnd, TRUE, FALSE);

     VioCloseWindow(pwnd);
     }


/*-----------------------------------------------------------------------------
   ShowCommandHelp: Shows help screen for commands dialog.
-----------------------------------------------------------------------------*/
void ShowCommandHelp(PSTVIO pstv)
     {
     char               szTmp[18][70];
     int                iCounter;
     PSTVIOWINDOW       pwnd;


     pwnd = VioOpenStandardWindow(pstv, 3, 5, 21, 69, IDS_BSWNDHELPEDIT);

     sprintf(szTmp[0],  IDS_BSHELPE01);
     sprintf(szTmp[1],  IDS_BSHELPE02);
     sprintf(szTmp[2],  IDS_BSHELPE03);
     sprintf(szTmp[3],  IDS_BSHELPE04);
     sprintf(szTmp[4],  IDS_BSHELPE05);
     sprintf(szTmp[5],  IDS_BSHELPE06);
     sprintf(szTmp[6],  IDS_BSHELPE07);
     sprintf(szTmp[7],  IDS_BSHELPE08);
     sprintf(szTmp[8],  IDS_BSHELPE09);
     sprintf(szTmp[9],  IDS_BSHELPE10);
     sprintf(szTmp[10], IDS_BSHELPE11);
     sprintf(szTmp[11], IDS_BSHELPE12);
     sprintf(szTmp[12], IDS_BSHELPE13);
     sprintf(szTmp[13], IDS_BSHELPE14);
     sprintf(szTmp[14], IDS_BSHELPE15);
     sprintf(szTmp[15], IDS_BSHELPE16);
     sprintf(szTmp[16], IDS_BSHELPE17);
     sprintf(szTmp[17], IDS_BSHELPE18);

     for (iCounter = 0; iCounter < 18; iCounter++)
         VioPrint(pwnd, iCounter + 1, 1, szTmp[iCounter]);

     VioGetKey(pwnd, TRUE, FALSE);

     VioCloseWindow(pwnd);
     }


/*-----------------------------------------------------------------------------
   ReadLine: Reads a line from a file.
-----------------------------------------------------------------------------*/
char *ReadLine(FILE *stream)
     {
     static char        szLine[255];


     /* Get a line that is not blank and with not # (comments) */
     do
        {
        fgets(szLine, 255, stream);
        szLine[strlen(szLine) - 1] = '\0';
        }
     while (szLine[0] == ';' || strlen(szLine) == 0);

     return (szLine);
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
