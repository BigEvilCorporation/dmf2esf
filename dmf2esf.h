// Deflemask -> Echo Converter
// Started 2013-12-05, in Linköping, Sverige by Ian Karlsson.

// Sample rate conversion uses Secret Rabbit Code, see http://www.mega-nerd.com/SRC

/*
                        Useful Links:

Deflemask Manual (contains effect list)
    http://www.delek.com.ar/soft/deflemask/manual.html
Deflemask DMF format documentation (semi-accurate)
    http://www.delek.com.ar/soft/deflemask/DMF_SPECS.txt
Echo ESF documentation
    https://github.com/sikthehedgehog/Echo/blob/master/doc/esf.txt
*/

#ifndef DMF2ESF_H_INCLUDED
#define DMF2ESF_H_INCLUDED

#define MAJORVER 0
#define MINORVER 1

    #include <iostream>
    #include <iomanip>
    #include <fstream>
    #include <vector>
    #include <stdio.h>
    #include <math.h>
    #include <algorithm>
    #include <cstring>
	#include <set>

    #define MINIZ_HEADER_FILE_ONLY
    #include "miniz.c"

    #include "ini.h"
    #include "inireader.h"

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;

/* Main options */
extern bool OutputInstruments;
extern bool ASMOut;
extern bool ExCommands;

enum DMFVersion
{
	DMFVersion_12_0 = 0x18
};

enum ChannelType
{
    CHANNEL_TYPE_INVALID,
    CHANNEL_TYPE_FM,    // Normal FM channel
    CHANNEL_TYPE_FM6,   // FM channel with DAC
    CHANNEL_TYPE_PSG,   // Normal PSG tone generator
    CHANNEL_TYPE_PSG4,
};

enum ChannelId
{
    CHANNEL_FM1,
    CHANNEL_FM2,
    CHANNEL_FM3,
    CHANNEL_FM4,
    CHANNEL_FM5,
    CHANNEL_FM6,
    CHANNEL_PSG1,
    CHANNEL_PSG2,
    CHANNEL_PSG3,
    CHANNEL_PSG4,
};

enum PSGNoiseMode
{
    PSG_WHITE_NOISE_HI = 0,
    PSG_WHITE_NOISE_MED = 1,
    PSG_WHITE_NOISE_LOW = 2,
    PSG_WHITE_NOISE_PSG3 = 3,
    PSG_PERIODIC_NOISE_HI = 4,
    PSG_PERIODIC_NOISE_MED = 5,
    PSG_PERIODIC_NOISE_LOW = 6,
    PSG_PERIODIC_NOISE_PSG3 = 7,
};

enum SpecialNotes
{
	NOTE_OFF = 100
};

enum EffectMode
{
    EFFECT_OFF,     // all effects
    EFFECT_NORMAL,  // all effects except portamento, note slide
    EFFECT_UP,      // vibrato, portamento, note slide
    EFFECT_DOWN,    // vibrato, portamento, note slide
    EFFECT_SCHEDULE,// porta note
};

enum EffectStage
{
	EFFECT_STAGE_OFF,
	EFFECT_STAGE_INITIALISE,
	EFFECT_STAGE_CONTINUE
};

enum EffectType
{
	EFFECT_TYPE_NONE = 0xff, // No effect
	EFFECT_TYPE_DAC_ON = 0x17, // DAC enable (already done)
	EFFECT_TYPE_PSG_NOISE = 0x20, // PSG noise mode (already done)
	EFFECT_TYPE_ARPEGGIO = 0x00, // Arpeggio
	EFFECT_TYPE_PORTMENTO_UP = 0x01, // Portamento up
	EFFECT_TYPE_PORTMENTO_DOWN = 0x02, // Portamento down
	EFFECT_TYPE_PORTMENTO_TO_NOTE = 0x03, // Tone portamento
	EFFECT_TYPE_VIBTRATO = 0x04, // Vibrato
	EFFECT_TYPE_PORTMENTO_TO_NOTE_AND_VOL_SLIDE = 0x05, // Tone portamento + volume slide
	EFFECT_TYPE_VIBRATO_AND_VOL_SLIDE = 0x06, // Vibrato + volume slide
	EFFECT_TYPE_TREMOLO = 0x07, // Tremolo
	EFFECT_TYPE_PAN = 0x08, // Set panning
	EFFECT_TYPE_SET_SPEED_1 = 0x09, // Set speed 1
	EFFECT_TYPE_SET_SPEED_2 = 0x0f, // Set speed 2
	EFFECT_TYPE_VOLUME_SLIDE = 0x0a, // Volume slide
	EFFECT_TYPE_NOTE_RETRIGGER = 0x0c, // Note retrig

