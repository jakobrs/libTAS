/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LINTAS_UTILS_H_INCLUDED
#define LINTAS_UTILS_H_INCLUDED

#include <string>

/* Create a directory if it does not exist already */
int create_dir(std::string& path);

/* Wrapper for gz* to be used with libtar.
 * Code of gzopen_wrapper was taken from libtar demo driver
 * https://github.com/tklauser/libtar
 */
int gzopen_wrapper(const char *pathname, int oflags, int mode);
ssize_t gzread_wrapper(int, void *buf, size_t count);
ssize_t gzwrite_wrapper(int, const void *buf, size_t count);
int gzclose_wrapper(int);

#endif
