#include "driver.h"
#include "wiz_lib.h"

int soundcard,usestereo;
int attenuation = 0;
int master_volume = 100;

static INT16 *stream_cache_data;
static int stream_cache_len;
static int stream_cache_stereo;
static int samples_per_frame;

int msdos_init_sound(void)
{
	/* Ask the user if no soundcard was chosen */
	if (soundcard == -1)
	{
		soundcard=1;
	}

	if (soundcard == 0)     /* silence */
	{
		/* update the Machine structure to show that sound is disabled */
		Machine->sample_rate = 0;
		return 0;
	}

	/* use stereo output if supported */
	wiz_sound_stereo=0;
	if (usestereo)
	{
		if (Machine->drv->sound_attributes & SOUND_SUPPORTS_STEREO)
			wiz_sound_stereo=1;
	}

	stream_cache_data = 0;
	stream_cache_len = 0;
	stream_cache_stereo = 0;

	wiz_sound_rate=options.samplerate;

	/* update the Machine structure to reflect the actual sample rate */
	Machine->sample_rate = wiz_sound_rate;

	logerror("set stereo: %d\n",wiz_sound_stereo);
	logerror("set sample rate: %d\n",Machine->sample_rate);

	osd_set_mastervolume(attenuation);	/* set the startup volume */

	return 0;
}

void msdos_shutdown_sound(void)
{
	return;
}

int osd_start_audio_stream(int stereo)
{
	if (stereo) stereo = 1;	/* make sure it's either 0 or 1 */

	stream_cache_stereo = stereo;
	wiz_sound_stereo = stereo;

	/* determine the number of samples per frame */
	samples_per_frame = Machine->sample_rate / Machine->drv->frames_per_second;

	if (Machine->sample_rate == 0) return 0;

	/* open audio device */
	wiz_sound_thread_start();

	return samples_per_frame;
}

void osd_stop_audio_stream(void)
{
	if (Machine->sample_rate != 0)
	{
		wiz_sound_thread_stop();
	}
}

static void updateaudiostream(void)
{
	INT16 *data = stream_cache_data;
	int stereo = stream_cache_stereo;
	int len = stream_cache_len;
	int buflen;

	if (stereo)
	{
		buflen=len*2*sizeof(signed short);
	}
	else
	{
		buflen=len*sizeof(signed short);
	}
	wiz_sound_play(data,buflen);
}

int osd_update_audio_stream(INT16 *buffer)
{
	stream_cache_data = buffer;
	stream_cache_len = samples_per_frame;

	return samples_per_frame;
}

int msdos_update_audio(void)
{
	if (Machine->sample_rate == 0 || stream_cache_data == 0) return 0;
	profiler_mark(PROFILER_MIXER);
	updateaudiostream();
	profiler_mark(PROFILER_END);
	return 0;
}

/* attenuation in dB */
void osd_set_mastervolume(int _attenuation)
{
	float volume;


	if (_attenuation > 0) _attenuation = 0;
	if (_attenuation < -32) _attenuation = -32;

	attenuation = _attenuation;

 	volume = 100.0;	/* range is 0-100 */
	while (_attenuation++ < 0)
		volume /= 1.122018454;	/* = (10 ^ (1/20)) = 1dB */

	master_volume = volume;

	wiz_sound_volume(master_volume,master_volume);
}

int osd_get_mastervolume(void)
{
	return attenuation;
}

void osd_sound_enable(int enable_it)
{
	if (enable_it)
		wiz_sound_volume(master_volume,master_volume);
	else
		wiz_sound_volume(0,0);
}

void osd_opl_control(int chip,int reg)
{
}

void osd_opl_write(int chip,int data)
{
}
