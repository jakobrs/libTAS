/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AudioPlayer.h"

#ifdef LIBTAS_ENABLE_SOUNDPLAYBACK

#include "../logging.h"
#include "../global.h" // shared_config
#include "../GlobalState.h"
#include "../hook.h"

namespace libtas {

snd_pcm_t *AudioPlayer::phandle;
bool AudioPlayer::inited = false;

DEFINE_ORIG_POINTER(snd_pcm_open);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_malloc);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_any);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_access);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_format);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_rate);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_buffer_size_near);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_channels);
DEFINE_ORIG_POINTER(snd_pcm_hw_params);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_free);
DEFINE_ORIG_POINTER(snd_pcm_prepare);
DEFINE_ORIG_POINTER(snd_pcm_writei);
DEFINE_ORIG_POINTER(snd_pcm_close);

bool AudioPlayer::init(snd_pcm_format_t format, int nbChannels, unsigned int frequency)
{
    debuglog(LCF_SOUND, "Init audio player");

    GlobalNative gn;

    LINK_NAMESPACE(snd_pcm_open, nullptr);
    if (orig::snd_pcm_open(&phandle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        debuglog(LCF_SOUND, "  Cannot open default audio device");
    }

    snd_pcm_hw_params_t *hw_params;

    LINK_NAMESPACE(snd_pcm_hw_params_malloc, nullptr);
    if (orig::snd_pcm_hw_params_malloc(&hw_params) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_malloc failed");
        return false;
    }

    LINK_NAMESPACE(snd_pcm_hw_params_any, nullptr);
    if (orig::snd_pcm_hw_params_any(phandle, hw_params) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_any failed");
        return false;
    }

    LINK_NAMESPACE(snd_pcm_hw_params_set_access, nullptr);
    if (orig::snd_pcm_hw_params_set_access(phandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_access failed");
        return false;
    }

    LINK_NAMESPACE(snd_pcm_hw_params_set_format, nullptr);
    if (orig::snd_pcm_hw_params_set_format(phandle, hw_params, format) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_format failed");
        return false;
    }

    int dir = 0;
    LINK_NAMESPACE(snd_pcm_hw_params_set_rate, nullptr);
    if (orig::snd_pcm_hw_params_set_rate(phandle, hw_params, frequency, dir) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_rate failed");
        return false;
    }

    LINK_NAMESPACE(snd_pcm_hw_params_set_channels, nullptr);
    if (orig::snd_pcm_hw_params_set_channels(phandle, hw_params, nbChannels) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_channels failed (", nbChannels, ")");
        return false;
    }

    snd_pcm_uframes_t buffer_size = 2 * frequency / ((shared_config.framerate>0)?shared_config.framerate:30);
    debuglog(LCF_SOUND, "  Buffer size is ", buffer_size);
    LINK_NAMESPACE(snd_pcm_hw_params_set_buffer_size_near, nullptr);
    if (orig::snd_pcm_hw_params_set_buffer_size_near(phandle, hw_params, &buffer_size) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_rate_near failed");
        return false;
    }
    debuglog(LCF_SOUND, "  new buffer size is ", buffer_size);

    LINK_NAMESPACE(snd_pcm_hw_params, nullptr);
    if (orig::snd_pcm_hw_params(phandle, hw_params) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params failed");
        return false;
    }

    LINK_NAMESPACE(snd_pcm_prepare, nullptr);
    if (orig::snd_pcm_prepare(phandle) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_prepare failed");
        return false;
    }

    LINK_NAMESPACE(snd_pcm_hw_params_free, nullptr);
    orig::snd_pcm_hw_params_free(hw_params);

    return true;
}

bool AudioPlayer::play(AudioContext& ac)
{
    if (!inited) {
        snd_pcm_format_t format;
        if (ac.outBitDepth == 8)
            format = SND_PCM_FORMAT_U8;
        if (ac.outBitDepth == 16)
            format = SND_PCM_FORMAT_S16_LE;
        if (!init(format, ac.outNbChannels, static_cast<unsigned int>(ac.outFrequency)))
            return false;
        inited = true;
    }

    if (shared_config.fastforward)
        return true;

    debuglog(LCF_SOUND, "Play an audio frame");
    int err;
    LINK_NAMESPACE(snd_pcm_writei, nullptr);
    {
        GlobalNative gn;
        err = orig::snd_pcm_writei(phandle, ac.outSamples.data(), ac.outNbSamples);
    }
	if (err < 0) {
		if (err == -EPIPE) {
			debuglog(LCF_SOUND, "  Underrun");
            {
                GlobalNative gn;
	            err = orig::snd_pcm_prepare(phandle);
            }
			if (err < 0) {
				debuglog(LCF_SOUND | LCF_ERROR, "  Can't recovery from underrun, prepare failed: ", snd_strerror(err));
			    return false;
            }
            else {
                {
                    GlobalNative gn;
                    orig::snd_pcm_writei(phandle, ac.outSamples.data(), ac.outNbSamples);
                }
            }
		}
		else {
			debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_writei() failed: ", snd_strerror (err));
			return false;
		}
	}

    return true;
}

void AudioPlayer::close()
{
    if (inited) {
        LINK_NAMESPACE(snd_pcm_close, nullptr);
        MYASSERT(orig::snd_pcm_close(phandle) == 0)
        inited = false;
    }
}

}

#endif
