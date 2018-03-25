/*===========================================================================*\
|* Functions for DMF2ESF                                                     *|
|* Some were taken from XM2ESF by Oerg866 and then ported by ctr             *|
\*===========================================================================*/

#include "dmf2esf.h"

double divrest(double a, int b)
{
    int last = 0;
    for(int i=0;i<=a;i++)
    {
        if((double)(i/b) - (int)(i/b) == 0)
            last=i;
    }
    return a-last;
}

uint16_t fmfreq(double a)
{
    double  temp2 = 644*(pow(2,(fmod(a,12)/12)));
    long    temp = ((int)(a/12))*2048;
    return  ((int)temp2 & 2047) + temp;
}
uint16_t fmfreq2(long a)
{
    long    temp2 = 644*(pow(2,(fmod(a,12)/12)));
    long    temp = (a/12)*2048;
    return  ((int)temp2 & 2047) + temp;
}
/*
void FindInstruments(char * inisection, INIReader *ini, DMFConverter *dmf)
{
    const std::string samplenotes[] = {
        "c","cs","d","ds","e","f","fs","g","gs","a","as","b"
    };
    int inst_counter;
    char search_string[50];

    dmf->UseTables = true;

    for(inst_counter=0;inst_counter<256;inst_counter++)
    {
        // find instruments
        sprintf(search_string,"ins_%02x",inst_counter);
        dmf->InstrumentTable[inst_counter] = (uint8_t) ini->GetInteger(inisection,search_string,0);
        #if DEBUG
            if(ini->GetInteger(inisection,search_string,0) > 0)
                fprintf(stdout, "Assigned DMF instrument %d to ESF instrument %ld\n", inst_counter,ini->GetInteger(inisection,search_string,0));
        #endif
        if(inst_counter<12)
        {
            // find samples
            sprintf(search_string,"pcm_%s",samplenotes[inst_counter].c_str());
            dmf->SampleTable[inst_counter] = (uint8_t) ini->GetInteger(inisection,search_string,0);
            #if DEBUG
                if(ini->GetInteger(inisection,search_string,0) > 0)
                    fprintf(stdout, "Assigned DMF sample %d (%s) to ESF sample %ld\n", inst_counter,samplenotes[inst_counter].c_str(),ini->GetInteger(inisection,search_string,0));
            #endif
        }
    }
    return;
}
*/
