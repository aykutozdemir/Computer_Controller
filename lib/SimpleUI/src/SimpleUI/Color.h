/**
 * @file Color.h
 * @brief RGB565 color utility class and predefined colors for the SimpleUI library.
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>

class Color {
private:
    uint16_t _value;
    
    // Helper method to clamp values
    static uint8_t clamp(uint8_t value) {
        return (value > 255) ? 255 : value;
    }
    
public:
    /// Construct color from 16-bit RGB565 value.
    Color(uint16_t rgb565 = 0x0000) : _value(rgb565) {}
    /// Construct color from 8-bit per channel RGB value.
    Color(uint8_t r, uint8_t g, uint8_t b) : _value(color565(r, g, b)) {}
    
    /// Convert 24-bit RGB to 16-bit RGB565.
    static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    
    /// Return underlying RGB565 value.
    uint16_t getValue() const { return _value; }
    /// Implicit conversion to RGB565.
    operator uint16_t() const { return _value; }
    
    /// Extract 5-bit red component.
    uint8_t getRed() const {
        return (_value >> 8) & 0xF8;
    }
    
    /// Extract 6-bit green component.
    uint8_t getGreen() const {
        return (_value >> 3) & 0xFC;
    }
    
    /// Extract 5-bit blue component.
    uint8_t getBlue() const {
        return (_value << 3) & 0xF8;
    }
    
    /// Return a lighter shade by the given amount (0-255).
    Color lighten(uint8_t amount) const {
        uint8_t r = clamp(getRed() + amount);
        uint8_t g = clamp(getGreen() + amount);
        uint8_t b = clamp(getBlue() + amount);
        return Color(r, g, b);
    }
    
    /// Return a darker shade.
    Color darken(uint8_t amount) const {
        uint8_t r = (getRed() > amount) ? getRed() - amount : 0;
        uint8_t g = (getGreen() > amount) ? getGreen() - amount : 0;
        uint8_t b = (getBlue() > amount) ? getBlue() - amount : 0;
        return Color(r, g, b);
    }
    
    /// Blend this color with another using ratio (0-1).
    Color blend(const Color& other, float ratio) const {
        uint8_t r = (uint8_t)(getRed() * (1.0f - ratio) + other.getRed() * ratio);
        uint8_t g = (uint8_t)(getGreen() * (1.0f - ratio) + other.getGreen() * ratio);
        uint8_t b = (uint8_t)(getBlue() * (1.0f - ratio) + other.getBlue() * ratio);
        return Color(r, g, b);
    }
    
    /// Return the inverted colour.
    Color invert() const {
        return Color(255 - getRed(), 255 - getGreen(), 255 - getBlue());
    }
    
    /// Compute simple brightness (average of R,G,B).
    uint8_t getBrightness() const {
        return (getRed() + getGreen() + getBlue()) / 3;
    }
    
    /// Whether the colour is considered dark (brightness < 128).
    bool isDark() const {
        return getBrightness() < 128;
    }
    
    /// Whether the colour is considered light.
    bool isLight() const {
        return getBrightness() >= 128;
    }
    
    /// Return either black or white for best contrast.
    Color getContrastColor() const {
        return isDark() ? Color::White : Color::Black;
    }
    
    // Predefined colors (RGB565 format)
    static const Color Black;      // 0x0000
    static const Color White;      // 0xFFFF
    static const Color Red;        // 0xF800
    static const Color Green;      // 0x07E0
    static const Color Blue;       // 0x001F
    static const Color Yellow;     // 0xFFE0
    static const Color Magenta;    // 0xF81F
    static const Color Cyan;       // 0x07FF
    static const Color Orange;     // 0xFD20
    static const Color Purple;     // 0x8010
    static const Color Pink;       // 0xFC18
    static const Color Brown;      // 0xA145
    static const Color Gray;       // 0x8410
    static const Color LightGray;  // 0xC618
    static const Color DarkGray;   // 0x4208
    
    // Material Design inspired colors
    static const Color MaterialRed;
    static const Color MaterialPink;
    static const Color MaterialPurple;
    static const Color MaterialDeepPurple;
    static const Color MaterialIndigo;
    static const Color MaterialBlue;
    static const Color MaterialLightBlue;
    static const Color MaterialCyan;
    static const Color MaterialTeal;
    static const Color MaterialGreen;
    static const Color MaterialLightGreen;
    static const Color MaterialLime;
    static const Color MaterialYellow;
    static const Color MaterialAmber;
    static const Color MaterialOrange;
    static const Color MaterialDeepOrange;
    static const Color MaterialBrown;
    static const Color MaterialGrey;
    static const Color MaterialBlueGrey;
    
    // Theme colors
    static const Color Primary;
    static const Color Secondary;
    static const Color Success;
    static const Color Warning;
    static const Color Error;
    static const Color Info;
    
    // Comparison operators
    bool operator==(const Color& other) const { return _value == other._value; }
    bool operator!=(const Color& other) const { return _value != other._value; }
    
    // Assignment
    Color& operator=(uint16_t value) { _value = value; return *this; }
    Color& operator=(const Color& other) { _value = other._value; return *this; }
}; 