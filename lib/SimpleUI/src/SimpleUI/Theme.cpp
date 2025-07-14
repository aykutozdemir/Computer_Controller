#include "Theme.h"

// -----------------------------------------------------------------------------
// Typography presets
// -----------------------------------------------------------------------------

Theme::Typography Theme::createDefaultTypography() {
    return {
        3,  // title - Large title text
        2,  // header - Medium header text
        2,  // body - Small body text
        2,  // button - Button text size
        2,  // status - Status text size
        2,  // label - Label text size
        2,  // value - Value text size
        2   // caption - Caption/small text
    };
}

Theme::Typography Theme::createCompactTypography() {
    return {
        2,  // title - Smaller title
        1,  // header - Smaller header
        1,  // body - Small body text
        1,  // button - Smaller button text
        1,  // status - Small status text
        1,  // label - Small label text
        1,  // value - Small value text
        1   // caption - Small caption text
    };
}

Theme::Typography Theme::createLargeTypography() {
    return {
        4,  // title - Extra large title
        3,  // header - Large header
        2,  // body - Medium body text
        3,  // button - Large button text
        2,  // status - Medium status text
        2,  // label - Medium label text
        2,  // value - Medium value text
        2   // caption - Small caption text
    };
}

// -----------------------------------------------------------------------------
// Pre-defined theme color schemes
// Order of definitions matters for static initialisation – define the schemes
// first, THEN initialise currentTheme with one of them.
// -----------------------------------------------------------------------------

const Theme::ColorScheme Theme::Light = {
    Color::MaterialBlue,   // primary
    Color::MaterialGrey,   // secondary
    Color::White,          // background
    Color::LightGray,      // surface
    Color::Black,          // text
    Color::DarkGray,       // textSecondary
    Color::MaterialGreen,  // success
    Color::MaterialAmber,  // warning
    Color::MaterialRed,    // error
    Color::MaterialCyan,   // info
    Color::Gray,           // border
    Color::DarkGray,       // shadow
    createDefaultTypography()  // typography
};

const Theme::ColorScheme Theme::Dark = {
    Color::MaterialLightBlue,  // primary
    Color::MaterialBlueGrey,   // secondary
    Color::Black,              // background
    Color::DarkGray,           // surface
    Color::White,              // text
    Color::LightGray,          // textSecondary
    Color::MaterialLightGreen, // success
    Color::MaterialAmber,      // warning
    Color::MaterialRed,        // error
    Color::MaterialCyan,       // info
    Color::Gray,               // border
    Color::Black,              // shadow
    createDefaultTypography()  // typography
};

const Theme::ColorScheme Theme::Blue = {
    Color::MaterialBlue,                   // primary
    Color::MaterialLightBlue,              // secondary
    Color::White,                          // background
    Color(231, 243, 255),                  // surface (very light blue)
    Color::Black,                          // text
    Color::DarkGray,                       // textSecondary
    Color::MaterialGreen,                  // success
    Color::MaterialAmber,                  // warning
    Color::MaterialRed,                    // error
    Color::MaterialCyan,                   // info
    Color::MaterialLightBlue,              // border
    Color::DarkGray,                       // shadow
    createDefaultTypography()              // typography
};

const Theme::ColorScheme Theme::Green = {
    Color::MaterialGreen,                  // primary
    Color::MaterialLightGreen,             // secondary
    Color::White,                          // background
    Color(232, 245, 232),                  // surface (very light green)
    Color::Black,                          // text
    Color::DarkGray,                       // textSecondary
    Color::MaterialGreen,                  // success
    Color::MaterialAmber,                  // warning
    Color::MaterialRed,                    // error
    Color::MaterialCyan,                   // info
    Color::MaterialLightGreen,             // border
    Color::DarkGray,                       // shadow
    createDefaultTypography()              // typography
};

const Theme::ColorScheme Theme::Red = {
    Color::MaterialRed,                    // primary
    Color::MaterialDeepOrange,             // secondary
    Color::White,                          // background
    Color(255, 232, 232),                  // surface (very light red)
    Color::Black,                          // text
    Color::DarkGray,                       // textSecondary
    Color::MaterialGreen,                  // success
    Color::MaterialAmber,                  // warning
    Color::MaterialRed,                    // error
    Color::MaterialCyan,                   // info
    Color::MaterialRed,                    // border
    Color::DarkGray,                       // shadow
    createDefaultTypography()              // typography
};

const Theme::ColorScheme Theme::Purple = {
    Color::MaterialPurple,                 // primary
    Color::MaterialDeepPurple,             // secondary
    Color::White,                          // background
    Color(243, 229, 245),                  // surface (very light purple)
    Color::Black,                          // text
    Color::DarkGray,                       // textSecondary
    Color::MaterialGreen,                  // success
    Color::MaterialAmber,                  // warning
    Color::MaterialRed,                    // error
    Color::MaterialCyan,                   // info
    Color::MaterialPurple,                 // border
    Color::DarkGray,                       // shadow
    createDefaultTypography()              // typography
};

