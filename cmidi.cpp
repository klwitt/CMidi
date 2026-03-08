// --------------------------------------------------------------------------
// cmidi.cpp - CMidi object for linux asound v2.0 library
// (C) COPYRIGHT 2026 by Kenneth Witt
// GPL-2.0 License
//
// NOTE: Must be linked with library asound
//       -lasound
//
// --- History --------------------------------------------------------------
//  01-01-2026  Created by Kenneth Witt
//
#include <iostream>
#include "cmidi.h"

// --------------------------------------------------------------------------
CMidi::CMidi(string paramPortName /* = "" */, bool bBlockingMode /* = true */) {
  if (paramPortName != "") {
    portName = paramPortName;
    if (portName.substr(0,3) == "hw:") {
      if (open(bBlockingMode) != ERR_SUCCESS) {
        // open() gives error message
        errStatus = ERR_ERROR;
        exit(errStatus);
      }
    }
    else {
      vIn.open(portName);
      if (!vIn.is_open()) {
        errStatus = ERR_DIE;
        cerr << "%% " << errTitles[errStatus]
             << " Could not open virtual input port '"
             << portName << "'" << endl;
        exit(errStatus);
      }
    }
  }
  else {
    for (int i = 1; i <= 4; i++) {
      portName = "hw:"+to_string(i)+",0,1";
      if (open(bBlockingMode, /* bSilent = */ true) == ERR_SUCCESS) {
        errStatus = ERR_INFO;
        cerr << "%% " << errTitles[errStatus]
             << " Opened MIDI port " << portName << endl;
        return;
      }
    }
    errStatus = ERR_DIE;
    cerr << "%% " << errTitles[errStatus]
         << " Could not find a MIDI port" << endl;
    exit(errStatus);
  }
} // CMidi::CMidi

// --------------------------------------------------------------------------
CMidi::~CMidi() {
  close();
}

// --------------------------------------------------------------------------
// close - close the MIDI input and output channels
//
// RETURN - status
// --- History --------------------------------------------------------------
//  01-01-2026  Created by Kenneth Witt
//
void CMidi::close() {
  if (in != NULL)
    ::snd_rawmidi_close(in);
  if (out != NULL)
    ::snd_rawmidi_close(out);
  in  = NULL;    // snd_rawmidi_close() does not clear invalid handle,
  out = NULL;    //   so clear handles after closing.
} // CMidi::close()

// --------------------------------------------------------------------------
// open - open the MIDI input and output channels
//
// RETURN - status
// --- History --------------------------------------------------------------
//  01-01-2026  Created by Kenneth Witt
//
int CMidi::open(bool bBlockingMode /* = true */,
                bool bSilent /* = false */) {
  midiMode = (bBlockingMode) ? SND_RAWMIDI_SYNC : SND_RAWMIDI_NONBLOCK;
  int status = ::snd_rawmidi_open(&in, &out, portName.c_str(), midiMode);
  if (status < 0) {
    errStatus = ERR_ERROR;
    if (!bSilent)
      cerr << "%% " << errTitles[errStatus]
           << " Problem opening MIDI input [" << portName << "]: "
           << string(::snd_strerror(status)) << endl;
    return errStatus;
  }
  errStatus = ERR_SUCCESS;
  return errStatus;
} // open()

// --------------------------------------------------------------------
// openVirtualOut - Open an output file for use if MIDI device port not open
// --History-----------------------------------------------------------
// 12-28-2025    Created by Kenneth Witt
// --------------------------------------------------------------------
int CMidi::openVirtualOut(string fileName) {
  if (vOut.is_open())
    vOut.close();
  vOut.open(fileName);
  errStatus = (vOut.is_open()) ? ERR_SUCCESS : ERR_ERROR;
  return errStatus;
} // CMidi::openVirtualOut()

// --------------------------------------------------------------------
// read - gets next byte from MIDI device or virtual MIDI
// --History-----------------------------------------------------------
// 12-28-2025    Created by Kenneth Witt
// --------------------------------------------------------------------
int CMidi::read() {
  if (!vIn.is_open()) {
    if (in != NULL)
      status = ::snd_rawmidi_read(in, &inByte, 1);
    else
      status = -1;
  }
  else { // vIn is a virtual MIDI input file
    string word;
    inByte = status = 0;
    vIn >> word;
    if (word.size() > 1) {
      for (auto c : word) {
        inByte <<= 4;
        if (c >= '0' && c <= '9')
          inByte += (c - '0');
        else if (c >= 'a' && c <= 'f')
          inByte += (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
          inByte += (c - 'A' + 10);
        else {
          status = -2;
          break;
        }
      }
    }
    else
      status = -2;
  }
  return (status < 0) ? status : inByte;
} // CMidi::read()

// --------------------------------------------------------------------
// write - writes byte(s) to MIDI device or virtual MIDI
// --History-----------------------------------------------------------
// 12-28-2025    Created by Kenneth Witt
// --------------------------------------------------------------------
int CMidi::write(string& s) {
  static string hexchars = "01234567789ABCDEF";
  status = 0;
  vOutCount = 0;
  if (vOut.is_open()) {  // Do virtual MIDI output if requested
    for (const auto& c : s) {
      if (vOutCount >= 24) {
        vOut << endl;
        vOutCount = 0;
      }
      else if (vOutCount > 0)
        vOut << ' ';
      int value = (int) ((unsigned char) c);
      char c0 = hexchars[value / 16];
      char c1 = hexchars[value % 16];
      vOut << c0 << c1;
      vOutCount++;
    }
    if (vOutCount > 0)
      vOut << endl;
  }
  if (out != NULL)
    status = ::snd_rawmidi_write(out, s.c_str(), s.size());
  return status;
} // CMidi::write()

// --------------------------------------------------------------------
// getArgsExpected - returns the number of MIDI data bytes which are
//     expected to follow the given command. 0xf0 set of commands
//     are not handled completely, particularly system exclusives.
// --History-----------------------------------------------------------
// 12-28-2025    Created by Kenneth Witt
int CMidi::getArgsExpected(int midicommand) {
   switch (midicommand & 0xf0) {
      case 0x80: return 2; // note off
      case 0x90: return 2; // note on
      case 0xa0: return 2; // aftertouch
      case 0xb0: return 2; // continuous controller
      case 0xc0: return 1; // patch change
      case 0xd0: return 1; // channel pressure
      case 0xe0: return 1; // pitch bend
      case 0xf0: return 0; // may be incorrect (such as sysex)
   }
   return 0;
} // getArgsExpected()