	//Extended effects
	EFFECT_TYPE_NOTE_SLIDE_UP = 0xe1, // Note slide up
	EFFECT_TYPE_NOTE_SLIDE_DOWN = 0xe2, // Note slide down
	EFFECT_TYPE_SET_VIBRATO_MODE = 0xe3, // Vibrato mode
	EFFECT_TYPE_SET_FINE_VIBRATO = 0xe4, // Fine vibrato depth
	EFFECT_TYPE_NOTE_CUT = 0xec, // Note cut
	EFFECT_TYPE_JUMP = 0x0b, // Position jump
	EFFECT_TYPE_BREAK = 0x0d, // Pattern break
};

enum FMRegister
{
	FMREG_22_LFO		= 0x22,
	FMREG_26_TIMER_B	= 0x26
};

enum ESFChannel
{
    ESF_FM1 = 0x00,
    ESF_FM2 = 0x01,
    ESF_FM3 = 0x02,
    ESF_FM4 = 0x04,
    ESF_FM5 = 0x05,
    ESF_FM6 = 0x06,
    ESF_PSG1 =0x08,
    ESF_PSG2 =0x09,
    ESF_PSG3 =0x0a,
    ESF_PSG4 =0x0b,
    ESF_DAC = 0x0c,
};

static ChannelType ESFChannelTypes[] =
{
    CHANNEL_TYPE_FM,        //0
    CHANNEL_TYPE_FM,        //1
    CHANNEL_TYPE_FM,        //2
    CHANNEL_TYPE_INVALID,   //3
    CHANNEL_TYPE_FM,        //4
    CHANNEL_TYPE_FM,        //5
    CHANNEL_TYPE_FM,        //6
    CHANNEL_TYPE_INVALID,   //7
    CHANNEL_TYPE_PSG,       //8
    CHANNEL_TYPE_PSG,       //9
    CHANNEL_TYPE_PSG,       //a
    CHANNEL_TYPE_PSG4,      //b
    CHANNEL_TYPE_FM6,       //c
};

enum DMFSystem
{
    DMF_SYSTEM_GENESIS = 2,
    DMF_SYSTEM_SMS = 3,
};

enum ESFPCMRate
{
	PCM_Freq_26632Hz = 0x1,
	PCM_Freq_17755Hz = 0x2,
	PCM_Freq_13316Hz = 0x3,
	PCM_Freq_10653Hz = 0x4,
	PCM_Freq_8877Hz  = 0x5,
	PCM_Freq_7609Hz  = 0x6,
	PCM_Freq_6658Hz  = 0x7,
	PCM_Freq_5918Hz  = 0x8,
	PCM_Freq_5326Hz  = 0x9,
	PCM_Freq_4842Hz  = 0xA,
	PCM_Freq_4439Hz  = 0xB,
	PCM_Freq_4097Hz  = 0xC,

	PCM_Freq_Default = PCM_Freq_10653Hz
};

struct ChannelArray
{
    ChannelId   aChannelId;
    ChannelType aChannelType;
    ESFChannel  aESFChannel;
};

static ChannelArray MDChannels[] =
{
    {CHANNEL_FM1, CHANNEL_TYPE_FM, ESF_FM1},
    {CHANNEL_FM2, CHANNEL_TYPE_FM, ESF_FM2},
    {CHANNEL_FM3, CHANNEL_TYPE_FM, ESF_FM3},
    {CHANNEL_FM4, CHANNEL_TYPE_FM, ESF_FM4},
    {CHANNEL_FM5, CHANNEL_TYPE_FM, ESF_FM5},
    {CHANNEL_FM6, CHANNEL_TYPE_FM6, ESF_FM6},
    {CHANNEL_PSG1, CHANNEL_TYPE_PSG, ESF_PSG1},
    {CHANNEL_PSG2, CHANNEL_TYPE_PSG, ESF_PSG2},
    {CHANNEL_PSG3, CHANNEL_TYPE_PSG, ESF_PSG3},
    {CHANNEL_PSG4, CHANNEL_TYPE_PSG4, ESF_PSG4},
};

