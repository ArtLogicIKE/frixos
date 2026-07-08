/*
 * Host-side unit test for the generic graph ring buffer (main/f-graph.c).
 *
 * Compiled as a single translation unit: this file defines the minimal
 * frixos.h surface that f-graph.h needs (GRAPH_* limits + screen_graph_cfg_t),
 * stubs token_numeric_value(), then includes f-graph.c directly.
 *
 * Build & run:
 *   gcc tools/test_graph.c -I main/include -lm -o /tmp/graphtest && /tmp/graphtest
 */
#define GRAPH_HOST_TEST
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/* --- Minimal frixos.h surface (must match main/include/frixos.h) --------- */
#define GRAPH_TOKEN_LEN 32
#define GRAPH_MAX_POINTS 100
#define GRAPH_MIN_W 60
#define GRAPH_MAX_W 90
#define GRAPH_MIN_H 28
#define GRAPH_MAX_H 60
#define GRAPH_VAL_UNSET ((int16_t)0x8000)
#define GRAPH_SAMPLE_UNSET ((int32_t)0x80000000)
#define GRAPH_FLAG_AUTOSCALE 0x01
#define GRAPH_FLAG_SHOW_AXIS 0x02
#define GRAPH_FLAG_BAND 0x04
#define GRAPH_FLAG_BACKFILL 0x08
#define GRAPH_FLAG_SHOW_VALUE 0x10
#define GRAPH_FLAG_BOOLEAN 0x20

typedef struct __attribute__((packed))
{
  char token[GRAPH_TOKEN_LEN];
  uint16_t interval_min;
  uint8_t points;
  uint8_t width;
  uint8_t height;
  uint8_t flags;
  int16_t band_low;
  int16_t band_high;
  int16_t y_min;
  int16_t y_max;
  uint8_t band_r, band_g, band_b;
  uint8_t warn_r, warn_g, warn_b;
  uint8_t axis_r, axis_g, axis_b;
  uint8_t reserved[1];
} screen_graph_cfg_t;

/* --- Accessor stub: controlled by the test ------------------------------ */
static int stub_has = 1;
static float stub_val = 0.0f;
bool token_numeric_value(const char *tok, float *out)
{
  (void)tok;
  if (!stub_has)
    return false;
  *out = stub_val;
  return true;
}

#include "../main/f-graph.c"

/* --- Tiny assert harness ------------------------------------------------- */
static int g_fail = 0;
#define CHECK(cond, msg)                                  \
  do                                                      \
  {                                                       \
    if (!(cond))                                          \
    {                                                     \
      printf("  FAIL: %s\n", msg);                        \
      g_fail++;                                           \
    }                                                     \
  } while (0)

static screen_graph_cfg_t mkcfg(const char *token, uint16_t iv, uint8_t pts, uint8_t flags)
{
  screen_graph_cfg_t c;
  memset(&c, 0, sizeof(c));
  strncpy(c.token, token, GRAPH_TOKEN_LEN - 1);
  c.interval_min = iv;
  c.points = pts;
  c.width = 90;
  c.height = 40;
  c.flags = flags;
  c.band_low = GRAPH_VAL_UNSET;
  c.band_high = GRAPH_VAL_UNSET;
  c.y_min = GRAPH_VAL_UNSET;
  c.y_max = GRAPH_VAL_UNSET;
  return c;
}

