#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>

#define MAX_KNOTS 256
#define KNOT_RADIUS 15
#define CONTROL_POINT_RADIUS 10
#define CONTROL_POINTS_PER_KNOT 2

typedef struct {
  Vector2 pos;
  bool active;
} ControlPoint;

typedef struct {
  Vector2 pos;
  ControlPoint control_points[CONTROL_POINTS_PER_KNOT];
} Knot;

typedef struct {
  Knot knots[MAX_KNOTS];
  int knots_count;
  bool is_pen_active;

  bool is_selected_knot;
  bool is_selected_control_point;
  int selected_knot_index;
  int selected_control_point_index;

  bool is_adding_knot;

  float u;
} State;

static State state;

void StateInit() {
  state.knots_count = 0;
  state.u = 0;
  state.is_pen_active = true;
  state.is_adding_knot = false;
  state.is_selected_knot = false;
  state.is_selected_control_point = false;
}

Vector2 VectorOtherSide(Vector2 v, Vector2 p) {
  Vector2 d = Vector2Subtract(v, p);
  return Vector2Subtract(v, Vector2Scale(d, 2));
}

Vector2 SampleBezierCurve(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3,
                          float t) {
  Vector2 a = Vector2Lerp(p0, p1, t);
  Vector2 b = Vector2Lerp(p1, p2, t);
  Vector2 c = Vector2Lerp(p2, p3, t);
  Vector2 d = Vector2Lerp(a, b, t);
  Vector2 e = Vector2Lerp(b, c, t);
  return Vector2Lerp(d, e, t);
}

void DrawBezierCurveBetweenKnots(int begin_knot_index, int end_knot_index,
                                 float step) {
  Vector2 points[1000];
  int points_count = 0;
  Vector2 p0 = state.knots[begin_knot_index].pos;
  Vector2 p1 = state.knots[begin_knot_index].control_points[1].pos;
  Vector2 p2 = state.knots[end_knot_index].control_points[0].pos;
  Vector2 p3 = state.knots[end_knot_index].pos;

  Vector2 c1 = Vector2Add(Vector2Scale(p0, -3), Vector2Scale(p1, 3));
  Vector2 c2 =
      Vector2Add(Vector2Subtract(Vector2Scale(p0, 3), Vector2Scale(p1, 6)),
                 Vector2Scale(p2, 3));
  Vector2 c3 = Vector2Add(
      Vector2Subtract(Vector2Add(Vector2Scale(p0, -1), Vector2Scale(p1, 3)),
                      Vector2Scale(p2, 3)),
      p3);

  for (float t = 0; t <= 1; t += step) {
    // points[points_count++] = SampleBezierCurve(p0, p1, p2, p3, t);
    points[points_count++] =
        Vector2Add(p0, Vector2Add(Vector2Scale(c1, t),
                                  Vector2Add(Vector2Scale(c2, t * t),
                                             Vector2Scale(c3, t * t * t))));
  }
  DrawLineStrip(points, points_count, RED);
}

