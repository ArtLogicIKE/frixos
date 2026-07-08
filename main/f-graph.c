/*
 * Generic graph widget. Generalised from the CGM glucose-graph contributed by
 * Benoit Poirier in PR #180 (features/graph-glucose) — the canvas rendering
 * approach and much of the plotting maths originate from that work; this module
 * extends it to sample and plot any numeric token.
 */
#include "f-graph.h"
#include <string.h>
#include <math.h>

/*
 * Lock abstraction: real FreeRTOS mutex on-device; no-op under the host test
 * harness (single-threaded) so f-graph.c can be compiled and exercised on a
 * dev machine (see tools/test_graph.c).
 */
#ifdef GRAPH_HOST_TEST
#define GRAPH_LOCK() ((void)0)
#define GRAPH_UNLOCK() ((void)0)
static void graph_lock_init(void) {}
#else
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static SemaphoreHandle_t graph_mtx;
static void graph_lock_init(void)
{
  if (!graph_mtx)
    graph_mtx = xSemaphoreCreateMutex();
}
#define GRAPH_LOCK()                              \
  do                                              \
  {                                               \
    if (graph_mtx)                                \
      xSemaphoreTake(graph_mtx, portMAX_DELAY);   \
  } while (0)
#define GRAPH_UNLOCK()       \
  do                         \
  {                          \
    if (graph_mtx)           \
      xSemaphoreGive(graph_mtx); \
  } while (0)
#endif

static graph_ring_t ring;

// Samples are stored as fixed-point deci-units (value x 10) so decimals (e.g.
// 26.3 °C) survive. int32 x10 covers roughly -214M .. 214M real units — ample
// for temperature, glucose, and high-magnitude tokens like power in watts
// (int16 x10 used to cap at 3276.7, silently clamping larger readings). The
// renderer divides by 10 for display.
static int32_t clamp_sample(float v)
{
  long long r = llroundf(v * 10.0f);
  if (r > INT32_MAX)
    r = INT32_MAX;
  if (r <= INT32_MIN) // keep INT32_MIN (GRAPH_SAMPLE_UNSET) reserved as the gap sentinel
    r = INT32_MIN + 1;
  return (int32_t)r;
}

// Push one sample into the ring (caller holds the lock).
static void ring_push(int32_t val)
{
  if (ring.cap == 0)
    return;
  ring.v[ring.head] = val;
  ring.head = (uint8_t)((ring.head + 1) % ring.cap);
  if (ring.count < ring.cap)
    ring.count++;
}

void graph_init(void)
{
  graph_lock_init();
  memset(&ring, 0, sizeof(ring));
}

void graph_configure(const screen_graph_cfg_t *cfg, bool widget_enabled)
{
  if (!cfg)
    return;

  uint16_t iv = cfg->interval_min ? cfg->interval_min : 1;
  uint8_t cap = cfg->points ? cfg->points : 2;
  if (cap > GRAPH_MAX_POINTS)
    cap = GRAPH_MAX_POINTS;
  bool want = widget_enabled && cfg->token[0] != '\0';

  GRAPH_LOCK();
  bool reshape = (strncmp(ring.token, cfg->token, GRAPH_TOKEN_LEN) != 0) ||
                 iv != ring.interval_min || cap != ring.cap;
  if (reshape)
  {
    memset(ring.v, 0, sizeof(ring.v));
    ring.count = 0;
    ring.head = 0;
    ring.last_sample = 0;
    strncpy(ring.token, cfg->token, GRAPH_TOKEN_LEN - 1);
    ring.token[GRAPH_TOKEN_LEN - 1] = '\0';
    ring.interval_min = iv;
    ring.cap = cap;
    ring.needs_backfill = want; // backfill is always on; GRAPH_FLAG_BACKFILL ignored
  }
  ring.active = want;
  GRAPH_UNLOCK();
}

void graph_sampler_tick(time_t now)
{
  GRAPH_LOCK();
  if (ring.active && ring.cap && ring.interval_min &&
      (ring.last_sample == 0 || (now - ring.last_sample) >= (time_t)ring.interval_min * 60))
  {
    float v;
    int32_t s = token_numeric_value(ring.token, &v) ? clamp_sample(v) : GRAPH_SAMPLE_UNSET;
    ring_push(s);
    ring.last_sample = now;
  }
  GRAPH_UNLOCK();
}

int graph_snapshot(int32_t *out_v, int max, time_t *last_sample, uint16_t *interval_min)
{
  GRAPH_LOCK();
  int n = ring.count;
  if (n > max)
    n = max;
  uint8_t cap = ring.cap ? ring.cap : 1;
  int oldest = (ring.head + cap - ring.count) % cap;
  for (int i = 0; i < n; i++)
    out_v[i] = ring.v[(oldest + i) % cap];
  if (last_sample)
    *last_sample = ring.last_sample;
  if (interval_min)
    *interval_min = ring.interval_min;
  GRAPH_UNLOCK();
  return n;
}

