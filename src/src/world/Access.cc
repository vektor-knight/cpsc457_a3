/******************************************************************************
    Copyright © 2012-2015 Martin Karsten

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "world/Access.h"

#include <cstring>

// Memory block and new file system are instantiated here.
char ramBlock[16000];
map<string, fileName> newFS;

map<string, RamFile> kernelFS; // Unused.

ssize_t FileAccess::pread(void *buf, size_t nbyte, off_t o) {
  if (o + nbyte > rf.size) nbyte = rf.size - o;
  memcpy( buf, (bufptr_t)(rf.vma + o), nbyte );
  return nbyte;
}

ssize_t newAccessor::pread(void *buf, size_t nbyte, off_t o) {
  if (o + nbyte > fn.size) nbyte = fn.size - o;
  memcpy( buf, (bufptr_t)(fn.vma + o), nbyte);
  return nbyte;
}

ssize_t newAccessor::read(char *buf, size_t nbyte) {
  olock.acquire();
  ssize_t len = pread(buf, nbyte, offset);
  if (len >= 0) offset += len;
  olock.release();
  return len;
}

ssize_t newAccessor::pwrite(off_t o, size_t nbyte, void *buf) {
	if (nbyte + o > fn.size) {
		nbyte = fn.size - o;
	}
	memcpy(buf, (bufptr_t)(fn.vma + o), nbyte);
	return nbyte;
}

ssize_t newAccessor::write(char *buf, size_t nbyte) {
	olock.acquire();
	ssize_t block = pwrite(offset, nbyte, buf);
	if (block >= 0) {
		offset += block; // naive contig allocation. start block from current offset
	}
	olock.release();
	return block;
}

ssize_t FileAccess::read(void *buf, size_t nbyte) {
  olock.acquire();
  ssize_t len = pread(buf, nbyte, offset);
  if (len >= 0) offset += len;
  olock.release();
  return len;
}

off_t FileAccess::lseek(off_t o, int whence) {
  off_t new_o;
  switch (whence) {
    case SEEK_SET: new_o = o; break;
    case SEEK_CUR: new_o = offset + o; break;
    case SEEK_END: new_o = rf.size + o; break;
    default: return -EINVAL;
  }
  if (new_o < 0) return -EINVAL;
  offset = new_o;
  return offset;
}

