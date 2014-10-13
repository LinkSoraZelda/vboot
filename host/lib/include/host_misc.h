/* Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Host-side misc functions for verified boot.
 */

#ifndef VBOOT_REFERENCE_HOST_MISC_H_
#define VBOOT_REFERENCE_HOST_MISC_H_

#include "utility.h"
#include "vboot_struct.h"

/* Copy up to dest_size-1 characters from src to dest, ensuring null
   termination (which strncpy() doesn't do).  Returns the destination
   string. */
char* StrCopy(char* dest, const char* src, int dest_size);

/* Read data from [filename].  Store the size of returned data in [size].
 *
 * Returns the data buffer, which the caller must Free(), or NULL if
 * error. */
uint8_t* ReadFile(const char* filename, uint64_t* size);

/* Read a string from a file.  Passed the destination, dest size, and
 * filename to read.
 *
 * Returns the destination, or NULL if error. */
char* ReadFileString(char* dest, int size, const char* filename);

/* Read an unsigned integer from a file and save into passed pointer.
 *
 * Returns 0 if success, -1 if error. */
int ReadFileInt(const char* filename, unsigned* value);

/* Check if a bit is set in a file which contains an integer.
 *
 * Returns 1 if the bit is set, 0 if clear, or -1 if error. */
int ReadFileBit(const char* filename, int bitmask);

/* Writes [size] bytes of [data] to [filename].
 *
 * Returns 0 if success, 1 if error. */
int WriteFile(const char* filename, const void *data, uint64_t size);

#endif  /* VBOOT_REFERENCE_HOST_MISC_H_ */
