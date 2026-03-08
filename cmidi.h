// ==========================================================================
// cmidi.h - CMidi class   CMidi object for linux asound v2.0 library
// (C) COPYRIGHT 2026 by Kenneth Witt
// GPL-2.0 License
//
// NOTE: Must be linked with library asound
//       -lasound
//
// ----History---------------------------------------------------------------
//  01-01-2026  Created by Kenneth Witt, based on Craig Stuart Sapp's midiecho.c
// ==========================================================================
#include <fstream>
#include <string>
#include <alsa/asoundlib.h>
using namespace std;

class CMidi {
// ---------------------------------------------------------------------------
// TYPES
// ---------------------------------------------------------------------------
private:
  // Enum and text used for error messages and function status returns
  typedef enum {
    ERR_SUCCESS,  ERR_INFO,     ERR_WARNING,  ERR_ERROR,    ERR_DIE,
    NUM_ERR
  } errorType;
  string errTitles[NUM_ERR] = { "SUCCESS", "INFO", "WARNING", "ERROR", "DIE" };
// ---------------------------------------------------------------------------
// DATA
// ---------------------------------------------------------------------------
public:
private:
  snd_rawmidi_t* in  = NULL;        // asound structure to access MIDI input
  snd_rawmidi_t* out = NULL;        // asound structure to access MIDI output
  ifstream vIn;                     // virtual MIDI input
  ofstream vOut;                    // virtual MIDI output
  int vOutCount = 0;                // counter for virtual MIDI formatting
  string portName;                  // MIDI port name (such as : 'hw:1,0,1")
  unsigned char inByte;             // single byte read from MIDI
  int status = 0;                   // status of last operation
  int midiMode = SND_RAWMIDI_SYNC;  // SND_RAWMIDI_SYNC or SND_RAWMIDI_NONBLOCK
  errorType errStatus;              // error code
// ---------------------------------------------------------------------------
// METHODS
// ---------------------------------------------------------------------------
public:
  // Open port on creation, or call open below after creation
  CMidi(string portName = "", bool bBlockingMode = true);
  ~CMidi();
  // Open and close the midi port
  int open(bool bBlockingMode = true, bool bSilent = false);
  void close();
  // Virtual port will write data to specified output file
  int openVirtualOut(string fileName);
  // Get the asound library last encountered error
  string getErrorMsg() {return string(::snd_strerror(status)); };
  // Midi port I/O (or virtual port I/O)
  int write(string& s);
  int read();
  // # of MIDI data bytes expected to follow given midiCommand
  int getArgsExpected(int midiCommand);
  // asound library functions
  snd_rawmidi_t* getIn()  { return in; };
  snd_rawmidi_t* getOut() { return out; };
private:
}; // class CMidi
