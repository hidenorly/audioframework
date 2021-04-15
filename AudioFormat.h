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
	enum {
		PCM_8BIT,
		PCM_16BIT,
		PCM_24BIT_PACKED,
		PCM_32BIT,
		PCM_FLOAT,
		PCM_UNKNOWN
	} ENCODING;

	enum {
		CHANNEL_MONO,
		CHANNEL_STEREO,
		CHANNEL_4CH,
		CHANNEL_5CH,
		CHANNEL_5_1CH,
		CHANNEL_5_1_2CH,
		CHANNEL_5_0_2CH,
		CHANNEL_7_1CH,
		CHANNEL_UNKNOWN
	} CHANNEL;

	enum {
		SPEAKER_MONO,
		SPEAKER_STEREO,
		SPEAKER_FL_FR_SL_SR,
		SPEAKER_FL_FR_C_SL_SR,
		SPEAKER_FL_FR_C_SL_SR_SW,
		SPEAKER_FL_FR_C_SL_SR_SW_EL_ER, // Atmos Enabled L/R
		HEADPHONE_MONO,
		HEADPHONE_STEREO,
		UNKNOWN
	} PRESENTATION;

	enum {
		SAMPLING_RATE_8_KHZ = 8,
		SAMPLING_RATE_16_KHZ = 16,
		SAMPLING_RATE_44_1_KHZ = 44.1f,
		SAMPLING_RATE_DEFAULT = SAMPLING_RATE_48_KHZ = 48,
		SAMPLING_RATE_88_2_KHZ = 88.2f,
		SAMPLING_RATE_96_KHZ = 96,
		SAMPLING_RATE_192_KHZ = 192,
		SAMPLING_RATE_UNKNOWN,
	} SAMPLING_RATE;
};

#endif /* __AUDIO_FORMAT_H__ */
