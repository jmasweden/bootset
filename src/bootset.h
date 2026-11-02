/***************************************************************************/
/* THIS CODE IS FREEWARE AND AS SUCH YOU ARE PERMITTED TO DO WHAT YOU WISH */
/* WITH IT. THESE PROGRAMS ARE PROVIDED AS IS WITHOUT ANY WARRANTY,        */
/* EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO FITNESS FOR ANY      */
/* PARTICULAR PURPOSE.                                                     */
/***************************************************************************/

/***************************************************************************/
/* Module:      BOOTSET.H, headerfile for custimized boot                  */
/***************************************************************************/

// #define declarations
//-----------------------
#define MAX_ITEMS       200

#define IDS_INIFILE     "BOOTSET.SET"
#define IDS_BATCHFILE   "BS.CMD"
#define IDS_LOGFILE     "LOG.TXT"

#define IDS_VERSION     "Oct 29, 2026"
#define IDS_BOOTSET     "BootSet"
#define IDS_BOOTSETVER  "BootSet 2.3"
#define IDS_BSEDIT      "BootSet Editor"
#define IDS_BSEDITVER   "BootSet Editor 2.3"


// struct declarations
//-----------------------
typedef struct _FILESETTYPE
       {
       char   szTitle[80];
       BOOL    bReboot;
       int    iCommand;
       char   szCommand[100][256];
       } FILESETTYPE;
typedef FILESETTYPE *PFILESETTYPE;



typedef struct _APPLICATION
       {
       FILE             *stream;
       BOOL             bExitProgram;
       BOOL             bShowTime;

       char            szBatchFile[_MAX_PATH];
       char            szLogFile[_MAX_PATH];
       char            szIniFile[_MAX_PATH];

       char            szRegKey[40];
       BOOL              bRegistered;

       int              iWait;
       int              iForeColor;
       int              iBackColor;
       int              iTotalSet;
       int              iTimeOut;

       PFILESETTYPE     set;
       PCHAR            *pszSelectList;

       TID              tid;

       USHORT           usCellNorm;
       USHORT           usCellRev;
       USHORT           usCellBlink;

       PSTVIOWINDOW     pmainWnd;
       int              iOneLineRow;
       int              iOneLineCol;
       char             *pbOneLine;

       } APPLICATION;


// Function declarations
//-----------------------
void SelectList(char *szINIfile);
void ExitProgram(void *arg);
void InitApplication(USHORT usFlag);
void ShowFileHelp(PSTVIO pstv);
