/*
 * Copyright (C) 2018 ykonoclast
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UPROLLUTILS_H
#define UPROLLUTILS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u_char;
#include <bsd/stdlib.h>

extern int (*engine)(int p_nbdl, int p_nbdg, bool p_nexpl);

int lancer(int p_nbLances, int p_nbGardes, bool p_non_relance_dix);
int dieroll(bool p_nexpl);
int gt_comp(const void* a, const void* b);
int wqsort(int p_nbdl, int p_nbdg, bool p_nexpl);
int wnosort(int p_nbdl, int p_nbdg, bool p_nexpl);

#endif /* UPROLLUTILS_H */

