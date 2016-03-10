/******************************************************************************
    Copyright � 2012-2015 Martin Karsten

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
#ifndef _Access_h_
#define _Access_h_

#include "runtime/SynchronizedArray.h"
#include "kernel/Output.h"
#include "devices/Keyboard.h"

#include <map>
#include <string>
#include <cerrno>
#include <unistd.h> // SEEK_SET, SEEK_CUR, SEEK_END

class Access : public SynchronizedElement {
public:
  virtual ~Access() {}
  virtual ssize_t pread(void *buf, size_t nbyte, off_t o) { return -EBADF; }
  virtual ssize_t pwrite(const void *buf, size_t nbyte, off_t o) { return -EBADF; }
  virtual ssize_t read(void *buf, size_t nbyte) { return -EBADF; }
  virtual ssize_t write(const void *buf, size_t nbyte) { return -EBADF; }
  virtual off_t lseek(off_t o, int whence) { return -EBADF; }
};

struct RamFile {
  vaddr vma;
  paddr pma;
  size_t size;
  RamFile(vaddr v, paddr p, size_t s) : vma(v), pma(p), size(s) {}
};

// Metadata structure for representing the name
// of a file as a structure. 
// DONE
struct fileName {
  vaddr vma;
  paddr pma;
  size_t size;
  fileName(vaddr v, paddr p, size_t s) : vma(v), pma(p), size(s) {}
};

extern map<string,RamFile> kernelFS;
// Modifying the format of 'kernelFS' to reflect our problem space.
// Creating a new map to traverse through the filesystem.
// DONE
extern map<string, fileName> newFS;
extern char ramBlock[];

// Since we are only doing simple I/O (read, write),
// spinlocks have to be checked. Each read/write entails
// a thread being created.
// Only need offset to characterize which location in
// the ramBlock we are currently in, and the information
// for the name of the file ("fn").
class newAccessor : public Access {
  SpinLock olock;
  off_t offset;
  const fileName &fn;

public:
  newAccessor(const fileName &fn) : offset(0), fn(fn) {}
  virtual ssize_t pwrite(off_t o, size_t nbyte, void *buf);
  virtual ssize_t write(void *buf, size_t nbyte);
  virtual ssize_t read(void *buf, size_t nbyte); // to do
  virtual ssize_t pread(void *buf, size_t nbyte, off_t o); //to do
};


class FileAccess : public Access {
  SpinLock olock;
  off_t offset;
  const RamFile &rf;
public:
  FileAccess(const RamFile& rf) : offset(0), rf(rf) {}
  virtual ssize_t pread(void *buf, size_t nbyte, off_t o);
  virtual ssize_t read(void *buf, size_t nbyte);
  virtual off_t lseek(off_t o, int whence);
};

class KernelOutput;
class OutputAccess : public Access {
  KernelOutput& ko;
public:
  OutputAccess(KernelOutput& ko) : ko(ko) {}
  virtual ssize_t write(const void *buf, size_t nbyte) {
    return ko.write(buf, nbyte);
  }
};

extern Keyboard keyboard;
class InputAccess : public Access {
public:
  InputAccess() {}
  virtual ssize_t read(void *buf, size_t nbyte) {
    if (nbyte == 0) return 0;
    Keyboard::KeyCode k = keyboard.read();
    char* s = (char*)buf;
    for (size_t r = 0; r < nbyte; r += 1) {
      s[r] = k;
      if (!keyboard.tryRead(k)) return r+1;
    }
    return nbyte;
  }
};

#endif /* _Access_h_ */
