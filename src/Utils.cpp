#include "Utils.h"

void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

