#include "Color.h"

// Basic colors
const Color Color::Black(0x0000);
const Color Color::White(0xFFFF);
const Color Color::Red(0xF800);
const Color Color::Green(0x07E0);
const Color Color::Blue(0x001F);
const Color Color::Yellow(0xFFE0);
const Color Color::Magenta(0xF81F);
const Color Color::Cyan(0x07FF);
const Color Color::Orange(0xFD20);
const Color Color::Purple(0x8010);
const Color Color::Pink(0xFC18);
const Color Color::Brown(0xA145);
const Color Color::Gray(0x8410);
const Color Color::LightGray(0xC618);
const Color Color::DarkGray(0x4208);

// Material Design inspired colors
const Color Color::MaterialRed(0xF800);           // Red 500
const Color Color::MaterialPink(0xF81F);          // Pink 500
const Color Color::MaterialPurple(0x8010);        // Purple 500
const Color Color::MaterialDeepPurple(0x6010);    // Deep Purple 500
const Color Color::MaterialIndigo(0x4010);        // Indigo 500
const Color Color::MaterialBlue(0x001F);          // Blue 500
const Color Color::MaterialLightBlue(0x049F);     // Light Blue 500
const Color Color::MaterialCyan(0x07FF);          // Cyan 500
const Color Color::MaterialTeal(0x07E0);          // Teal 500
const Color Color::MaterialGreen(0x07E0);         // Green 500
const Color Color::MaterialLightGreen(0x87E0);    // Light Green 500
const Color Color::MaterialLime(0x87E0);          // Lime 500
const Color Color::MaterialYellow(0xFFE0);        // Yellow 500
const Color Color::MaterialAmber(0xFFE0);         // Amber 500
const Color Color::MaterialOrange(0xFD20);        // Orange 500
const Color Color::MaterialDeepOrange(0xFA20);    // Deep Orange 500
const Color Color::MaterialBrown(0xA145);         // Brown 500
const Color Color::MaterialGrey(0x8410);          // Grey 500
const Color Color::MaterialBlueGrey(0x5AEB);      // Blue Grey 500

// Theme colors (using Material Design as base)
const Color Color::Primary(Color::MaterialBlue);
const Color Color::Secondary(Color::MaterialGrey);
const Color Color::Success(Color::MaterialGreen);
const Color Color::Warning(Color::MaterialAmber);
const Color Color::Error(Color::MaterialRed);
const Color Color::Info(Color::MaterialCyan); 