int main(void) {
  InitWindow(1600, 900, "Bézier spline");
  SetTargetFPS(60);

  StateInit();

  while (!WindowShouldClose()) {
    state.u += GetFrameTime() / 2;
    if (state.u >= (state.knots_count) - 1) {
      state.u = 0;
    }

    if (IsKeyPressed(KEY_C)) {
      state.knots_count = 0;
      state.is_pen_active = true;
    }
    if (IsKeyPressed(KEY_P)) {
      state.is_pen_active = true;
    }
    if (IsKeyPressed(KEY_S)) {
      state.is_pen_active = false;
    }

    if (state.is_pen_active) {
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse_pos = GetMousePosition();
        state.knots[state.knots_count].pos = mouse_pos;
        state.knots[state.knots_count].control_points[0].active = true;
        state.knots[state.knots_count].control_points[1].active = true;
        state.knots[state.knots_count].control_points[0].pos = mouse_pos;
        state.knots[state.knots_count].control_points[1].pos = mouse_pos;
        state.knots_count += 1;
        state.is_adding_knot = true;
      };

      if (state.is_adding_knot) {
        Vector2 mouse_pos = GetMousePosition();
        state.knots[state.knots_count - 1].control_points[0].pos =
            VectorOtherSide(mouse_pos, state.knots[state.knots_count - 1].pos);
        state.knots[state.knots_count - 1].control_points[1].pos = mouse_pos;

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
          state.is_adding_knot = false;
          state.knots[state.knots_count - 1].control_points[1].active = false;
          if (state.knots_count > 1) {
            state.knots[state.knots_count - 1].control_points[0].active = true;
          } else {
            state.knots[state.knots_count - 1].control_points[0].active = false;
          }
          state.knots[state.knots_count - 2].control_points[1].active = true;
        }
      }
    }
    if (!state.is_pen_active) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        state.is_selected_control_point = false;
        state.is_selected_knot = false;
        Vector2 mouse_pos = GetMousePosition();
        for (int i = 0; i < state.knots_count; i++) {
          for (int j = 0; j < CONTROL_POINTS_PER_KNOT; j++) {
            if (CheckCollisionPointCircle(mouse_pos,
                                          state.knots[i].control_points[j].pos,
                                          KNOT_RADIUS)) {
              state.selected_knot_index = i;
              state.selected_control_point_index = j;
              state.is_selected_control_point = true;
              break;
            }
          }

          if (!state.is_selected_control_point &&
              CheckCollisionPointCircle(mouse_pos, state.knots[i].pos,
                                        KNOT_RADIUS)) {
            state.is_selected_knot = true;
            state.selected_knot_index = i;
          }
        }
      }

      if ((state.is_selected_knot || state.is_selected_control_point) &&
          IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        state.is_selected_knot = false;
        state.is_selected_control_point = false;
      }

      if (state.is_selected_knot) {
        Vector2 mouse_pos = GetMousePosition();
        Vector2 d = Vector2Subtract(mouse_pos,
                                    state.knots[state.selected_knot_index].pos);
        state.knots[state.selected_knot_index].pos = mouse_pos;
        state.knots[state.selected_knot_index].control_points[0].pos =
            Vector2Add(
                state.knots[state.selected_knot_index].control_points[0].pos,
                d);
        state.knots[state.selected_knot_index].control_points[1].pos =
            Vector2Add(
                state.knots[state.selected_knot_index].control_points[1].pos,
                d);
      }

      if (state.is_selected_control_point) {
        Vector2 mouse_pos = GetMousePosition();
        state.knots[state.selected_knot_index]
            .control_points[state.selected_control_point_index]
            .pos = mouse_pos;
      }
    }

    BeginDrawing();
    ClearBackground(GetColor(0x181818FF));

    if (state.knots_count > 1) {
      for (int i = 1; i < state.knots_count; i++) {
        DrawBezierCurveBetweenKnots(i - 1, i, 0.01);
      }
    }

    for (int i = 0; i < state.knots_count; i++) {
      for (int j = 0; j < CONTROL_POINTS_PER_KNOT; j++) {
        if (state.knots[i].control_points[j].active) {
          DrawLineEx(state.knots[i].pos, state.knots[i].control_points[j].pos,
                     2, BLUE);
        }
      }
    }

    for (int i = 0; i < state.knots_count; i++) {
      DrawCircleV(state.knots[i].pos, KNOT_RADIUS, RED);
      for (int j = 0; j < CONTROL_POINTS_PER_KNOT; j++) {
        if (state.knots[i].control_points[j].active) {
          DrawCircleV(state.knots[i].control_points[j].pos,
                      CONTROL_POINT_RADIUS, BLUE);
        }
      }
    }

    // Draw a circle that follows the curve
    // for (int i = 0; i < state.knots_count; i++) {
    //   if (state.u - i >= 0 && state.u - i < 1) {
    //     int begin_knot_index = i;
    //     int end_knot_index = i + 1;
    //     Vector2 p0 = state.knots[begin_knot_index].pos;
    //     Vector2 p1 = state.knots[begin_knot_index].control_points[1].pos;
    //     Vector2 p2 = state.knots[end_knot_index].control_points[0].pos;
    //     Vector2 p3 = state.knots[end_knot_index].pos;
    //
    //     DrawCircleV(SampleBezierCurve(p0, p1, p2, p3, state.u - i), 30, PURPLE);
    //   }
    // }

    EndDrawing();
  }
  CloseWindow();
  return 0;
}