int main(void)
{
  graph_init();

  /* 1. Push + chronological order + wrap (cap=5). */
  {
    screen_graph_cfg_t c = mkcfg("[temp]", 1, 5, 0);
    graph_configure(&c, true);
    stub_has = 1;
    time_t t = 1000;
    /* Push 7 values 10,20,30,40,50,60,70 at 60s apart; cap=5 keeps last 5. */
    for (int i = 0; i < 7; i++)
    {
      stub_val = (float)((i + 1) * 10);
      graph_sampler_tick(t);
      t += 60;
    }
    int32_t out[GRAPH_MAX_POINTS];
    int n = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    CHECK(n == 5, "wrap: count should be 5");
    /* Stored as deci-units (value x10): last 5 of 30..70 -> 300..700. */
    int32_t expect[5] = {300, 400, 500, 600, 700};
    int ok = 1;
    for (int i = 0; i < 5; i++)
      if (out[i] != expect[i])
        ok = 0;
    CHECK(ok, "wrap: chronological order 30..70");
  }

  /* 2. Sub-interval ticks do not oversample. */
  {
    screen_graph_cfg_t c = mkcfg("[temp]", 5, 10, 0); /* 5-min interval */
    graph_configure(&c, true);
    stub_has = 1;
    stub_val = 42;
    time_t t = 2000;
    graph_sampler_tick(t);        /* first sample (last==0) */
    graph_sampler_tick(t + 60);   /* +1min: too soon */
    graph_sampler_tick(t + 120);  /* +2min: too soon */
    int32_t out[GRAPH_MAX_POINTS];
    int n = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    CHECK(n == 1, "interval: only one sample within 5 min");
    graph_sampler_tick(t + 300);  /* +5min: due */
    n = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    CHECK(n == 2, "interval: second sample at 5 min");
  }

  /* 3. Gap sentinel when token has no value. */
  {
    screen_graph_cfg_t c = mkcfg("[temp]", 1, 10, 0);
    graph_configure(&c, true);
    time_t t = 3000;
    stub_has = 1; stub_val = 11; graph_sampler_tick(t);
    stub_has = 0;               graph_sampler_tick(t + 60); /* no value -> gap */
    stub_has = 1; stub_val = 13; graph_sampler_tick(t + 120);
    int32_t out[GRAPH_MAX_POINTS];
    int n = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    CHECK(n == 3, "gap: three slots");
    CHECK(out[0] == 110 && out[1] == GRAPH_SAMPLE_UNSET && out[2] == 130, "gap: middle is UNSET");
  }

  /* 4. Reconfigure with same token+interval+points keeps history. */
  {
    screen_graph_cfg_t c = mkcfg("[temp]", 1, 10, 0);
    graph_configure(&c, true);
    stub_has = 1; stub_val = 5; graph_sampler_tick(5000);
    int32_t out[GRAPH_MAX_POINTS];
    int before = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    graph_configure(&c, true); /* identical cfg */
    int after = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    CHECK(before == after && after >= 1, "reconfigure: history preserved on identical cfg");
    screen_graph_cfg_t c2 = mkcfg("[hum]", 1, 10, 0); /* token change */
    graph_configure(&c2, true);
    int reset = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    CHECK(reset == 0, "reconfigure: ring reset on token change");
  }

  /* 5. Backfill resampling into bins. */
  {
    screen_graph_cfg_t c = mkcfg("[temp]", 10, 6, GRAPH_FLAG_BACKFILL); /* 10-min bins, 6 pts */
    graph_configure(&c, true);
    time_t now = 100000;
    /* Series: newest at now, then now-10m, now-20m ... */
    float vals[4] = {25, 24, 23, 22};
    time_t times[4] = {now, now - 600, now - 1200, now - 1800};
    graph_backfill(vals, times, 4, now);
    int32_t out[GRAPH_MAX_POINTS];
    int n = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    CHECK(n == 4, "backfill: 4 points landed");
    /* chronological oldest->newest = 22,23,24,25 (deci-units x10) */
    CHECK(out[n - 1] == 250 && out[0] == 220, "backfill: newest=25, oldest=22");
  }

  /* 6. High-magnitude token (e.g. [HA:sensor.ground_power] in watts) is stored
   *    without int16 saturation. Pre-fix, value x10 overflowed int16 and clamped
   *    to 32767 -> 3276.7 on display; int32 deci-units represent it exactly. */
  {
    screen_graph_cfg_t c = mkcfg("[HA:sensor.ground_power]", 1, 5, 0);
    graph_configure(&c, true);
    stub_has = 1; stub_val = 4357.2f; graph_sampler_tick(200000);
    int32_t out[GRAPH_MAX_POINTS];
    int n = graph_snapshot(out, GRAPH_MAX_POINTS, NULL, NULL);
    CHECK(n == 1, "power: one sample");
    CHECK(out[0] == 43572, "power: 4357.2 stored as 43572 (not clamped to 32767)");
  }

  if (g_fail == 0)
    printf("test_graph: ALL PASS\n");
  else
    printf("test_graph: %d FAILURE(S)\n", g_fail);
  return g_fail ? 1 : 0;
}
