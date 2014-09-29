#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "options.h"
#include "thread.h"
#include "exfun.h"
#include "utils2.h"

#define AUDIO_DEV "/dev/dsp"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define FORMAT_PCM 1
#define AUDIO_BUFFER_SIZE 1024
#define MAX_WAVHDR_SIZE 64
#define MAX_PATH_LEN 128

typedef enum {
	MONO = 0x01,
	STEREO
} chan;

typedef struct wav_header_ {
	unsigned int riff_id;
	unsigned int riff_siz;
	unsigned int riff_fmt;
	unsigned int fmt_id;
	unsigned int fmt_siz;
	unsigned short format_type;
	unsigned short channels;
	unsigned int sample_rate;
	unsigned int byte_rate;
	unsigned short block_align;
	unsigned short sample_bits;
} wav_hdr;

static int audio_enable = 0;
static int audio_paused = 0;
static int post_count = 0;
static char play_list[MAX_PATH_LEN];
static pthread_mutex_t *mixer_lock = NULL;
static pthread_mutex_t *pause_lock = NULL;
static pthread_t *audio_thread_id = NULL;
static int gCurAudioVolum = 0;

static int audio_device_open(void)
{
	int fd = -1;

	fd = open(AUDIO_DEV, O_WRONLY, 0);
	if (fd < 0) {
		fprintf(stderr, "audio device open err: %d\n", errno);
		return -1;
	}

	return fd;
}

static int set_audio_device_param(int fd, unsigned int sample_rate, unsigned int sample_bits, unsigned int channels)
{
	int arg;

	if (fd < 0) {
		return -1;
	}
#ifndef ZEM510
	if (ioctl(fd, SNDCTL_DSP_RESET, NULL) < 0) {
		return -1;
	}
	
	if (ioctl(fd, SNDCTL_DSP_SYNC, NULL) < 0) {
		return -1;
	}
#endif
	arg = (channels == STEREO ? 1 : 0);
	if (ioctl(fd, SNDCTL_DSP_STEREO, &arg) < 0) {
		return -1;
	}

	arg = sample_bits;
	if (ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &arg) < 0) {
		return -1;
	}

	arg = sample_rate;
	if (ioctl(fd, SNDCTL_DSP_SPEED, &arg) < 0) {
		return -1;
	}

	return 0;
}

static int audio_device_play(int fd, unsigned char *buf, int nbytes)
{
	size_t count = 0;
	int written = -1;

	while (count < nbytes) {
		do {
			written = write(fd, buf + count, nbytes - count);
		} while (written < 0 && (EINTR == errno || errno == EAGAIN));

		if (written < 0) {
			fprintf(stderr, "[WAV]: audio play err : %d\n", errno);
			return -1;
		}

		count += written;
	}

	return 0;

}

static void audio_device_close(int fd)
{
	if (fd > 0) {
		close(fd);
	}
}

static unsigned char* findchunk(unsigned char* pstart, unsigned char* fourcc, size_t n)
{	unsigned char *pend;
	int	k, test ;

	pend = pstart + n ;

	while (pstart < pend) {	
		if (*pstart == *fourcc) {	
			test = 1 ;
			for (k = 1 ; fourcc [k] != 0 ; k++)
				test = (test ? (pstart [k] == fourcc [k] ) : 0) ;
			
			if (test) {
				return pstart;
			}
		}
		pstart++;
	}

	return  NULL ;
}

static void volume_handle(char *buf, int size, int databits, int volume)
{
        int i;
        int count = 0;
        long data = 0;

        if (buf == NULL) {
                return;
        }

        if(databits == 16) {
                count = size / 2;
                for(i = 0; i < count; i++) {
                        data = ((short*)(buf))[i];
                        data = data * volume / 100;
                        if (data > 0x7fff) {
                                data = 0x7fff;
                        } else if(data < -0x7fff) {
                                data = -0x7fff;
                        }
                        ((short*)(buf))[i] = data;
                }
        } else {
                count = size;
                for(i = 0; i < count; i++) {
                        data = (unsigned char)buf[i];
                        data = (data - 128) * volume / 100 + 128;
                        buf[i] = data;
                }
        }
}

