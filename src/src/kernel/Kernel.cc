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
#include "runtime/Thread.h"
#include "kernel/AddressSpace.h"
#include "kernel/Clock.h"
#include "kernel/Output.h"
#include "world/Access.h"
#include "machine/Machine.h"
#include "devices/Keyboard.h"

#include "main/UserMain.h"

AddressSpace kernelSpace(true); // AddressSpace.h
volatile mword Clock::tick;     // Clock.h

extern Keyboard keyboard;

#if TESTING_KEYCODE_LOOP
static void keybLoop() {
  for (;;) {
    Keyboard::KeyCode c = keyboard.read();
    StdErr.print(' ', FmtHex(c));
  }
}
#endif

void kosMain() {
  auto iter = newFS.find("filesystem_test");
  if (iter == newFS.end()) {
    KOUT::outl("filesystem_test information not found");
  } else {
    newAccessor f(iter->second);
    for (;;) {
      char c;
      if (f.read(&c, 1) == 0) break;	// reading done
      KOUT::out1(c);
    }
  }

  auto writeIter = newFS.find("filesystem_test");
  char* message = "My filesystem works !";

  newAccessor g(writeIter->second);
  for (int i = 0; i < sizeof(ramBlock); i++) {
    strcpy(ramBlock, message);
    g.write(&ramBlock[i], 1);
    KOUT::out1(&ramBlock[i]);
  } 

  for(int i = 0; i < sizeof("My filesystem works !"); i++) {
    KOUT::out1(ramBlock[i]);
  }

    KOUT::outl();

}


extern "C" void kmain(mword magic, mword addr, mword idx)         __section(".boot.text");
extern "C" void kmain(mword magic, mword addr, mword idx) {
  if (magic == 0 && addr == 0xE85250D6) {
    // low-level machine-dependent initialization on AP
    Machine::initAP(idx);
  } else {
    // low-level machine-dependent initialization on BSP -> starts kosMain
    Machine::initBSP(magic, addr, idx);
  }
}
