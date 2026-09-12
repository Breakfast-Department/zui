#include "zui/image.h"
#include <zui/zui.h>
#include <stdio.h>
#include <time.h>

#define WORK_MINUTES 2
#define BREAK_MINUTES 1

typedef enum {
  STATE_IDLE,
  STATE_WORK,
  STATE_BREAK,
  STATE_PAUSED,
} TimerState;

typedef struct {
  ZuiWindow *window;
  ZuiWidget *main_panel;
  ZuiWidget *progress;
  ZuiWidget *status_label;

  TimerState state;
  TimerState prev_state;
  int remaining_secs;
  int total_secs;
  time_t last_tick;
  int completed_sessions;

  int last_width;
  int last_height;

  ZuiAudio *notify_sound;
} PomodoroApp;

static PomodoroApp app = {0};

static void update_ui(void);

static const char *get_status_text(void)
{
  switch (app.state) {
    case STATE_IDLE:   return "Click to start";
    case STATE_WORK:   return "Focus";
    case STATE_BREAK:  return "Break";
    case STATE_PAUSED: return "Paused";
    default:           return "";
  }
}

static void update_time_display(void)
{
  int mins = app.remaining_secs / 60;
  int secs = app.remaining_secs % 60;

  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
  zui_circularprogress_set_text(ZUI_CIRCULAR_PROGRESS(app.progress), buf);
}

static void update_progress(void)
{
  float val = 1.0f - ((float)app.remaining_secs / (float)app.total_secs);
  zui_circularprogress_set_value(ZUI_CIRCULAR_PROGRESS(app.progress), val);
}

static void update_ui(void)
{
  update_time_display();
  update_progress();
  zui_label_set_text(ZUI_LABEL(app.status_label), get_status_text());

  ZuiColor work_color = ZUI_COLOR_HEX(0x3B38A0);
  ZuiColor break_color = ZUI_COLOR_HEX(0xF96E2A);
  ZuiColor idle_color = ZUI_COLOR_HEX(0x3498db);

  ZuiColor fill_color;
  switch (app.state) {
    case STATE_WORK:   fill_color = work_color; break;
    case STATE_BREAK:  fill_color = break_color; break;
    case STATE_PAUSED:
      fill_color = (app.prev_state == STATE_WORK) ? work_color : break_color;
      break;
    default:           fill_color = idle_color; break;
  }
  zui_circularprogress_set_colors(ZUI_CIRCULAR_PROGRESS(app.progress), ZUI_COLOR_HEX(0x2a2a3a), fill_color);
}

static void start_work(void)
{
  app.state = STATE_WORK;
  app.remaining_secs = WORK_MINUTES * 60;
  app.total_secs = WORK_MINUTES * 60;
  app.last_tick = time(NULL);
  update_ui();
}

static void start_break(void)
{
  app.state = STATE_BREAK;
  app.remaining_secs = BREAK_MINUTES * 60;
  app.total_secs = BREAK_MINUTES * 60;
  app.last_tick = time(NULL);
  update_ui();
}

static void reset_timer(void)
{
  app.state = STATE_IDLE;
  app.remaining_secs = WORK_MINUTES * 60;
  app.total_secs = WORK_MINUTES * 60;
  update_ui();
}

static void toggle_timer(void)
{
  switch (app.state) {
    case STATE_IDLE:
      start_work();
      break;
    case STATE_WORK:
    case STATE_BREAK:
      app.prev_state = app.state;
      app.state = STATE_PAUSED;
      update_ui();
      break;
    case STATE_PAUSED:
      app.state = app.prev_state;
      app.last_tick = time(NULL);
      update_ui();
      break;
  }
}

static void on_progress_click(ZuiWidget *widget, uint32_t button, void *data)
{
  (void)widget;
  (void)data;

  if (button == BTN_LEFT) {
    toggle_timer();
  } else if (button == BTN_RIGHT) {
    reset_timer();
  }
}

static void timer_tick(void)
{
  if (app.state != STATE_WORK && app.state != STATE_BREAK) return;

  time_t now = time(NULL);
  int elapsed = (int)(now - app.last_tick);

  if (elapsed > 0) {
    app.remaining_secs -= elapsed;
    app.last_tick = now;

    if (app.remaining_secs <= 0) {
      if (app.notify_sound) {
        zui_audio_play(app.notify_sound);
      }
      if (app.state == STATE_WORK) {
        app.completed_sessions++;
        start_break();
      } else {
        start_work();
      }
    } else {
      update_time_display();
      update_progress();
    }
  }
}

