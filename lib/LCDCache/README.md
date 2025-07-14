# LCDCache Library

High-performance LCD display cache with run-length encoding for embedded systems.

## Overview

LCDCache provides efficient memory management for LCD displays using run-length encoding compression. It supports thread-safe operations, dirty row tracking for optimized updates, and automatic color deduplication.

## Key Features

- **Run-Length Encoding**: Compresses consecutive pixels of the same color
- **Thread-Safe Operations**: Built-in spinlock protection for multi-threaded environments
- **Dirty Row Tracking**: Optimizes display updates by tracking changed rows
- **Color Deduplication**: Automatically deduplicates colors to save memory
- **Memory Pool Management**: Reuses memory allocations for better performance
- **Abstract Display Interface**: Works with any display driver through IDisplayDriver interface

## Abstract Display Interface

The LCDCache class now uses an abstract `IDisplayDriver` interface, making it compatible with any display driver that implements the interface. This provides better flexibility and removes direct dependencies on specific display libraries.

### Interface Definition

```cpp
class IDisplayDriver {
public:
    virtual ~IDisplayDriver() = default;
    virtual void drawPixel(uint16_t x, uint16_t y, uint16_t color) = 0;
    virtual void flush() {}  // Optional flush operation
};
```

### Usage Examples

#### 1. Using Arduino_GFX with the provided adapter

```cpp
#include "LCDCache.h"
#include "ArduinoGFXAdapter.h"
#include <Arduino_GFX_Library.h>

// Create your Arduino_GFX display
Arduino_GFX* gfx = new Arduino_ST7735(/* parameters */);

// Create the adapter
ArduinoGFXAdapter adapter(gfx);

// Create cache and set display driver
LCDCache cache(320, 240);
cache.setDisplayDriver(&adapter);

// Use the cache
cache.setPixel(10, 20, 0xFF0000);  // Red pixel
cache.update();  // Flush to display
```

#### 2. Using function pointers with FunctionAdapter

```cpp
#include "LCDCache.h"
#include "ArduinoGFXAdapter.h"

// Your display functions
void myDrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    // Your display implementation
}

void myFlush() {
    // Optional flush implementation
}

// Create function adapter
FunctionAdapter adapter(myDrawPixel, myFlush);

// Create cache and set display driver
LCDCache cache(320, 240);
cache.setDisplayDriver(&adapter);
```

#### 3. Creating a custom adapter

```cpp
#include "LCDCache.h"

class MyCustomAdapter : public IDisplayDriver {
private:
    MyDisplayDriver* myDisplay;
    
public:
    MyCustomAdapter(MyDisplayDriver* display) : myDisplay(display) {}
    
    void drawPixel(uint16_t x, uint16_t y, uint16_t color) override {
        myDisplay->setPixel(x, y, color);
    }
    
    void flush() override {
        myDisplay->refresh();
    }
};

// Usage
MyDisplayDriver* myDisplay = new MyDisplayDriver();
MyCustomAdapter adapter(myDisplay);
LCDCache cache(320, 240);
cache.setDisplayDriver(&adapter);
```

## Migration Guide

### From Previous Versions

The main change is the introduction of the abstract display interface. Here's how to migrate:

#### Before (old version):
```cpp
#include <Arduino_GFX_Library.h>
Arduino_GFX* gfx = new Arduino_ST7735(/* parameters */);
LCDCache cache(320, 240);
cache.setDisplayDriver(gfx);  // Direct void* pointer
```

#### After (new version):
```cpp
#include <Arduino_GFX_Library.h>
#include "ArduinoGFXAdapter.h"
Arduino_GFX* gfx = new Arduino_ST7735(/* parameters */);
ArduinoGFXAdapter adapter(gfx);
LCDCache cache(320, 240);
cache.setDisplayDriver(&adapter);  // Abstract interface
```

### Legacy Compatibility

The old `setDisplayDriver(void*)` method is still available but deprecated. It will log a warning and set the display driver to nullptr. Users should migrate to the new interface-based approach.

## Basic Usage

### Creating a Cache

```cpp
#include "LCDCache.h"

// Create a cache for a 320x240 display with black background
LCDCache cache(320, 240, 0x000000);
```

### Setting Pixels

```cpp
// Set individual pixels
cache.setPixel(10, 20, 0xFF0000);  // Red pixel
cache.setPixel(11, 20, 0x00FF00);  // Green pixel
cache.setPixel(12, 20, 0x0000FF);  // Blue pixel

// RGB565 format (for SimpleUI compatibility)
cache.updateCachePixel(10, 21, 0xF800);  // Red in RGB565
```

### Efficient Display Updates

```cpp
// Check which rows need updating
for (uint16_t y = 0; y < cache.getHeight(); y++) {
    if (cache.isRowDirty(y)) {
        // Update display for this row
        // ... your display update code ...
        cache.markRowClean(y);
    }
}

// Or use the built-in update method
cache.update();  // Automatically updates all dirty rows
```

### Thread Safety

```cpp
// Manual locking
cache.lock();
cache.setPixel(100, 100, 0x00FF00);
cache.setPixel(101, 100, 0x00FF00);
cache.unlock();

// Or use thread-safe methods
cache.setPixelSafe(100, 100, 0x00FF00);
cache.clearSafe();
```

## Performance Tips

1. **Use dirty row tracking**: Only update rows that have changed
2. **Batch operations**: Lock once and perform multiple operations
3. **Color reuse**: The cache automatically deduplicates colors
4. **Background optimization**: Background pixels are not stored in memory

## Requirements

- ESP32 or ESP8266 (or any FreeRTOS-based system)
- FreeRTOS
- C++11 or later
- Display driver implementing IDisplayDriver interface

## License

This library is part of the Computer Controller project.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests. 