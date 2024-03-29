#pragma once
/* int_to_string.c
 *
 * Copyright (C) 2014-2024 wolfSSL Inc.
 *
 * This file is part of wolfSSH.
 *
 * wolfSSH is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfSSH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wolfSSH.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifdef __cplusplus
extern "C" {
#endif

    int int_to_string_VERSION();

    char *int_to_base(char *dest, unsigned long n, int base);

    char *int_to_string(char *dest, unsigned long n, long x);
    char *int_to_hex(char *dest, unsigned long n);
    char *int_to_dec(char *dest, unsigned long n);
    char *int_to_bin(char *dest, unsigned long n);

#ifdef __cplusplus
}
#endif