static void update_responsive_layout(int width, int height)
{
  int min_dim = (width < height) ? width : height;

  float progress_size = (float)min_dim * 0.65f;
  if (progress_size < 120) progress_size = 120;
  if (progress_size > 280) progress_size = 280;

  zui_circularprogress_set_size(ZUI_CIRCULAR_PROGRESS(app.progress), progress_size);

  float thickness = progress_size * 0.03f;
  if (thickness < 4) thickness = 4;
  if (thickness > 10) thickness = 10;
  zui_circularprogress_set_thickness(ZUI_CIRCULAR_PROGRESS(app.progress), thickness);

  float text_size = progress_size * 0.18f;
  if (text_size < 24) text_size = 24;
  if (text_size > 48) text_size = 48;
  zui_circularprogress_set_text_size(ZUI_CIRCULAR_PROGRESS(app.progress), text_size);

  float label_size = progress_size * 0.12f;
  if (label_size < 14) label_size = 14;
  if (label_size > 22) label_size = 22;
  zui_label_set_size(ZUI_LABEL(app.status_label), label_size);

  float spacing = progress_size * 0.1f;
  if (spacing < 12) spacing = 12;
  if (spacing > 28) spacing = 28;
  zui_panel_set_spacing(ZUI_PANEL(app.main_panel), spacing);
}

static ZuiWidget *create_ui(void)
{
  app.main_panel = zui_panel_new();
  zui_panel_set_layout(ZUI_PANEL(app.main_panel), ZUI_LAYOUT_VERTICAL);
  zui_panel_set_alignment(ZUI_PANEL(app.main_panel), ZUI_ALIGN_CENTER, ZUI_ALIGN_CENTER);
  zui_panel_set_fill(ZUI_PANEL(app.main_panel), true, true);
  zui_panel_set_spacing(ZUI_PANEL(app.main_panel), 24);
  zui_panel_set_background(ZUI_PANEL(app.main_panel), ZUI_COLOR(0, 0, 0, 0));

  app.progress = zui_circularprogress_new();
  zui_circularprogress_set_size(ZUI_CIRCULAR_PROGRESS(app.progress), 160);
  zui_circularprogress_set_thickness(ZUI_CIRCULAR_PROGRESS(app.progress), 5);
  zui_circularprogress_set_colors(ZUI_CIRCULAR_PROGRESS(app.progress),
    ZUI_COLOR_HEX(0x2a2a3a), ZUI_COLOR_HEX(0x3498db));
  zui_circularprogress_set_value(ZUI_CIRCULAR_PROGRESS(app.progress), 0);
  zui_circularprogress_set_text(ZUI_CIRCULAR_PROGRESS(app.progress), "25:00");
  zui_circularprogress_set_text_color(ZUI_CIRCULAR_PROGRESS(app.progress), ZUI_COLOR_HEX(0xffffff));
  zui_circularprogress_set_text_size(ZUI_CIRCULAR_PROGRESS(app.progress), 32);
  zui_circularprogress_on_click(ZUI_CIRCULAR_PROGRESS(app.progress), on_progress_click, NULL);
  zui_set_cursor(app.progress, ZUI_CURSOR_POINTER);
  zui_panel_add_child(ZUI_PANEL(app.main_panel), app.progress);

  app.status_label = zui_label_new("Click to start");
  zui_label_set_size(ZUI_LABEL(app.status_label), 16);
  zui_label_set_color(ZUI_LABEL(app.status_label), ZUI_COLOR_HEX(0x888899));
  zui_panel_add_child(ZUI_PANEL(app.main_panel), app.status_label);

  return app.main_panel;
}

int main(void)
{
  if (!zui_init()) {
    fprintf(stderr, "Failed to initialize ZUI\n");
    return 1;
  }

  if (!zui_audio_init()) {
    fprintf(stderr, "Warning: Failed to initialize audio\n");
  }

  app.notify_sound = zui_audio_load("res:/audio/notify.mp3");
  if (!app.notify_sound) {
    app.notify_sound = zui_audio_load("assets/audio/notify.mp3");
    if (!app.notify_sound) {
      fprintf(stderr, "Warning: Failed to load notification sound\n");
    }
  }

  ZuiWindow *window = zui_window_create(320, 400, "Pomodoro");
  if (!window) {
    fprintf(stderr, "Failed to create window\n");
    return 1;
  }

  app.window = window;

  zui_window_set_min_size(window, 180, 230);
  zui_window_set_max_size(window, 400, 500);
  zui_window_set_corner_radius(window, 16.0f);
  zui_window_set_background_color(window, ZUI_COLOR_HEX(0x1a1a2e));

  ZuiWindowDecoration *decor = zui_default_window_decoration(window);
  zui_window_decoration_set_title(decor, "Pomodoro");
  zui_window_decoration_set_logo(decor, "res:/images/app_icon.webp");

  ZuiWidget *content = zui_window_content(window);
  ZuiWidget *ui = create_ui();
  zui_widget_add_child(content, ui);

  app.state = STATE_IDLE;
  app.remaining_secs = WORK_MINUTES * 60;
  app.total_secs = WORK_MINUTES * 60;
  app.completed_sessions = 0;

  update_ui();
  update_responsive_layout(320, 400);

  zui_window_show(window);

  while (zui_window_running(window)) {
    int w, h;
    zui_window_get_size(window, &w, &h);

    if (w != app.last_width || h != app.last_height) {
      update_responsive_layout(w, h);
      app.last_width = w;
      app.last_height = h;
    }

    timer_tick();
    zui_window_render(window);
    zui_poll_events();
  }

  zui_window_destroy(window);

  if (app.notify_sound) {
    zui_audio_destroy(app.notify_sound);
  }
  zui_audio_shutdown();
  zui_shutdown();

  return 0;
}
