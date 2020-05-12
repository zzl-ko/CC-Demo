#include <iostream>

#include "AudioFile.h"

//=============================================================
// Writes an audio file with a given number of channels, sample rate, bit deoth and format
// Returns true if it was successful
bool writeTestAudioFile (int numChannels, int sampleRate, int bitDepth, AudioFileFormat format)
{
    float sampleRateAsFloat = (float) sampleRate;
    
    AudioFile<float> audioFile;
    
    audioFile.setAudioBufferSize (numChannels, sampleRate * 4);
    
    for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
    {
        float sample = sinf (2. * M_PI * ((float) i / sampleRateAsFloat) * 440.) ;
        
        for (int k = 0; k < audioFile.getNumChannels(); k++)
            audioFile.samples[k][i] = sample * 0.5;
    }
    
    audioFile.setSampleRate (sampleRate);
    audioFile.setBitDepth (bitDepth);
    
    std::string numChannelsAsString = numChannels == 1 ? "mono" : "stereo";
    std::string bitDepthAsString = std::to_string (bitDepth);
    std::string sampleRateAsString = std::to_string (sampleRate);

    if (format == AudioFileFormat::Wave)
    {
        return audioFile.save ("./audio-write-tests/" + numChannelsAsString + "_" + sampleRateAsString + "_" + bitDepthAsString + "bit" + ".wav", format);
    }
    
    else if (format == AudioFileFormat::Aiff)
    {
        return audioFile.save ("./audio-write-tests/" + numChannelsAsString + "_" + sampleRateAsString + "_" + bitDepthAsString + "bit" + ".aif", format);
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

int main()
{
    audio_file_generater();

    return 0;
}

/* Complie: g++ AudioFile.cpp -Wall -std=c++11 -o testWriteAudioFile
   Execute: ./testWriteAudioFile
   AudioTool: Audacity (File->Imort->RAW DATA)
 */