#include "dmf2esf.h"

using namespace std;

bool OutputInstruments = false;
bool ASMOut = false;
bool ExCommands = false;

static uint32_t waitDebugger = 1;

int main(int argc, char *argv[])
{
	//while (waitDebugger)
	{
		if(++waitDebugger > 100)
			waitDebugger = 1;
	}

    int     InputId = 0;
    int     OutputId = 0;
    int     Opt = 0;

    OutputInstruments = false;
    ExCommands = false;
	bool Verbose = false;
	bool OutputChannelMask = false;

    DMFConverter * dmf;
    ESFOutput * esf;
    INIReader * ini;

    fprintf(stdout, "\nDMF2ESF ver %d.%d (built %s %s)\n", MAJORVER, MINORVER, __DATE__, __TIME__);
    fprintf(stdout, "Copyright 2013-2014 ctr.\n");
	fprintf(stdout, "Extended by Big Evil Corporation 2017.\n");
    fprintf(stdout, "Licensed under GPLv2, see LICENSE.txt.\n\n");
	fprintf(stdout, "Includes source from Secret Rabbit Code, see libsamplerate\\COPYING.\n\n");

	bool error = false;

	struct File
	{
		std::string InFilename;
		std::string OutFilename;
		bool loopWholeTrack = false;
		bool lockChannels = false;
		bool PALMode = false;
		uint16_t ChannelMask = 0;
	};

	std::vector<File> filenames;

	const char* configFile = NULL;
	int instrumentIdxOffset = 0;
	
	File currentFile;

    if(argc > 1)
    {
		for(int i = 1; i < argc; ++i)
		{
			/* Check for options */
			if(!strcmp(argv[i], "-i"))
			{
				OutputInstruments = true;
			}
			else if(!strcmp(argv[i], "-a"))
			{
				ASMOut = true;
			}
			else if(!strcmp(argv[i], "-e"))
			{
				ExCommands = true;
			}
			else if(!strcmp(argv[i], "-m"))
			{
				OutputChannelMask = true;
			}
			else if(!strcmp(argv[i], "-v"))
			{
				Verbose = true;
			}
			else if(!strcmp(argv[i], "-l"))
			{
				currentFile.loopWholeTrack = true;
			}
			else if(!strcmp(argv[i], "-s"))
			{
				currentFile.lockChannels = true;
			}
			else if(!strcmp(argv[i], "-p"))
			{
				currentFile.PALMode = true;
			}
			else if(!strcmp(argv[i], "-c"))
			{
				i++;
				if(i < argc)
				{
					configFile = argv[i];
				}
				else
				{
					error = true;
				}
			}
			else if(!strcmp(argv[i], "-instroffset"))
			{
				i++;
				if(i < argc)
				{
					instrumentIdxOffset = atoi(argv[i]);
				}
				else
				{
					error = true;
				}
			}
			else
			{
				//Filename
				std::string inFile = argv[i++];

				if(i < argc)
				{
					std::string outFile = argv[i];
					currentFile.InFilename = inFile;
					currentFile.OutFilename = outFile;

					filenames.push_back(currentFile);
					currentFile = File();
				}
				else
				{
					error = true;
				}
			}
		}
    }

	if(error || filenames.size() == 0) // no input file?
    {
        //print usage and exit
        fprintf(stderr, "Usage: dmf2esf <options> input output <input output> <input output>...\n");
        fprintf(stderr, "Options:\n");
		fprintf(stderr, "\t-c <config.ini> : Use INI config file\n");
        fprintf(stderr, "\t-i : Output FM instrument data\n");
        fprintf(stderr, "\t-a : Output ESF as an assembly file\n");
		fprintf(stderr, "\t-l : Loop whole track\n");
		fprintf(stderr, "\t-s : SFX (locks used channels)\n");
		fprintf(stderr, "\t-p : PAL mode (sets FM timer to PAL speed)\n");
        fprintf(stderr, "\t-e : Use EchoEx extended commands\n");
		fprintf(stderr, "\t-m : Output used channel mask\n");
		fprintf(stderr, "\t-v : Verbose output\n");
		fprintf(stderr, "\t-instroffset : Offset the first instrument index\n");
        fprintf(stderr, "Please read \"readme.md\" for further usage instructions.\n");

        for(int a=0;a<12*7;a++)
        {
            fprintf(stderr, "%d\n", fmfreq(a));
        }
    }
    else
    {
#if 0
        if(configFile)
        {
            fprintf(stdout, "Loading ini: %s\n",configFile);
            ini = new INIReader(configFile);

            if (ini->ParseError() < 0) {
                fprintf(stderr, "Failed to load ini: %s\n",argv[InputId]);
                return EXIT_FAILURE;
            }

            bool SearchFlag=true;
            int TrackIndex=0;
            char IniSection[64];
            string input;

            while(SearchFlag)
            {
                sprintf(IniSection,"%d",TrackIndex);
                fprintf(stdout, "======== Reading [%s] ========\n",IniSection);
                string input = ini->Get(IniSection,"input","");
                if(input.length() > 0)
                {
                    if(input.rfind(".") < 0)
                    {
                        fprintf(stderr, "Silly input\n");
                        return EXIT_FAILURE;
                    }

                    string output = ini->Get(IniSection,"output","");
                    bool TrackSFX = ini->GetBoolean(IniSection,"sfx",false);
                    if(output.length() == 0)
                        output = input.substr(0,input.rfind("."));

                    if(output.rfind(".") < 0)
                    {
                        fprintf(stderr, "Silly output\n");
                        return EXIT_FAILURE;
                    }

                    fprintf(stderr, "Converting %s to %s\n",input.c_str(), output.c_str());

                    esf = new ESFOutput(output);
                    dmf = new DMFConverter(&esf);

                    //FindInstruments(IniSection,ini,dmf);

                    if(dmf->Initialize(input.c_str()))
                    {
                        fprintf(stderr, "Failed to initialize, aborting\n");
                        break;
                    }
                    if(dmf->Parse())
                    {
                        fprintf(stderr, "Conversion failed, aborting\n");
                        break;
                    }
                    fprintf(stdout, "Successfully converted, continuing.\n");
                    delete esf;
                    delete dmf;
                }
                else
                {
                    //fprintf(stderr, "invalid input '%s' (%d)\n",input.c_str(), input.length());
                    break;
                }
                TrackIndex++;
            }
            delete ini;
        }
        else
#endif
        {
			for(int i = 0; i < filenames.size(); i++)
			{
				File& file = filenames[i];

				fprintf(stdout, "Converting: %s from instrument offset %i\n", file.InFilename.c_str(), instrumentIdxOffset);

				esf = new ESFOutput(file.OutFilename);
				dmf = new DMFConverter(&esf);

				esf->InstrumentOffset = instrumentIdxOffset;
				dmf->InstrumentOffset = instrumentIdxOffset;
				esf->VerboseLog = Verbose;
				dmf->VerboseLog = Verbose;
				dmf->PALMode = file.PALMode;
				dmf->LoopWholeTrack = file.loopWholeTrack;
				dmf->LockChannels = file.lockChannels;

				//Initialize, and write all instruments if last file
				if(dmf->Initialize(file.InFilename.c_str(), i == (filenames.size() - 1)))
				{
					fprintf(stderr, "Aborting\n");
					return EXIT_FAILURE;
				}
				if(dmf->Parse())
				{
					fprintf(stderr, "Conversion aborted.\n");
					return EXIT_FAILURE;
				}

				fprintf(stdout, "Done. Instruments: %i Samples: %i\n", dmf->TotalInstruments, dmf->TotalSamples);

				instrumentIdxOffset += dmf->TotalInstruments + dmf->TotalSamples;

				for(std::set<uint8_t>::iterator it = dmf->UsedChannels.begin(), end = dmf->UsedChannels.end(); it != end; ++it)
				{
					file.ChannelMask |= 1 << *it;
				}

				delete dmf;
				delete esf;
			}

			if(OutputChannelMask)
			{
				for(int i = 0; i < filenames.size(); i++)
				{
					std::cout << "Used channel mask for " << filenames[i].OutFilename << ":" << std::endl;
					hexy(std::cout, filenames[i].ChannelMask, "0x");
					std::cout << std::endl;
				}
			}
        }
    }
    return EXIT_SUCCESS;
}
