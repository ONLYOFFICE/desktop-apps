#include "version.h"

#define IDI_MAINICON          101
#define IDD_DIALOG            102

#define IDC_MAIN_ICON         1001
#define IDC_LABEL_TITLE       1002
#define IDC_LABEL_MESSAGE     1003
#define IDC_PROGRESS          1004
#define IDC_BUTTON_CANCEL     1005
#define IDC_SILENT_CHECK      1006

#define IDT_TRANSLATIONS      10001

#define CAPTION_TEXT              VER_PRODUCTNAME_STR
#define MESSAGE_TEXT_ERR1         "The application cannot continue because this architecture is not supported.\0"
#define MESSAGE_TEXT_ERR2         "The application is already running.\0"
#define LABEL_TITLE_TEXT          "Preparing for installation\0"
#define LABEL_MESSAGE_TEXT        "Downloading a package\0"
#define LABEL_MESSAGE_TEXT_ERR1   "An error occurred during initialization.\nPlease try restarting the app later.\0"
#define LABEL_MESSAGE_TEXT_ERR2   "Package download failed: Not enough memory!\nPlease try restarting the app later.\0"
#define LABEL_MESSAGE_TEXT_ERR3   "Package download failed: Server connection error!\nPlease try restarting the app later.\0"
#define LABEL_MESSAGE_TEXT_ERR4   "Package download failed: Network error!\nPlease try restarting the app later.\0"
#define LABEL_MESSAGE_TEXT_ERR5   "An error occurred while running the package.\nPlease try restarting the app later.\0"
#define LABEL_MESSAGE_TEXT_ERR6   "An error occurred during initialization: Url not set.\0"
#define LABEL_MESSAGE_TEXT_ERR7   "An error occurred during initialization: File name not specified.\0"
#define BUTTON_CANCEL_TEXT        "Cancel\0"
#define SILENT_CHECK_TEXT         "Silent Installation\0"
