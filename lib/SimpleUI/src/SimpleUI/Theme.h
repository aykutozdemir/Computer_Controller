/**
 * @file Theme.h
 * @brief Theme and color scheme manager for the SimpleUI library.
 */

#pragma once

#include "Color.h"

class Theme {
public:
    // Typography structure for text sizes
    struct Typography {
        uint8_t title;        // Large title text
        uint8_t header;       // Medium header text
        uint8_t body;         // Small body text
        uint8_t button;       // Button text size
        uint8_t status;       // Status text size
        uint8_t label;        // Label text size
        uint8_t value;        // Value text size
        uint8_t caption;      // Caption/small text
    };
    
    // Theme structure
    struct ColorScheme {
        Color primary;
        Color secondary;
        Color background;
        Color surface;
        Color text;
        Color textSecondary;
        Color success;
        Color warning;
        Color error;
        Color info;
        Color border;
        Color shadow;
        Typography typography;  // Text sizes
    };
    
    // Predefined themes
    static const ColorScheme Light;
    static const ColorScheme Dark;
    static const ColorScheme Blue;
    static const ColorScheme Green;
    static const ColorScheme Red;
    static const ColorScheme Purple;
    static const ColorScheme Orange;
    static const ColorScheme Material;
    static const ColorScheme HighContrast;
    
    // Current theme
    static ColorScheme currentTheme;
    
    /// Set the global UI theme.
    static void setTheme(const ColorScheme& theme);
    /// Retrieve the currently active theme.
    static const ColorScheme& getCurrentTheme();
    
    // Quick access to current theme colors
    static Color getPrimary() { return currentTheme.primary; }
    static Color getSecondary() { return currentTheme.secondary; }
    static Color getBackground() { return currentTheme.background; }
    static Color getSurface() { return currentTheme.surface; }
    static Color getText() { return currentTheme.text; }
    static Color getTextSecondary() { return currentTheme.textSecondary; }
    static Color getSuccess() { return currentTheme.success; }
    static Color getWarning() { return currentTheme.warning; }
    static Color getError() { return currentTheme.error; }
    static Color getInfo() { return currentTheme.info; }
    static Color getBorder() { return currentTheme.border; }
    static Color getShadow() { return currentTheme.shadow; }
    
    // Quick access to current theme typography
    static Typography getTypography() { return currentTheme.typography; }
    static uint8_t getTitleSize() { return currentTheme.typography.title; }
    static uint8_t getHeaderSize() { return currentTheme.typography.header; }
    static uint8_t getBodySize() { return currentTheme.typography.body; }
    static uint8_t getButtonSize() { return currentTheme.typography.button; }
    static uint8_t getStatusSize() { return currentTheme.typography.status; }
    static uint8_t getLabelSize() { return currentTheme.typography.label; }
    static uint8_t getValueSize() { return currentTheme.typography.value; }
    static uint8_t getCaptionSize() { return currentTheme.typography.caption; }
    
    /// Create a custom theme from individual colours.
    static ColorScheme createCustomTheme(
        Color primary,
        Color secondary = Color::Gray,
        Color background = Color::White,
        Color surface = Color::LightGray,
        Color text = Color::Black,
        Color textSecondary = Color::DarkGray
    );
    
    /// Create a custom theme with typography.
    static ColorScheme createCustomThemeWithTypography(
        Color primary,
        Color secondary,
        Color background,
        Color surface,
        Color text,
        Color textSecondary,
        Typography typography
    );
    
    /// Generate a dark theme variant based on a primary colour.
    static ColorScheme createDarkTheme(Color primary);
    /// Generate a light theme variant based on a primary colour.
    static ColorScheme createLightTheme(Color primary);
    
    /// Create default typography settings.
    static Typography createDefaultTypography();
    /// Create compact typography settings (smaller sizes).
    static Typography createCompactTypography();
    /// Create large typography settings (bigger sizes).
    static Typography createLargeTypography();
}; 