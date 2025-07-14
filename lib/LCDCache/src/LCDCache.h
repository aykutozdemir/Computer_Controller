/**
 * @file LCDCache.h
 * @brief High-performance LCD display cache with run-length encoding
 * @author Computer Controller Project
 * @version 1.1.0
 * @date 2024
 * 
 * @section Description
 * LCDCache provides efficient memory management for LCD displays using run-length 
 * encoding compression. It supports automatic thread-safe operations, dirty row 
 * tracking for optimized updates, and automatic color deduplication.
 * 
 * @section Features
 * - Run-Length Encoding: Compresses consecutive pixels of the same color
 * - Automatic Thread Safety: All operations are automatically thread-safe
 * - Dirty Row Tracking: Optimizes display updates by tracking changed rows
 * - Color Deduplication: Automatically deduplicates colors to save memory
 * - Memory Pool Management: Reuses memory allocations for better performance
 * - Equal Color Treatment: All colors are treated equally
 * 
 * @section Usage
 * @code
 * #include "LCDCache.h"
 * 
 * // Create a cache for a 320x240 display
 * LCDCache cache(320, 240);
 * 
 * // Set pixels (automatically thread-safe)
 * cache.setPixel(10, 20, 0xFF0000);  // Red pixel
 * 
 * // Check if row needs updating
 * if (cache.isRowDirty(20)) {
 *     // Update display for row 20
 *     cache.markRowClean(20);
 * }
 * @endcode
 * 
 * @section Thread Safety
 * All methods are automatically thread-safe using FreeRTOS critical sections.
 * No manual locking is required:
 * @code
 * // These operations are automatically thread-safe
 * cache.setPixel(100, 100, 0x00FF00);
 * ColorValue color = cache.getPixel(100, 100);
 * cache.clear();
 * @endcode
 * 
 * @section Memory Management
 * - Color deduplication: Each unique color stored only once
 * - Run-length encoding: Consecutive pixels of same color compressed
 * - Memory pool: Reuses allocated memory for better performance
 * - Equal treatment: All colors are stored in cache
 * 
 * @section Performance
 * - Memory usage: Significantly reduces memory for displays with repeated colors
 * - Update efficiency: Only update rows that have changed
 * - Thread safety: Minimal overhead for automatic thread-safe operations
 * - Color limits: Supports up to 65,535 unique colors per cache
 * 
 * @section Requirements
 * - ESP32 or ESP8266 (or any FreeRTOS-based system)
 * - FreeRTOS
 * - C++11 or later
 * 
 * @section License
 * This library is part of the Computer Controller project.
 */

#pragma once
#include <unordered_map>
#include <vector>
#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

// ============================================================================
// ABSTRACT DISPLAY INTERFACE
// ============================================================================

/**
 * @brief Abstract interface for display drivers
 * 
 * This interface allows LCDCache to work with any display driver that
 * implements the basic pixel drawing functionality.
 */
class IDisplayDriver {
public:
    virtual ~IDisplayDriver() = default;
    virtual void drawPixel(uint16_t x, uint16_t y, uint16_t color) = 0;
    virtual void flush() {}  // Optional flush operation
};

/**
 * @class LCDCache
 * @brief High-performance LCD display cache with run-length encoding
 * 
 * LCDCache provides efficient memory management for LCD displays using run-length 
 * encoding compression. It automatically compresses consecutive pixels of the same 
 * color and tracks which rows have been modified for optimized display updates.
 * 
 * The class is designed to be automatically thread-safe and memory-efficient, 
 * making it ideal for embedded systems with limited RAM. All colors are treated 
 * equally and stored in the cache. No manual locking is required.
 */
class LCDCache {
public:
    /** @brief Color value type (32-bit RGB) */
    using ColorValue = uint32_t;
    
    /** @brief Color ID type for internal color management */
    using ColorID = uint16_t;

    /**
     * @brief Constructor
     * @param width Display width in pixels
     * @param height Display height in pixels
     * 
     * Creates a new LCD cache with the specified dimensions.
     * The cache will automatically handle memory management and color deduplication.
     * All colors are treated equally and assigned unique IDs.
     */
    LCDCache(uint16_t width, uint16_t height);

    /**
     * @brief Clear the cache while keeping allocated memory
     * 
     * Removes all pixel data from the cache but preserves allocated memory
     * for reuse. This is more efficient than destroying and recreating the cache.
     * Automatically thread-safe.
     */
    void clear();
    
    /**
     * @brief Set a pixel to a specific color
     * @param x X coordinate (0 to width-1)
     * @param y Y coordinate (0 to height-1)
     * @param color Color value (32-bit RGB)
     * 
     * Sets the pixel at the specified coordinates to the given color.
     * All colors are treated equally and stored in the cache.
     * The row is automatically marked as dirty for efficient display updates.
     * Automatically thread-safe.
     */
    void setPixel(uint16_t x, uint16_t y, ColorValue color);
    
