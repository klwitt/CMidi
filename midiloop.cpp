// ==========================================================================
// midiloop.cpp - Loop midi in to midi out, transposed up a tritone
// (C) COPYRIGHT 2026 by Kenneth Witt
// GPL-2.0 License
// NOTE: Must be linked with library asound
//       -lasound
//
// Compile:       g++ -o midiloop midiloop.cpp -lasound
//
// ----History---------------------------------------------------------------
//  01-01-2026  Created by Kenneth Witt, based on Craig Stuart Sapp's midiecho.c
// ==========================================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include "cmidi.h"

using namespace std;

const string version = "v0.01";

typedef enum {
  ERR_SUCCESS,  ERR_INFO,     ERR_WARNING,  ERR_ERROR,    ERR_DIE,
  NUM_ERR
} errorType;
string errTitles[NUM_ERR] = { "SUCCESS", "INFO", "WARNING", "ERROR", "DIE" };

// --------------------------------------------------------------------
// errorMessage - print a diagnostic message based on 'errorType'
// --History-----------------------------------------------------------
// 02-28-2013    Created by Kenneth Witt
void errorMessage(errorType e, string cmd) {
  string title;
  if ((int) e < NUM_ERR)
    title = errTitles[e];
  else
    title = "ERROR (ALT)";

  cerr << "%% " << title << " " << cmd;
  if (e == ERR_DIE) {
    cerr << endl;
    exit(1);
  }
  if (e != ERR_INFO && e != ERR_WARNING && e != ERR_ERROR && e != ERR_SUCCESS)
    cerr << " - error code " << e;
  cerr << endl;
} // errorMessage()

// --------------------------------------------------------------------
// midiecho - Echo MIDI notes out with an added transposition.
//
// --History-----------------------------------------------------------
// 12-31-2025    Created by Kenneth Witt
//
void midiEcho(CMidi& midi,          // MIDI object reference
              int transpose = 2) {  // transposition for note out
  string buffer;                    // storage space for incoming commands
  int argsLeft     = 0;             // bytes left to read for a command
  int newnote;                      // temp storage for output note

  while (1) {
    int inByte = midi.read();       // Get a byte from MIDI if available
    if (inByte < 0)
      errorMessage(ERR_DIE, "Problem reading MIDI input: "
                            + midi.getErrorMsg());

    if (inByte & 0x80) {   // a command byte has arrived
      argsLeft = midi.getArgsExpected(inByte);
      buffer = (char) inByte;
    } else {                   // a data byte has arrived
      if (buffer.empty() || (argsLeft <= 0))
        argsLeft = midi.getArgsExpected(inByte);
      buffer.append(1, inByte);
      argsLeft--;
    }

    if ( (argsLeft == 0) &&
         (buffer.size() > 1) &&
         ( ((buffer[0] & 0xf0) == 0x90) ||
           ((buffer[0] & 0xf0) == 0x80) ) ) {
      newnote = buffer[1] + transpose;
      if ((newnote > 0) && (newnote < 0x80)) {
        buffer[1] = (unsigned char) newnote;
        if (midi.write(buffer) < 0) {
          buffer.clear();
          errorMessage(ERR_ERROR, "Problem with MIDI out: "
                                  + midi.getErrorMsg());
          exit(1);
        } else if ( (buffer.size() > 1) &&
                    ((buffer[0] & 0xf0) == 0x90) &&
                    ((buffer[2] & 0x0f) != 0) ) {
          printf("New note: %d\n", newnote);
        }
      } // if newnote > 0
    } // buffer.size() > 1
  } // while (1)
} // midiEcho()

// --------------------------------------------------------------------------
// main - Perform processing of MIDI input
//
// --- History --------------------------------------------------------------
//  01-01-2026  Created by Kenneth Witt
//
int main(int argc, char *argv[]) {
  CMidi midi;  // No portname specified, so midi object will find it

  cout << "Type control-c to exit." << endl;
  midiEcho(midi);          // Do MIDI fun

  return 0;
} // main()
