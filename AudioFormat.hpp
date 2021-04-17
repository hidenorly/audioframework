/* 
  Copyright (C) 2021 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __AUDIO_FORMAT_H__
#define __AUDIO_FORMAT_H__

class AudioFormat
{
public:
	enum ENCODING {
		PCM_8BIT,
		PCM_16BIT,
		PCM_24BIT_PACKED,
		PCM_32BIT,
		PCM_FLOAT,
		PCM_UNKNOWN,
		ENCODING_DEFAULT = PCM_16BIT
	};

	static int getSampleByte(ENCODING encoding)
	{
		int sampleByte = 0;

		switch( encoding ){
			case PCM_8BIT:
				sampleByte = 1;
				break;
			case PCM_16BIT:
			case PCM_UNKNOWN:
				sampleByte = 2;
				break;
			case PCM_24BIT_PACKED:
				sampleByte = 3;
				break;
			case PCM_32BIT:
			case PCM_FLOAT:
				sampleByte = 4;
				break;
		}

		return sampleByte;
	}

	enum CHANNEL {
		CHANNEL_MONO,
		CHANNEL_STEREO,
		CHANNEL_4CH,
		CHANNEL_5CH,
		CHANNEL_5_1CH,
		CHANNEL_5_1_2CH,
		CHANNEL_5_0_2CH,
		CHANNEL_7_1CH,
		CHANNEL_UNKNOWN,
		CHANNEL_DEFAULT = CHANNEL_STEREO
	};

	static int getNumberOfChannels(CHANNEL channel)
	{
		int numOfChannels = 0;

		switch( channel ){
			case CHANNEL_MONO:
				numOfChannels = 1;
				break;
			case CHANNEL_STEREO:
			case CHANNEL_UNKNOWN:
				numOfChannels = 2;
				break;
			case CHANNEL_4CH:
				numOfChannels = 4;
				break;
			case CHANNEL_5CH:
				numOfChannels = 5;
				break;
			case CHANNEL_5_1CH:
				numOfChannels = 6;
				break;
			case CHANNEL_5_1_2CH:
				numOfChannels = 8;
				break;
			case CHANNEL_5_0_2CH:
				numOfChannels = 7;
				break;
			case CHANNEL_7_1CH:
				numOfChannels = 8;
				break;
		}

		return numOfChannels;
	}

	static int getChannelsSampleByte(ENCODING encoding, CHANNEL channel)
	{
		return getSampleByte(encoding) * getNumberOfChannels(channel);
	}

	enum PRESENTATION {
		SPEAKER_MONO,
		SPEAKER_STEREO,
		SPEAKER_FL_FR_SL_SR,
		SPEAKER_FL_FR_C_SL_SR,
		SPEAKER_FL_FR_C_SL_SR_SW,
		SPEAKER_FL_FR_C_SL_SR_SW_EL_ER, // Atmos Enabled L/R
		HEADPHONE_MONO,
		HEADPHONE_STEREO,
		PRESENTATION_DEFAULT = SPEAKER_STEREO,
		UNKNOWN
	};

	enum SAMPLING_RATE {
		SAMPLING_RATE_8_KHZ = 8000,
		SAMPLING_RATE_16_KHZ = 16000,
		SAMPLING_RATE_44_1_KHZ = 44100,
		SAMPLING_RATE_48_KHZ = 48000,
		SAMPLING_RATE_88_2_KHZ = 88200,
		SAMPLING_RATE_96_KHZ = 96000,
		SAMPLING_RATE_192_KHZ = 192000,
		SAMPLING_RATE_DEFAULT = SAMPLING_RATE_48_KHZ,
		SAMPLING_RATE_UNKNOWN,
	};

};

#endif /* __AUDIO_FORMAT_H__ */