    /**
     * @brief Get the color of a pixel
     * @param x X coordinate (0 to width-1)
     * @param y Y coordinate (0 to height-1)
     * @return Color value of the pixel, or 0 if not set
     * Automatically thread-safe.
     */
    ColorValue getPixel(uint16_t x, uint16_t y) const;

    /**
     * @brief Get the display width
     * @return Display width in pixels
     */
    uint16_t getWidth() const;
    
    /**
     * @brief Get the display height
     * @return Display height in pixels
     */
    uint16_t getHeight() const;

    /**
     * @brief Check if a row is dirty (needs updating)
     * @param y Y coordinate of the row
     * @return true if the row has been modified since last clean mark
     * 
     * Use this to determine which rows need to be updated on the display.
     * Only rows that have been modified will return true.
     * Automatically thread-safe.
     */
    bool isRowDirty(uint16_t y) const;
    
    /**
     * @brief Mark a row as clean (updated)
     * @param y Y coordinate of the row
     * 
     * Call this after updating the display for a specific row to mark it as clean.
     * This prevents unnecessary re-updates of the same row.
     * Automatically thread-safe.
     */
    void markRowClean(uint16_t y);
    
    /**
     * @brief Mark a row as dirty (needs update)
     * @param y Y coordinate of the row
     * Automatically thread-safe.
     */
    void markRowDirty(uint16_t y);
    
    /**
     * @brief Clear all dirty row flags
     * 
     * Marks all rows as clean, effectively resetting the dirty tracking system.
     * Use this when you want to reset the dirty tracking or after a full
     * display update has been completed.
     * Automatically thread-safe.
     */
    void clearDirtyFlags();
    
    /**
     * @brief Update a single pixel in the cache
     * @param x X coordinate (0 to width-1)
     * @param y Y coordinate (0 to height-1)
     * @param color RGB565 color value
     * 
     * This method converts RGB565 to RGB888 and calls setPixel.
     */
    void updateCachePixel(uint16_t x, uint16_t y, uint16_t color);
    
    /**
     * @brief Update a rectangular region in the cache
     * @param x X coordinate of top-left corner
     * @param y Y coordinate of top-left corner
     * @param w Width of rectangle
     * @param h Height of rectangle
     * 
     * This method marks the region as dirty for efficient updates.
     */
    void updateCacheRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    
    /**
     * @brief Update the display with cached data
     * 
     * This method flushes dirty rows to the display hardware.
     * It should be called periodically to update the display.
     */
    void update();
    
    /**
     * @brief Set the display driver for cache updates
     * @param driver Pointer to display driver implementing IDisplayDriver interface
     * 
     * This method sets the display driver that will be used to flush
     * cached data to the hardware display. The driver must implement the
     * IDisplayDriver interface.
     */
    void setDisplayDriver(IDisplayDriver* driver);

    /**
     * @brief Destructor – frees all allocated RunList objects.
     */
    ~LCDCache();

private:
    /**
     * @brief Run structure for run-length encoding
     * 
     * Represents a sequence of consecutive pixels with the same color.
     */
    struct Run {
        uint16_t startX;  /**< Starting X coordinate of the run */
        uint16_t length;  /**< Number of pixels in the run */
    };

    using RunList = std::vector<Run>;  /**< List of runs for a color */
    using RowColorMap = std::unordered_map<ColorID, RunList*>;  /**< Color to runs mapping for a row */
    
    /**
     * @brief Main cache storage: one entry per physical row.
     * Using a flat vector removes hash-table overhead and guarantees
     * O(1) access by index.
     */
    std::vector<RowColorMap> cache;

    /**
     * @brief Dirty row tracking: compact bit/byte per row instead of a hash map.
     */
    std::vector<bool> dirtyRows;

    /** @brief Color value to ID mapping for deduplication */
    std::unordered_map<ColorValue, ColorID> colorToId;
    
    /** @brief ID to color value mapping for resolution */
    std::vector<ColorValue> idToColor;
    
    /** @brief Next available color ID */
    ColorID nextColorId;

    /** @brief Pool of reusable RunList objects */
    std::vector<RunList*> freePool;
    
    /** @brief Spinlock for thread safety */
    mutable portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
    
    /** @brief Pointer to display driver for hardware updates */
    IDisplayDriver* displayDriver = nullptr;

    /**
     * @brief Release memory for a row's color map
     * @param row Reference to the row's color map
     */
    void releaseRowMap(RowColorMap& row);
    
    /**
     * @brief Add a pixel to a run list
     * @param list Pointer to the run list
     * @param x X coordinate of the pixel
     * 
     * Efficiently adds a pixel to a run list, merging with existing runs when possible.
     */
    void addRun(RunList* list, uint16_t x);
    
    /**
     * @brief Register a color and get its ID
     * @param color Color value to register
     * @return Color ID for the registered color
     * 
     * Ensures each unique color is stored only once and returns a unique ID.
     */
    ColorID registerColor(ColorValue color);
    
    /**
     * @brief Resolve a color ID to its value
     * @param id Color ID to resolve
     * @return Color value for the given ID
     */
    ColorValue resolveColor(ColorID id) const;

    uint16_t screenWidth, screenHeight;  /**< Display dimensions */
}; 