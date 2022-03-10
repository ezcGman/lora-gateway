#include "lora-ids.h"

/* The sync word functions like a "tag": This device will only listen to
 * messages with this sync word (tag) and completely ignore all others */
#define LORA_SYNC_WORD 0x42 // 66

/* If desired change this devices ID.
 * If so, also change the LORA_DEVICE_IDS map in "lora-ids.h" */
#define LORA_DEVICE_ID 0x1 // 1