static ChannelArray SMSChannels[] =
{
    {CHANNEL_PSG1, CHANNEL_TYPE_PSG, ESF_PSG1},
    {CHANNEL_PSG2, CHANNEL_TYPE_PSG, ESF_PSG2},
    {CHANNEL_PSG3, CHANNEL_TYPE_PSG, ESF_PSG3},
    {CHANNEL_PSG4, CHANNEL_TYPE_PSG4, ESF_PSG4},
};

static std::string NoteNames[] =
{
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-",
};
static std::string ESFChanNames[] =
{
    "FM 1", "FM 2", "FM 3", "", "FM 4", "FM 5", "FM 6", "", "PSG 1", "PSG 2", "PSG 3", "PSG 4", "PCM"
};

static uint16_t PSGFreqs[12][7] = // [semitone][octave]
{
    // 0   1   2   3   4   5,  6
    { 851,425,212,106, 53, 26, 13 }, // c
    { 803,401,200,100, 50, 25, 12 }, // c#
    { 758,379,189, 94, 47, 23, 11 }, // d
    { 715,357,178, 89, 44, 22, 10 }, // d#
    { 675,337,168, 84, 42, 21, 9 }, // e
    { 637,318,159, 79, 39, 19, 8 }, // f
    { 601,300,150, 75, 37, 18, 7 }, // f#
    { 568,284,142, 71, 31, 15, 6 }, // g
    { 536,268,134, 67, 33, 16, 5 }, // g#
    { 506,253,126, 63, 31, 15, 4 }, // a
    { 477,438,119, 59, 29, 14, 3 }, // a#
    { 450,225,112, 56, 28, 14, 2 }, // b
};

static const int MaxFMFreqs = 12;
static const int MaxOctave = 7;
static uint16_t FMFreqs[MaxFMFreqs] =
{
    644,681,722,765,810,858,910,964,1021,1081,1146,1214
};

static const int FM_TimerB_NTSC = 0xC9;
static const int FM_TimerB_PAL = 0xBD;

struct EffectArpeggio
{
	EffectArpeggio() { Arp = EFFECT_OFF; }

	//0xx (arpeggio)
	EffectMode  Arp;
	uint16_t    Arp1;
	uint16_t    Arp2;
	uint8_t     ArpCounter;
};

struct EffectPortmento
{
	EffectPortmento() { Porta = EFFECT_OFF; }

	//1xx, 2xx (portamento)
	EffectMode  Porta;
	EffectStage Stage;
	uint8_t     PortaSpeed;
	uint8_t     Octave;
	uint32_t    Semitone;
	bool        NoteOnthisTick;
};

struct EffectPortaNote
{
	EffectPortaNote() { PortaNote = EFFECT_OFF; }

	//3xx (porta to note), cancels out 1xx,2xx
	EffectMode  PortaNote;
	uint8_t     PortaNoteActive;
	uint8_t     PortaNoteSpeed;
	uint8_t     PortaNoteCurrentNote;
	uint8_t     PortaNoteTargetNote;
	uint8_t     PortaNoteCurrentOctave;
	uint8_t     PortaNoteTargetOctave;
};

struct EffectVibrato
{
	EffectVibrato() { Vibrato = EFFECT_OFF; }

	//4xx (vibrato)
	EffectMode  Vibrato;
	uint8_t     VibratoActive;
	uint8_t     VibratoFineDepth;
	uint8_t     VibratoDepth;
	uint8_t     VibratoSpeed;
	uint8_t     VibratoOffset;
};

// 5xx, 6xx ignored

struct EffectTremolo
{
	EffectTremolo() { Tremolo = EFFECT_OFF; }