bool graph_is_active(void)
{
  GRAPH_LOCK();
  bool a = ring.active;
  GRAPH_UNLOCK();
  return a;
}

bool graph_take_backfill_request(char *token_out, size_t token_len,
                                 uint16_t *interval_min, uint8_t *points)
{
  bool req = false;
  GRAPH_LOCK();
  // needs_backfill is one-shot, set only when the ring was just (re)shaped, so
  // it is the correct gate; don't also require count==0 (the sampler may have
  // pushed a point first, and graph_backfill() replaces the ring regardless).
  if (ring.needs_backfill && ring.active)
  {
    req = true;
    ring.needs_backfill = false; // consume the request; caller will fill
    if (token_out && token_len)
    {
      strncpy(token_out, ring.token, token_len - 1);
      token_out[token_len - 1] = '\0';
    }
    if (interval_min)
      *interval_min = ring.interval_min;
    if (points)
      *points = ring.cap;
  }
  GRAPH_UNLOCK();
  return req;
}

void graph_request_backfill(void)
{
  GRAPH_LOCK();
  if (ring.active)
    ring.needs_backfill = true;
  GRAPH_UNLOCK();
}

// --- Shared CGM history -------------------------------------------------
static float cgm_v[GRAPH_MAX_POINTS];
static time_t cgm_t[GRAPH_MAX_POINTS];
static int cgm_n;

void cgm_history_begin(void)
{
  GRAPH_LOCK();
  cgm_n = 0;
  GRAPH_UNLOCK();
}

void cgm_history_add(float mgdl, time_t ts)
{
  if (mgdl <= 0 || ts <= 0)
    return;
  GRAPH_LOCK();
  for (int i = 0; i < cgm_n; i++)
    if (cgm_t[i] == ts) // dedup by timestamp
    {
      GRAPH_UNLOCK();
      return;
    }
  if (cgm_n < GRAPH_MAX_POINTS)
  {
    cgm_v[cgm_n] = mgdl;
    cgm_t[cgm_n] = ts;
    cgm_n++;
  }
  else
  {
    int oldest = 0; // full: evict the oldest if this sample is newer
    for (int i = 1; i < GRAPH_MAX_POINTS; i++)
      if (cgm_t[i] < cgm_t[oldest])
        oldest = i;
    if (ts > cgm_t[oldest])
    {
      cgm_v[oldest] = mgdl;
      cgm_t[oldest] = ts;
    }
  }
  GRAPH_UNLOCK();
}

int cgm_history_get(float *vals, time_t *times, int max)
{
  GRAPH_LOCK();
  int n = cgm_n < max ? cgm_n : max;
  for (int i = 0; i < n; i++)
  {
    vals[i] = cgm_v[i];
    times[i] = cgm_t[i];
  }
  GRAPH_UNLOCK();
  return n;
}

void graph_backfill(const float *vals, const time_t *times, int n, time_t now)
{
  GRAPH_LOCK();
  uint8_t cap = ring.cap;
  uint16_t iv = ring.interval_min;
  if (cap == 0 || iv == 0 || n <= 0 || !vals || !times)
  {
    GRAPH_UNLOCK();
    return;
  }

  // Bin the time-stamped series into the most recent `cap` interval slots
  // ending at `now`. Slot k (0=oldest .. cap-1=newest) covers the wall-time
  // window centred on now-(cap-1-k)*interval. Last value wins within a bin.
  const long bin = (long)iv * 60;
  bool filled[GRAPH_MAX_POINTS] = {0};
  for (int k = 0; k < cap; k++)
    ring.v[k] = GRAPH_SAMPLE_UNSET;

  for (int i = 0; i < n; i++)
  {
    long age = (long)(now - times[i]);
    if (age < 0)
      age = 0;
    int slot = (cap - 1) - (int)(age / bin);
    if (slot < 0 || slot >= cap)
      continue; // older than the window — discarded
    if (vals[i] == (float)GRAPH_VAL_UNSET)
      continue;
    ring.v[slot] = clamp_sample(vals[i]);
    filled[slot] = true;
  }

  // Determine count = span from oldest filled slot to newest (cap-1).
  int oldest = cap;
  for (int k = 0; k < cap; k++)
    if (filled[k]) { oldest = k; break; }
  if (oldest == cap)
  {
    GRAPH_UNLOCK();
    return; // nothing landed in the window
  }
  // Compact so the ring reads chronologically: head past the newest slot.
  ring.count = (uint8_t)(cap - oldest);
  // Shift filled region down to index 0.
  if (oldest > 0)
  {
    for (int k = 0; k < ring.count; k++)
      ring.v[k] = ring.v[oldest + k];
    for (int k = ring.count; k < cap; k++)
      ring.v[k] = GRAPH_SAMPLE_UNSET;
  }
  ring.head = (uint8_t)(ring.count % cap);
  ring.last_sample = now;
  GRAPH_UNLOCK();
}
