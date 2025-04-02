#include "sound.h"

#include "pd_api.h"

extern PlaydateAPI *playdate;

static float global_volume = 1;
static float inv_out_samplerate;

struct sound_source_t {
  int index;
  union {
    FilePlayer *file_player;
    // AudioSample *audio_sample;
    // SamplePlayer *sample_player;
  };
};

static sound_source_t sources[SOUND_MAX_SOURCES];
static uint32_t sources_len = 0;
static char *source_paths[SOUND_MAX_SOURCES] = {};
static AudioSample *audio_samples[SOUND_MAX_SOURCES] = {};
static SamplePlayer *sample_players[SOUND_MAX_SOURCES] = {};

sound_mark_t sound_mark(void) { return (sound_mark_t){.index = 0}; }

void sound_reset(sound_mark_t mark) {}

// Put all playing nodes in a halt state; usefull for e.g. a pause screen
void sound_halt(void) {}

// Resume playing all halted sounds
void sound_resume(void) {}

// Return the global volume for all sounds
float sound_global_volume(void) { return 0; }

// Set the global volume for all nodes
void sound_set_global_volume(float volume) {}

// Periodically called by the platform to mix playing nodes into output buffer
void sound_mix_stereo(float *dest_samples, uint32_t dest_len) {}

// Load a sound sorce from a QOA file. Calling this function multiple times with
// the same path will return the same, cached sound source,
sound_source_t *sound_source(char *path) {
  for (uint32_t i = 0; i < sources_len; i++) {
    if (str_equals(path, source_paths[i])) {
      return &sources[i];
    }
  }
  PlaydateAPI *pd = playdate;

  source_paths[sources_len] = strdup(path);

  char *tmp = strdup(path);
  strcpy(tmp + strlen(tmp) - 3, "pda");

  sound_source_t *source = &sources[sources_len];
  source->index = sources_len;

  // source->audio_sample = pd->sound->sample->load(source_paths[sources_len]);
  // source->sample_player = pd->sound->sampleplayer->newPlayer();
  // pd->sound->sampleplayer->setSample(source->sample_player,
  // source->audio_sample);
  // pd->sound->channel->addSource(pd->sound->getDefaultChannel(),
  //                           (SoundSource *)source->sample_player);

  source->file_player = pd->sound->fileplayer->newPlayer();
  pd->sound->fileplayer->loadIntoPlayer(source->file_player, tmp);

  free(tmp);
  sources_len++;
  return source;
}

// Return the duration of a sound source
float sound_source_duration(sound_source_t *source) { return 0; }

// Obtain a free node for the given source. This will "reserve" the source. It
// can not be re-used until it is disposed via sound_dispose(). The node will be
// in a paused state and must be explicitly unpaused. Returns an invalid node
// with id = 0 when no free node is available.
sound_t sound(sound_source_t *source) {
  sound_t s = {.index = source->index};
  return s;
}

// Play a sound source. The node used to play it will be automatically disposed
// once it has played through.
void sound_play(sound_source_t *source) {
  if (!source)
    return;
  PlaydateAPI *pd = playdate;
  pd->sound->fileplayer->stop(source->file_player);
  pd->sound->fileplayer->play(source->file_player, 1);
}

// Play a sound source with the given volume, pan and pitch. The node used to
// play it will be automatically disposed once it has played through.
void sound_play_ex(sound_source_t *source, float volume, float pan,
                   float pitch) {}

// Unpauses a paused node
void sound_unpause(sound_t sound) {}

// Pauses a node
void sound_pause(sound_t sound) {}

// Pauses a node and rewind it to the start
void sound_stop(sound_t sound) {}

// Dispose this node. The node is invalid afterwards, but will still play to the
// end if it's not paused.
void sound_dispose(sound_t sound) {}

// Return whether this node loops
bool sound_loop(sound_t sound) { return false; }

// Set whether to loop this node
void sound_set_loop(sound_t sound, bool loop) {
  PlaydateAPI *pd = playdate;
  sound_source_t *source = &sources[sound.index];
  if (!source)
    return;
  pd->sound->fileplayer->stop(source->file_player);
  pd->sound->fileplayer->play(source->file_player, 0);
}

// Return the duration in seconds of the underlying sound source. This does not
// take the node's current pitch into account
float sound_duration(sound_t sound) { return 0; }

// Return the current position of this node in seconds. This does not take the
// node's current pitch into account
float sound_time(sound_t sound) { return 0; }

// Set the current position of this node in seconds. This does not take the
// node's current pitch into account
void sound_set_time(sound_t sound, float time) {}

// Return the current volume of this node
float sound_volume(sound_t sound) { return 0; }

// Set the current volume of this node
void sound_set_volume(sound_t sound, float volume) {}

// Return the current pan of the node (-1 = left, 0 = center, 1 = right)
float sound_pan(sound_t sound) { return 0; }

// Return the current pan of a the node
void sound_set_pan(sound_t sound, float pan) {}

// Return the current pitch (playback speed) of this node. Default 1.
float sound_pitch(sound_t sound) { return 0; }

// Set the current pitch (playback speed) of this node
void sound_set_pitch(sound_t sound, float pitch) {}

// Called by the platform
void sound_init(int samplerate) {
  memset(sources, 0, sizeof(sound_source_t) * SOUND_MAX_SOURCES);
}

void sound_cleanup(void) {}

void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len)) {}