	// 7xx (tremolo)
	EffectMode  Tremolo;
	uint8_t     TremoloActive;
	uint8_t     TremoloDepth;
	uint8_t     TremoloSpeed;
	uint8_t     TremoloOffset;
};

// 8xx (panning) doesn't need variables

struct EffectVolSlide
{
	EffectVolSlide() { VolSlide = EFFECT_OFF; }

	// Axx (volume slide)
	EffectMode  VolSlide;
	int8_t      VolSlideValue;
	uint16_t    CurrVol;
};

// Bxx (position jump, global effect)

struct EffectRetrigger
{
	EffectRetrigger() { Retrig = EFFECT_OFF; }

	// Cxx (note retrig)
	EffectMode  Retrig;
	uint8_t     RetrigSpeed;
};

// Dxx (pattern break, global effect)

struct EffectNoteSlide
{
	EffectNoteSlide() { NoteSlide = EFFECT_OFF; }

	// E1xy, E2xy (note slide)
	EffectMode  NoteSlide;
	uint8_t     NoteSlideSpeed;
	uint8_t     NoteSlideFinal;
};

// E3xx (set vibrato mode)
// E4xx (set fine vibrato depth)
// E5xx (fine tune, no support for now)
// EBxx (set sample bank, no support for now)

struct EffectNoteCut
{
	EffectNoteCut() { NoteCut = EFFECT_OFF; }

	// ECxx (note cut)
	EffectMode  NoteCut;
	uint8_t     NoteCutActive;
	uint8_t     NoteCutOffset;
};

struct EffectNoteDelay
{
	EffectNoteDelay() { NoteDelay = EFFECT_OFF; }

	// EDxx (note delay)
	EffectMode  NoteDelay;
	uint8_t     NoteDelayOffset;
};

struct EffectPSGNoise
{
	EffectPSGNoise() { Mode = EFFECT_OFF; EnvelopeSize = 0; }

	EffectMode  Mode;
	uint8_t     EnvelopeIdx;
	uint8_t     EnvelopeSize;
	int32_t*    EnvelopeData;
};

// EBxx (global fine tune, no support for now)
// 17xx (DAC enable)

// 10xy (set LFO, not supported by Echo)
// 11xy, 12xx, 13xx ... (operator modifying effects not supported by Echo)

struct Channel
{
    ChannelId   Id;
    ChannelType Type;
    ESFChannel  ESFId;

    uint8_t     EffectCount;

    bool        NoteOn;

    uint8_t     Note;
    uint8_t     Octave;
    double      NoteFreq; // freq of last note on
    double      ToneFreq;
    uint8_t     NewNote;
    uint8_t     NewOctave;
	uint8_t     LastNote;
	uint8_t     LastOctave;
    int16_t     LastFreq; // difference
    int16_t     NewFreq;  // difference
    uint8_t     Instrument;
    uint8_t     NewInstrument;
    uint8_t     Volume;
    uint8_t     NewVolume;
	uint8_t     LastVolume;
    uint8_t     SubtickFX;       // 0=none, >0=yes
	uint8_t     lastPanning;
	uint8_t     lastFMS;
	uint8_t     lastAMS;

	EffectArpeggio m_effectArpeggio;
	EffectPortmento m_effectPortmento;
	EffectPortaNote m_effectPortaNote;
	EffectVibrato m_effectVibrato;
	EffectTremolo m_effectTremolo;
	EffectVolSlide m_effectVolSlide;
	EffectRetrigger m_effectRetrigger;
	EffectNoteSlide m_effectNoteSlide;
	EffectNoteCut m_effectNoteCut;
	EffectNoteDelay m_effectNoteDelay;
	EffectPSGNoise m_effectPSGNoise;
};

static const int ChannelCount[] =
{
	0,	//NONE
	0,	//NONE
	10,	//SYSTEM_GENESIS
	4,	//SYSTEM_SMS
	0,	//SYSTEM_GAMEBOY
	0,	//SYSTEM_PCENGINE
	0,	//SYSTEM_NES
	0,	//SYSTEM_C64
	0	//SYSTEM_YM2151
};

