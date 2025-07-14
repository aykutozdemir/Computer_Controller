/**
 * @file ProgressBar.h
 * @brief Progress bar widget class for the SimpleUI library.
 */

#pragma once

#include "Widget.h"

class ProgressBar : public Widget {
private:
    float _progress{0.0f};
    uint16_t _bgColor{0x39E7};
    uint16_t _fillColor{0x07E0};
    float _prevProgress{-1.0f};
public:
    /// Construct a ProgressBar widget.
    ProgressBar(int16_t x, int16_t y, int16_t w, int16_t h);

    /// Set progress value in the range [0.0, 1.0].
    void setProgress(float p);

    /// Draw the progress bar.
    void draw() override;
}; 