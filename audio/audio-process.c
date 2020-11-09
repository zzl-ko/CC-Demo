#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#define ch1_file  "raw_mic0.wav"
#define ch2_file  "raw_mic1.wav"
#define ch3_file  "raw_mic2.wav"
#define ch4_file  "raw_mic3.wav"
#define mix_file  "raw_spk_mixed.wav"


void merge_pcm()
{
    size_t len1 = 0, len2 = 0, len3 = 0, len4 = 0;
    short  dch1 = 0, dch2 = 0, dch3 = 0, dch4 = 0;

    // printf("sizeof(short)=%ld\n", sizeof(short) );

    FILE *mix = fopen(mix_file, "wb");
    FILE *ch1 = fopen(ch1_file, "rb");
    FILE *ch2 = fopen(ch2_file, "rb");
    FILE *ch3 = fopen(ch3_file, "rb");
    FILE *ch4 = fopen(ch4_file, "rb");

    fseek(ch1, 44, SEEK_SET);
    fseek(ch2, 44, SEEK_SET);
    fseek(ch3, 44, SEEK_SET);
    fseek(ch4, 44, SEEK_SET);

    do{
        len1 = fread(&dch1, 1, sizeof(short), ch1);
        fwrite(&dch1, 1, sizeof(short), mix);

        len2 = fread(&dch2, 1, sizeof(short), ch2);
        fwrite(&dch2, 1, sizeof(short), mix);

        len3 = fread(&dch3, 1, sizeof(short), ch3);
        fwrite(&dch3, 1, sizeof(short), mix);

        len4 = fread(&dch4, 1, sizeof(short), ch4);
        fwrite(&dch4, 1, sizeof(short), mix);
    } while(len1 > 0 && len2 > 0 && len3 >0 && len4 >0);

    fclose(mix);
    fclose(ch1);
    fclose(ch2);
    fclose(ch3);
    fclose(ch4);
}

void split_pcm()
{
    size_t len0 = 0;
    short  dmix = 0;

    FILE *mix = fopen(mix_file, "rb");
    FILE *ch1 = fopen(ch1_file, "wb");
    FILE *ch2 = fopen(ch2_file, "wb");
    FILE *ch3 = fopen(ch3_file, "wb");
    FILE *ch4 = fopen(ch4_file, "wb");

    fseek(mix, 44, SEEK_SET);

    do {
        len0 = fread(&dmix, 1, sizeof(short), mix);
        fwrite(&dmix, 1, sizeof(short), ch1);

        len0 = fread(&dmix, 1, sizeof(short), mix);
        fwrite(&dmix, 1, sizeof(short), ch2);

        // len0 = fread(&dmix, 1, sizeof(short), mix);
        // fwrite(&dmix, 1, sizeof(short), ch3);

        // len0 = fread(&dmix, 1, sizeof(short), mix);
        // fwrite(&dmix, 1, sizeof(short), ch4);
    } while(len0 > 0);

    fclose(mix);
    fclose(ch1);
    fclose(ch2);
    fclose(ch3);
    fclose(ch4);
}

int main()
{
    split_pcm();

    return 0;
}

/* Compile: gcc audio-process.c -o audio-process
   Execute: ./audio-process
 */