class Stream
{
public:
	enum Direction
	{
		STREAM_OUT,
		STREAM_IN
	};

	Stream(char* ptr)
	{
		//TODO: Output support
		m_direction = STREAM_IN;
		m_ptr = ptr;
	}

	Direction GetDirection() const { return m_direction; }

	template <typename T> void Serialise(T& value)
	{
		value.Serialise(*this);
	}

	void Serialise(int8_t& value)
	{
		value = *(int8_t*)m_ptr;
		m_ptr += sizeof(int8_t);
	}

	void Serialise(uint8_t& value)
	{
		value = *(uint8_t*)m_ptr;
		m_ptr += sizeof(uint8_t);
	}

	void Serialise(uint16_t& value)
	{
		value = *(uint16_t*)m_ptr;
		m_ptr += sizeof(uint16_t);
	}

	void Serialise(uint32_t& value)
	{
		value = *(uint32_t*)m_ptr;
		m_ptr += sizeof(uint32_t);
	}

	void Serialise(int32_t& value)
	{
		value = *(int32_t*)m_ptr;
		m_ptr += sizeof(int32_t);
	}

	void Serialise(std::string& value)
	{
		uint8_t length;
		Serialise(length);
		value.resize(length);
		memcpy(&value[0], m_ptr, length);
		m_ptr += length;
	}

	void Serialise(std::string& value, uint8_t length)
	{
		value.resize(length);
		memcpy(&value[0], m_ptr, length);
		m_ptr += length;
	}

private:
	char* m_ptr;
	Direction m_direction;
};

struct DMFFile
{
	enum System
	{
		SYSTEM_GENESIS = 2,
		SYSTEM_SMS = 3,
		SYSTEM_GAMEBOY = 4,
		SYSTEM_PCENGINE = 5,
		SYSTEM_NES = 6,
		SYSTEM_C64 = 7,
		SYSTEM_YM2151 = 8
	};

	enum InstrumentMode
	{
		INSTRUMENT_PSG,
		INSTRUMENT_FM
	};

	enum FMParams
	{
		FMPARAM_ar,
		FMPARAM_dr,
		FMPARAM_sr,
		FMPARAM_rr,
		FMPARAM_tl,
		FMPARAM_sl,
		FMPARAM_mul,
		FMPARAM_dt,
		FMPARAM_rs,
		FMPARAM_ssg,

		FMPARAM_max
	};

	static const int sFormatStringSize = 16;
	static const int sMaxChannels = 10;
	static const int sMaxInstruments = 32;
	static const int sMaxOperators = 4;
	static const int sMaxEnvelopeSize = 8;
	static const int sMaxWaveTables = 16;
	static const int sWaveTableDataSize = 4;
	static const int sMaxEffects = 4;
	static const int sMaxSamples = 16;
	static const int sTargetSampleRate = 10650;
	static const int sSampleRates[6];

	struct Instrument
	{
		void Serialise(Stream& stream);

		std::string m_name;
		uint8_t m_mode;

		struct ParamDataFM
		{
			void Serialise(Stream& stream);

			uint8_t alg;
			uint8_t fb;
			uint8_t lfo;
			uint8_t lfo2;

			struct Operator
			{
				void Serialise(Stream& stream);

				int8_t am;
				int8_t ar;
				int8_t dr;
				int8_t mul;
				int8_t rr;
				int8_t sl;
				int8_t tl;
				int8_t dt2;
				int8_t rs;
				int8_t dt;
				int8_t d2r;
				int8_t ssg;
			};

			Operator m_operators[sMaxOperators];
		};

		struct ParamDataPSG
		{
			void Serialise(Stream& stream);

			struct Envelope
			{
				void Serialise(Stream& stream);

				uint8_t envelopeSize;
				int32_t* envelopeData;
				uint8_t loopPosition;
			};

			Envelope envelopeVolume;
			Envelope envelopeArpeggio;
			Envelope envelopeDutyNoise;
			Envelope envelopeWaveTable;
			uint8_t arpeggioMode;
		};

		ParamDataFM m_paramsFM;
		ParamDataPSG m_paramsPSG;
	};