const Theme::ColorScheme Theme::Orange = {
    Color::MaterialOrange,                 // primary
    Color::MaterialDeepOrange,             // secondary
    Color::White,                          // background
    Color(255, 243, 224),                  // surface (very light orange)
    Color::Black,                          // text
    Color::DarkGray,                       // textSecondary
    Color::MaterialGreen,                  // success
    Color::MaterialAmber,                  // warning
    Color::MaterialRed,                    // error
    Color::MaterialCyan,                   // info
    Color::MaterialOrange,                 // border
    Color::DarkGray,                       // shadow
    createDefaultTypography()              // typography
};

const Theme::ColorScheme Theme::Material = {
    Color::MaterialBlue,                   // primary
    Color::MaterialGrey,                   // secondary
    Color::White,                          // background
    Color(250, 250, 250),                  // surface
    Color(33, 33, 33),                     // text (approx 0x212121)
    Color(117, 117, 117),                  // textSecondary (0x757575)
    Color::MaterialGreen,                  // success
    Color::MaterialAmber,                  // warning
    Color::MaterialRed,                    // error
    Color::MaterialCyan,                   // info
    Color(224, 224, 224),                  // border (0xE0E0E0)
    Color::Black,                          // shadow
    createDefaultTypography()              // typography
};

const Theme::ColorScheme Theme::HighContrast = {
    Color::Yellow,   // primary
    Color::Cyan,     // secondary
    Color::Black,    // background
    Color::DarkGray, // surface
    Color::Yellow,   // text
    Color::Cyan,     // textSecondary
    Color::Green,    // success
    Color::Orange,   // warning
    Color::Red,      // error
    Color::Cyan,     // info
    Color::Yellow,   // border
    Color::Black,    // shadow
    createLargeTypography()  // typography - use larger text for high contrast
};

// -----------------------------------------------------------------------------
// Current theme (initialised AFTER predefined schemes)
// -----------------------------------------------------------------------------

Theme::ColorScheme Theme::currentTheme = Theme::Light;

// -----------------------------------------------------------------------------
// Theme management functions
// -----------------------------------------------------------------------------

void Theme::setTheme(const ColorScheme &theme) {
    currentTheme = theme;
}

const Theme::ColorScheme &Theme::getCurrentTheme() {
    return currentTheme;
}

// -----------------------------------------------------------------------------
// Helper factories (no designated initialisers)
// -----------------------------------------------------------------------------

Theme::ColorScheme Theme::createCustomTheme(
    Color primary,
    Color secondary,
    Color background,
    Color surface,
    Color text,
    Color textSecondary
) {
    return {
        primary,
        secondary,
        background,
        surface,
        text,
        textSecondary,
        Color::MaterialGreen,  // success
        Color::MaterialAmber,  // warning
        Color::MaterialRed,    // error
        Color::MaterialCyan,   // info
        Color::Gray,           // border
        Color::DarkGray,       // shadow
        createDefaultTypography()  // typography
    };
}

Theme::ColorScheme Theme::createCustomThemeWithTypography(
    Color primary,
    Color secondary,
    Color background,
    Color surface,
    Color text,
    Color textSecondary,
    Typography typography
) {
    return {
        primary,
        secondary,
        background,
        surface,
        text,
        textSecondary,
        Color::MaterialGreen,  // success
        Color::MaterialAmber,  // warning
        Color::MaterialRed,    // error
        Color::MaterialCyan,   // info
        Color::Gray,           // border
        Color::DarkGray,       // shadow
        typography
    };
}

Theme::ColorScheme Theme::createDarkTheme(Color primary) {
    return {
        primary,
        primary.darken(20),    // secondary
        Color::Black,          // background
        Color::DarkGray,       // surface
        Color::White,          // text
        Color::LightGray,      // textSecondary
        Color::MaterialGreen,  // success
        Color::MaterialAmber,  // warning
        Color::MaterialRed,    // error
        Color::MaterialCyan,   // info
        Color::Gray,           // border
        Color::Black,          // shadow
        createDefaultTypography()  // typography
    };
}

Theme::ColorScheme Theme::createLightTheme(Color primary) {
    return {
        primary,
        primary.lighten(20),   // secondary
        Color::White,          // background
        Color::LightGray,      // surface
        Color::Black,          // text
        Color::DarkGray,       // textSecondary
        Color::MaterialGreen,  // success
        Color::MaterialAmber,  // warning
        Color::MaterialRed,    // error
        Color::MaterialCyan,   // info
        Color::Gray,           // border
        Color::DarkGray,       // shadow
        createDefaultTypography()  // typography
    };
} 