int play_wav_file(int fd, const char *path)
{
	int n;
	int id = -1;
	int size = 0;
	wav_hdr hdr;
	unsigned char *pos;
	unsigned char buffer[AUDIO_BUFFER_SIZE];

	if (fd < 0 || path == NULL) {
		return -1;
	}

	id = open(path, O_RDONLY);
	if (id < 0) {
		fprintf(stderr, "open wav file %s failed:%d\n", path, errno);
		return -1;
	}

	if (read(id, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		fprintf(stderr, "read wav hdr failed\n");
		close(id);
		return -1;
	}

	if (hdr.riff_id != ID_RIFF || hdr.riff_fmt != ID_WAVE || hdr.fmt_id != ID_FMT) {
		fprintf(stderr, "wav hdr info err\n");
		close(id);
		return -1;
	}

	if (hdr.format_type != FORMAT_PCM) {
		fprintf(stderr, "wav fmt err\n");
		close(id);
		return -1;
	}

	if (set_audio_device_param(fd, hdr.sample_rate, hdr.sample_bits, hdr.channels) < 0) {
		fprintf(stderr, "set audio device param err\n");
		close(id);
		return -1;
	}
	
	memset(buffer, 0x00, sizeof(buffer));
	if (read(id, buffer, MAX_WAVHDR_SIZE) < 0) {
		fprintf(stderr, "read wav data hdr err\n");
		close(id);
		return -1;
	}

	pos = findchunk(buffer, (unsigned char *)"data", MAX_WAVHDR_SIZE);
	if (pos == NULL) {
		fprintf(stderr, "find chunk err\n");
		close(id);
		return -1;
	}
	pos += 4;

	memcpy(&size, pos, sizeof(size));
	pos += sizeof(size);

	lseek(id, pos - buffer + sizeof(hdr), SEEK_SET);
	while (size > 0) {
		if (audio_paused == 1) {
			audio_paused = 0;
			break;
		}

		do {
			n = read(id, buffer, sizeof(buffer));
		} while (n < 0 && (errno == EINTR || errno == EAGAIN));

		if (n <= 0) {
			break;
		}

		if (n > size) {
			volume_handle((char*)buffer, size, hdr.sample_bits, gCurAudioVolum);
			audio_device_play(fd, buffer, size);
			break;
		} else {
			volume_handle((char*)buffer, n, hdr.sample_bits, gCurAudioVolum);
			audio_device_play(fd, buffer, n);
		}
		size -= n;
	}

	close(id);

	return 0;
}

void *audio_thread(void *arg)
{
	int fd = -1;

	audio_enable = 1;
	while (audio_enable) {
		if (post_count == 0) {
			msleep(10);
			continue;
		}

		mutexP(pause_lock);
		audio_paused = 0;
		mutexV(pause_lock);

		mutexP(mixer_lock);
		post_count--;

		if (fd < 0) {
			fd = audio_device_open();
		}
		play_wav_file(fd, play_list);
		mutexV(mixer_lock);
	}
	
	audio_device_close(fd);

	mutex_destroy(mixer_lock);

	mutex_destroy(pause_lock);

	thread_exit();

	return NULL;
}

int audio_thread_init(void)
{
	if (audio_thread_id != NULL) {
		return -1;
	}

	mixer_lock = mutex_init();
	if (mixer_lock == NULL) {
		return -1;
	}

	pause_lock = mutex_init();
	if (pause_lock == NULL) {
		return -1;
	}

	audio_thread_id = thread_create(audio_thread, NULL);
	if (audio_thread_id == NULL) {
		mutex_destroy(mixer_lock);
		mutex_destroy(pause_lock);
		return -1;
	}

	return 0;
}

void audio_thread_exit(void)
{
	audio_enable = 0;
}

int wav_play(const char *path, int volum)
{
	if (audio_thread_id == NULL) {
		return -1;
	}

	if (strlen(path) > MAX_PATH_LEN || pause_lock == NULL || mixer_lock == NULL) {
		return -1;
	}

	mutexP(pause_lock);
	audio_paused = 1;
	mutexV(pause_lock);

	mutexP(mixer_lock);
	memset(play_list, 0x00, sizeof(play_list));
	memcpy(play_list, path, strlen(path));
	gCurAudioVolum = volum;
	if (post_count == 0) {
		post_count++;
	}
	mutexV(mixer_lock);

	return 0;
}


int PlayWavFileAsync(int argc, char *path)
{
	int volum = 0;

#ifdef _TTS_
	//加入TTS 播放语音功能
	TTS_Wait();
	TTS_PlayWav(path);
	//调试打印播放语音的语音文件名称
	return 0;
#endif

	if(argc >= 0)
	{
		volum = argc;
	}
	else
	{
		volum = gOptions.AudioVol;
	}

	return wav_play(path, volum);
}

