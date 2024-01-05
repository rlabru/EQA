#ifndef PREFERENCES_H
#define PREFERENCES_H

#define ORGANIZATION_NAME "rlab.ru"
#define APPLICATION_NAME "EQA_test1"

#define EQA_test1_GENERAL_VERSION "0"
#define EQA_test1_VERSION "06"

/*
k23.06.11 - ver. 0.01, inital
k23.06.17 - ver. 0.02, struct Sense and HDD ID highlight added
k23.07.03 - ver. 0.03, sector buffer read/write added
k23.08.07 - ver. 0.04, write buffer loop back test added
k23.08.20 - ver. 0.05, mass read/write HDD buffer sector test added
k23.09.27 - ver. 0.06, simple power switch added


*/


/*!
    Main window parameters.
*/
#define WND_X 100
#define WND_Y 100
#define WND_WIDTH 1100
#define WND_HIGH 800
#define WND_LOG_HIGH 200
#define WND_ACTION_WIDTH_MAX 900
#define WND_ACTION_WIDTH_MIN 400
#define WND_HEX_WIDTH_MIN 600
#define WND_HEX_HEIGHT_MIN 600

/* main window size -> move to QSettings ! */
const int MAINWND_X_SIZE = 1800;
const int MAINWND_Y_SIZE = 800;

/* size of one sector */
#define SECTOR_S            512
static const int sector_s = 512;     /// main sector size

/*+pata host*/
#define WUKONG_UDP_ATA 8082
/*-pata host*/

#endif // PREFERENCES_H
