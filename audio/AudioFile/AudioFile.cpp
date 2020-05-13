#include <iostream>
#include <fstream> // C++ ifstream ofstream
#include <stdio.h> // C FILE open

#include "AudioFile.h"

//=============================================================
// Writes an audio file with a given number of channels, sample rate, bit deoth and format
// Returns true if it was successful
bool writeTestAudioFile (int numChannels, int sampleRate, int bitDepth, AudioFileFormat format, int recordTime=10 /*second*/, int div=1)
{
    float sampleRateAsFloat = (float) sampleRate;
    
    AudioFile<float> audioFile;
    
    audioFile.setAudioBufferSize (numChannels, sampleRate * recordTime);
    
    for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
    {
        float sample = sinf (2./div * M_PI * ((float) i / sampleRateAsFloat) * 440.);
        
        for (int k = 0; k < audioFile.getNumChannels(); k++)
            audioFile.samples[k][i] = sample * 0.5;
    }
    
    audioFile.setSampleRate (sampleRate);
    audioFile.setBitDepth (bitDepth);
    
    // std::string numChannelsAsString = numChannels == 1 ? "mono" : "stereo";
    std::string numChannelsAsString = std::to_string (numChannels);
    std::string bitDepthAsString = std::to_string (bitDepth);
    std::string sampleRateAsString = std::to_string (sampleRate);

    if (format == AudioFileFormat::Wave)
    {
        return audioFile.save ("./audio-write-tests/" + std::to_string(div) + "_" + numChannelsAsString + "Ch_" + sampleRateAsString + "_" + bitDepthAsString + "Bit" + ".wav", format);
    }
    
    else if (format == AudioFileFormat::Aiff)
    {
        return audioFile.save ("./audio-write-tests/" + std::to_string(div) + "_" + numChannelsAsString + "Ch_" + sampleRateAsString + "_" + bitDepthAsString + "Bit" + ".aif", format);
    }
    
    return false;
}

void audio_file_generater()
{
    std::vector<int> sampleRates = {16000};
    std::vector<int> bitDepths   = {16};
    std::vector<int> numChannels = {1, 2, 4};
    std::vector<AudioFileFormat> audioFormats = {AudioFileFormat::Wave};
    
    for (auto& sampleRate : sampleRates)
    {
        for (auto& bitDepth : bitDepths)
        {
            for (auto& channels : numChannels)
            {
                for (auto& format : audioFormats)
                {
                    writeTestAudioFile (channels, sampleRate, bitDepth, format);
                }
            }
        }
    }
}

#define out_file "./audio-write-tests/out_4Ch_16K_16Bit_10s.pcm"

#define tmp_file "./audio-write-tests/tmp_4Ch_16K_16Bit_10s.pcm"
#define ch1_file "./audio-write-tests/1_1Ch_16000_16Bit.wav"
#define ch2_file "./audio-write-tests/2_1Ch_16000_16Bit.wav"
#define ch3_file "./audio-write-tests/3_1Ch_16000_16Bit.wav"
#define ch4_file "./audio-write-tests/4_1Ch_16000_16Bit.wav"

void merge_pcm_audio_file()
{
    FILE *out = fopen(tmp_file, "wb");
    FILE *ch1 = fopen(ch1_file, "rb");
    FILE *ch2 = fopen(ch2_file, "rb");
    FILE *ch3 = fopen(ch3_file, "rb");
    FILE *ch4 = fopen(ch4_file, "rb");

    size_t len1= 0, len2 = 0, len3 =0, len4 =0;
    short dmic1= 0, dmic2= 0, dspk1=0, dspk2=0;

    // skip wav header
    fseek(ch1, 44, SEEK_SET);
    fseek(ch2, 44, SEEK_SET);
    fseek(ch3, 44, SEEK_SET);
    fseek(ch4, 44, SEEK_SET);

    do{
        len1 = fread(&dmic1, 1, sizeof(short), ch1);
        fwrite(&dmic1, 1, sizeof(short), out);

        len2 = fread(&dmic2, 1, sizeof(short), ch2);
        fwrite(&dmic2, 1, sizeof(short), out);

        len3 = fread(&dspk1, 1, sizeof(short), ch3);
        fwrite(&dspk1, 1, sizeof(short), out);

        len4 = fread(&dspk2, 1, sizeof(short), ch4);
        fwrite(&dspk2, 1, sizeof(short), out);
    } while(len1 > 0 && len2 > 0 && len3 >0 && len4 >0);

    fclose(out);
    fclose(ch1);
    fclose(ch2);
    fclose(ch3);
    fclose(ch4);
}

void multi_channel_audio_generater()
{
    for(int i=1; i<5; i++)
    {
        writeTestAudioFile(1, 16000, 16, AudioFileFormat::Wave, 10, i);
    }
    merge_pcm_audio_file();

    AudioFile<float> audioFile;
    short audioBuff[1 * 4 * 16]; // 1ms, 4channel, 16samples (16bit -- short)

    audioFile.init(4, 16000, 16, 10 /*second*/, out_file);

    std::ifstream in(tmp_file, std::ios::in | std::ios::binary);
    if (in.is_open())
    {
        while (! in.eof())
        {
            in.read((char *)audioBuff, sizeof(audioBuff));

            audioFile.assignToAudioBuffer(audioBuff, sizeof(audioBuff), true);
        }
    }
    else
    {
        std::cerr << "open file failed!" << std::endl;
    }
}

int main()
{
    // audio_file_generater();
    multi_channel_audio_generater();

    return 0;
}

/* Complie: g++ AudioFile.cpp -Wall -std=c++11 -o testWriteAudioFile
   Execute: ./testWriteAudioFile
   AudioTool: Audacity (File->Imort->RAW DATA)
 */