	struct WaveTable
	{
		void Serialise(Stream& stream);

		uint32_t m_waveTableSize;
		uint32_t* m_waveTableData;
	};

	struct Channel
	{
		struct PatternPage
		{
			struct Note
			{
				struct Effect
				{
					uint16_t m_effectType;
					uint16_t m_effectValue;
				};

				uint16_t m_note;
				uint16_t m_octave;
				uint16_t m_volume;
				Effect m_effects[sMaxEffects];
				uint16_t m_instrument;
			};

			Note* m_notes;
		};

		uint8_t m_numEffects;
		PatternPage* m_patternPages;
	};
	
	struct Sample
	{
		void Serialise(Stream& stream);

		uint32_t m_sampleSize;
		std::string m_name;
		uint8_t m_sampleRate;
		uint8_t m_pitch;
		uint8_t m_amplitude;
		uint8_t m_bitsPerSample;
		uint16_t* m_sampleData;
	};
	
	void Serialise(Stream& stream);

	std::string m_formatString;
	uint8_t m_fileVersion;
	uint8_t m_systemType;
	std::string m_songName;
	std::string m_songAuthor;
	uint8_t m_highlightA;
	uint8_t m_highlightB;
	uint8_t m_timeBase;
	uint8_t m_tickTimeEven;
	uint8_t m_tickTimeOdd;
	uint8_t m_framesMode;
	uint8_t m_usingCustomHz;
	uint8_t m_customHz1;
	uint8_t m_customHz2;
	uint8_t m_customHz3;
	uint32_t m_numNoteRowsPerPattern;
	uint8_t m_numPatternPages;
	uint8_t m_arpeggioTickSpeed;
	uint8_t* m_patternMatrix[sMaxChannels];
	uint8_t m_numInstruments;
	Instrument m_instruments[sMaxInstruments];
	uint8_t m_numWaveTables;
	WaveTable m_waveTables[sMaxWaveTables];
	Channel m_channels[sMaxChannels];
	uint8_t m_numSamples;
	Sample m_samples[sMaxSamples];
};

struct ESFFile
{
#pragma pack(push, 1)
	struct ParamDataFM
	{
		static const int sMaxOperators = 4;

		uint8_t alg_fb;					// Algorithm and feedback
		uint8_t mul[sMaxOperators];		// Multiplier and detune
		uint8_t tl[sMaxOperators];		// Total level
		uint8_t ar_rs[sMaxOperators];	// Attack rate and release scale
		uint8_t dr[sMaxOperators];		// Decay rate and amplitude modulation
		uint8_t sr[sMaxOperators];		// Sustain rate
		uint8_t rr_sl[sMaxOperators];	// Release rate and sustain level
		uint8_t ssg[sMaxOperators];		// SSG-EG
	};
#pragma pack(pop)

	struct ParamDataPSG
	{

	};
};

//=============================================================================

class ESFOutput
{
private:
    void    SetShortDelay(uint8_t length); // use WaitCounter
    void    SetDelay(uint8_t length);

public:
    std::ofstream ESFFile;

	bool VerboseLog;

	uint8_t     InstrumentOffset;

    int32_t    WaitCounter; // just increase every time you want to wait...

    ESFOutput(std::string);             // ctor
    virtual ~ESFOutput();    // dtor

    void    Wait();
    void    NoteOn(ESFChannel chan,uint8_t note,uint8_t octave = 0);
    void    NoteOff(ESFChannel chan);
    void    SetVolume(ESFChannel chan,uint8_t volume);
	void    SetFrequency(ESFChannel chan, uint16_t freq, bool processDelay = true);
    void    SetPSGNoise(uint8_t noise);
    void    SetInstrument(ESFChannel chan,uint8_t index);
    void    LockChannel(ESFChannel chan);
    void    SetParams(ESFChannel chan,uint8_t params);
	void    SetPan_AMS_FMS(ESFChannel chan, uint8_t pan, uint8_t AMS, uint8_t FMS);
	void    SetRegisterBank0(uint8_t reg, uint8_t value);
	void    SetRegisterBank1(uint8_t reg, uint8_t value);
	void    SetPCMRate(uint8_t rate);
    void    GotoLoop();
    void    SetLoop();
    void    StopPlayback();

