#include <unistd.h> // write()
#include <stdint.h> // uint types
#include <stdlib.h> // free()
#include <string.h> // memcpy()

#include "client-request.h"
#include "server-reply.h"
#include "timing.h"

void write_request(int, uint16_t, commandline*, struct timing*, uint64_t);
