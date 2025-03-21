/**
 * Copyright (c) 2025 Ayan Shafqat <ayan.x.shafqat@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "raylib.h"

#include "haar.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define NUM_BINS (10)
#define BLOCK_SIZE (1 << NUM_BINS)
static float spectrumData[NUM_BINS] = {0};
static float spectrumSlow[NUM_BINS] = {0};

void DrawSpectrum(const float *spectrumData, size_t numBins);
void ComputeSpectrum(const float *data, size_t len);
void AudioProcessor(void *buffer, unsigned int frames);

float ScaleDbToPlot(float dbValue);

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Wavelet Spectrum Analyzer");

  InitAudioDevice();

  Music music = {0};
  float timePlayed = 0.0f;
  bool pause = false;
  bool fileLoaded = false;

  while (!WindowShouldClose()) {

    if (IsFileDropped()) {
      FilePathList droppedFiles = LoadDroppedFiles();

      if (fileLoaded) {
        StopMusicStream(music);
        UnloadMusicStream(music);
        fileLoaded = false;
      }

      music = LoadMusicStream(droppedFiles.paths[0]);
      AttachAudioStreamProcessor(music.stream, AudioProcessor);
      PlayMusicStream(music);
      fileLoaded = true;

      UnloadDroppedFiles(droppedFiles);
    }

    if (fileLoaded) {
      if (IsKeyPressed(KEY_R)) {
        StopMusicStream(music);
        PlayMusicStream(music);
      }

      if (IsKeyPressed(KEY_SPACE)) {
        pause = !pause;
        if (pause)
          PauseMusicStream(music);
        else
          ResumeMusicStream(music);
      }

      if (IsKeyPressed(KEY_RIGHT)) {
        float newTime = GetMusicTimePlayed(music) + 10.0f;
        float duration = GetMusicTimeLength(music);
        newTime = fminf(newTime, duration);
        SeekMusicStream(music, newTime);
      }

      if (IsKeyPressed(KEY_LEFT)) {
        float newTime = GetMusicTimePlayed(music) - 10.0f;
        newTime = fmaxf(newTime, 0.0F);
        SeekMusicStream(music, newTime);
      }

      // Get normalized time played for current music stream
      timePlayed = GetMusicTimePlayed(music) / GetMusicTimeLength(music);
      // Make sure time played is no longer than music
      timePlayed = fminf(timePlayed, 1.0F);

      // Update music buffer with new stream data
      UpdateMusicStream(music);
    }

    BeginDrawing();

    ClearBackground(BLACK);

    if (fileLoaded) {
      DrawSpectrum(spectrumData, NUM_BINS);

      DrawRectangle(200, 380, 400, 12, LIGHTGRAY);
      DrawRectangle(200, 380, (int)(timePlayed * 400.0f), 12, MAROON);
      DrawRectangleLines(200, 380, 400, 12, GRAY);
    } else {
      DrawText("DRAG AND DROP MUSIC", 255, 150, 20, LIGHTGRAY);
    }

    EndDrawing();
  }

  if (fileLoaded) {
    UnloadMusicStream(music);
  }

  CloseAudioDevice();
  CloseWindow();

  return 0;
}

void DrawSpectrum(const float *spectrumData, size_t numBins) {
  // Draw spectrum bars
  for (size_t i = 0; i < numBins; i++) {
    float barHeight = ScaleDbToPlot(spectrumData[i]);
    float barWidth = GetScreenWidth() / (float)numBins;

    DrawRectangle(i * barWidth, GetScreenHeight() - barHeight, barWidth - 2, barHeight,
                  GREEN);

    // Thin horizontal line
    float slowHeight = ScaleDbToPlot(spectrumSlow[i]);
    float x = i * barWidth;
    float y = GetScreenHeight() - slowHeight;

    DrawLine(x, y, x + barWidth - 2, y, RAYWHITE);
  }
}

float LowPass1(float in, float b0, float state) { return (b0 * (in - state)) + state; }

void ComputeSpectrum(const float *data, size_t len) {
  if (len != BLOCK_SIZE) {
    return;
  }

  static float haarOut[BLOCK_SIZE];
  haar_spectrum_l1(haarOut, data, NUM_BINS);

  for (size_t bin; bin < NUM_BINS; ++bin) {
    float newValue = haarOut[bin];
    float state = spectrumData[bin];
    float stateSlow = spectrumSlow[bin];

    float out = LowPass1(newValue, 0.2F, state);
    float outSlow = LowPass1(newValue, 0.01F, stateSlow);

    spectrumData[bin] = out;
    spectrumSlow[bin] = outSlow;
  }
}

void AudioProcessor(void *buffer, unsigned int frames) {
  float *sampleData = buffer;
  static float monoData[BLOCK_SIZE];

  size_t count = BLOCK_SIZE < frames ? BLOCK_SIZE : frames;

  for (size_t frame = 0; frame < frames; ++frame) {
    size_t left = 2 * frame + 0;
    size_t right = 2 * frame + 1;
    float y = 0.5F * (sampleData[left] + sampleData[right]);
    monoData[frame] = y;
  }

  for (size_t frame = count; frame < BLOCK_SIZE; ++frame) {
    monoData[frame] = 0.0F;
  }

  ComputeSpectrum(monoData, BLOCK_SIZE);
}

#define DB_MIN -80.0F
#define DB_MAX 0.0F
#define SCREEN_HEIGHT 400

static float ClampFloat(float value, float minVal, float maxVal) {
  return fminf(maxVal, fmaxf(minVal, value));
}

float ScaleDbToPlot(float dbValue) {
  // Clamp dB value between DB_MIN and DB_MAX
  dbValue = ClampFloat(dbValue, DB_MIN, DB_MAX);

  // Normalize dB value between 0 and 1
  float normalized = (dbValue - DB_MIN) / (DB_MAX - DB_MIN);

  // Scale to screen height (invert so louder values are higher)
  return (normalized)*SCREEN_HEIGHT;
}