    void    InsertPatRow(uint8_t pattern, uint8_t row);

    void    Close();
};

class DMFConverter
{
public:

    ESFOutput * esf;

	bool VerboseLog;

	DMFFile m_dmfFile;

    char*   comp_data;                  // compressed data
    std::vector<uint8_t>   data;                       // uncompressed data

    uint32_t *   PatternData;           // pattern data table

	bool LockChannels;
	bool LoopWholeTrack;
	bool PALMode;

	std::set<uint8_t> UsedChannels;

    bool        UseTables;
    uint8_t     InstrumentTable[256];   // instrument conversion table
	uint8_t     TotalInstruments;
	uint8_t     TotalSamples;
    uint8_t     SampleTable[12];

	uint8_t     InstrumentOffset;

    DMFSystem   System;                 // current system
    uint8_t     ChannelCount;           // amount of channels
    uint8_t     RegionType;             // PAL or NTSC

    uint8_t     SongType;               // song type (BGM, SFX)

    uint8_t     TickBase;               // DMF timing
    uint8_t     TickTimeEvenRow;
    uint8_t     TickTimeOddRow;

    uint8_t     CurrPattern;            // in pattern matrix.
    uint8_t     CurrRow;                // in pattern

    bool        SkipPattern;            // set if a command to skip the rest
                                        // of the pattern was found
    uint8_t     NextPattern;            // next pattern
    uint8_t     NextRow;                // row to start next pattern at

    uint8_t     TotalRowsPerPattern;
    uint8_t     TotalPatterns;
    uint32_t   * PatternList;
    uint8_t     ArpTickSpeed;           // arpeggio tick speed
//    PSGNoiseMode    NoiseMode;        // Set here because this option affects
                                        // multiple channels.
    bool        PSGNoiseFreq;           // 1: use PSG3 frequency
    bool        PSGPeriodicNoise;       // 1: use periodic noise
	bool        PSGNoiseEnvelope;       // 1: use PSG noise envelope

    bool        DACEnabled;             // Set here because the 17xx effect can be
                                        // done on any channel.

	bool        LFOEnable;
	uint8_t     LFOFreq;

    Channel     Channels[10];
    Channel     LoopState[10];

    bool        LoopFound;
    uint8_t     LoopPattern;
    uint8_t     LoopRow;

    bool        LoopFlag;               // set to 1 when loop is found,
                                        // then 2 when we're ready to loop for real

    DMFConverter(ESFOutput ** esfout);             // ctor
    virtual     ~DMFConverter();    // dtor
    bool        Initialize(const char* Filename);     // load DMF
    bool        Parse();    // parse DMF
	bool        ParseChannelRow(uint8_t chan, uint32_t CurrPattern, uint32_t CurrRow); // parse channel
	EffectStage GetActiveEffectStage(uint8_t chan);
    int         ProcessActiveEffects(uint8_t chan);
    void        NoteOn(uint8_t chan); // checks channel type and sends appropriate command to ESF
	void        SetFrequency(uint8_t chan, uint32_t FMSemitone, bool processDelay = true);
	void        OutputInstrument(int instrumentIdx, const char* filename); // outputs an FM instrument or PSG envelope
	void        OutputSample(int sampleIdx, const char* filename, bool convertFormat);

    uint16_t    GetFreq(ChannelType chan);

};

/* Helper functions */
double divrest(double a, int b);
uint16_t fmfreq(double a);
uint16_t fmfreq2(long a);
void FindInstruments(char * inisection, INIReader *ini, DMFConverter *dmf);

//=============================================================================

// Some macros:
#define chpat(channel,pattern) (channel*TotalPatterns+pattern)
#define rowsize(channel,row) (row*(8+(4*Channels[channel].EffectCount)))
#define hexy(stream,var,pre) (stream<<pre<<setiosflags(ios::right)<<setbase(16)<<setfill('0')<<setw(2)<<(int)var<<setbase(0)<<resetiosflags(ios::right)<<setfill(' ')<<setw(0))

#endif // DMF2ESF_H_INCLUDED
