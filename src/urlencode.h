/*
Filename: urlencode.h
Author: Bartoloměj Bičovský
Purpose: urlencode header
Version: 0.0.1
*/

#ifndef URLENCODE_H
#define URLENCODE_H

#include <stdbool.h>

char *urlDecode(char *inbuf, bool urlsafe);

#endif