#ifndef F_GRAPH_H
#define F_GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#ifndef GRAPH_HOST_TEST
#include "frixos.h" // pulls GRAPH_* limits + screen_graph_cfg_t (host test supplies its own)
#endif

/*
 * Generic single-instance time-series graph data model.
 *
 * One ring buffer holds up to GRAPH_MAX_POINTS evenly-spaced samples of one
 * selected token. Most tokens (weather, HA, stock) only expose a "latest
 * value", so the firmware samples them itself on a fixed interval; CGM (which
 * carries its own history) and HA (which has a /history API) can additionally
 * be backfilled. Points are value-only (no per-point timestamp) — they are
 * evenly spaced by interval_min, anchored at last_sample.
 *
 * Concurrency: the ring is written by the integration task (sampler/backfill)
 * and read by the display task (renderer). All access goes through the
 * functions below, which take an internal mutex. The renderer snapshots under
 * the lock and then draws, so it never holds the LVGL lock and the ring lock
 * at the same time.
 */
typedef struct
{
  int16_t v[GRAPH_MAX_POINTS]; // samples; GRAPH_VAL_UNSET = gap / no sample
  char token[GRAPH_TOKEN_LEN]; // token currently sampled, e.g. "[temp]"
  uint8_t count;               // valid samples (0..cap)
  uint8_t cap;                 // active capacity = cfg.points
  uint8_t head;                // ring write index (next slot)
  uint16_t interval_min;       // sample interval, minutes
  time_t last_sample;          // wall time of most recent push (0 = none yet)
  bool active;                 // graph enabled and token set
  bool needs_backfill;         // set on (re)configure when backfill requested
} graph_ring_t;

// Resolve a token string to its current numeric value (implemented in
// f-display.c, which owns the token registry + data globals). Booleans like
// "on"/"off"/"true"/"false" map to 1/0. Returns false if unknown/non-numeric.
bool token_numeric_value(const char *token, float *out);

void graph_init(void);                                       // create mutex (once, at boot)
void graph_sampler_tick(time_t now);                         // call from integration task (~60s)
void graph_configure(const screen_graph_cfg_t *cfg, bool widget_enabled); // on layout apply / profile switch

// Snapshot up to max chronological samples (oldest first) into out_v. Returns
// the number copied; optionally returns the anchor time and interval.
int graph_snapshot(int16_t *out_v, int max, time_t *last_sample, uint16_t *interval_min);

bool graph_is_active(void);

// --- Backfill helpers (Phase 5) -----------------------------------------
// True if the active graph wants backfilling and its ring is still empty;
// consumes the request (one-shot) and returns the token + window (interval and
// point count) so the caller can size its history query.
bool graph_take_backfill_request(char *token_out, size_t token_len,
                                  uint16_t *interval_min, uint8_t *points);
// Replace ring contents from an externally-collected, time-stamped series,
// resampling into the active interval bins anchored at `now`. Points with
// value GRAPH_VAL_UNSET are treated as gaps. Safe to call from any task.
void graph_backfill(const float *vals, const time_t *times, int n, time_t now);

#endif /* F_GRAPH_H */
