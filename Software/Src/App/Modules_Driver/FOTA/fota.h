#ifndef FOTA_H
#define FOTA_H

#include "../../../Cfg.h"

#if FOTA_ENABLED == STD_ON

#include <stdbool.h>

// Function prototypes
void Fota_Init(void);
int Fota_CheckUpdate(void);
int Fota_PerformUpdate(const char *firmware_url);
const char* Fota_GetCurrentVersion(void);

#endif // FOTA_ENABLED

#endif // FOTA_H
