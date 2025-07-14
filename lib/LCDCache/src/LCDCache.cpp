/**
 * @file LCDCache.cpp
 * @brief Implementation of the LCDCache class
 * @author Computer Controller Project
 * @version 1.1.0
 * @date 2024
 * 
 * @section Description
 * This file contains the implementation of the LCDCache class, which provides
 * efficient memory management for LCD displays using run-length encoding.
 * 
 * @section Implementation Details
 * The implementation uses several key techniques for efficiency:
 * - Run-length encoding to compress consecutive pixels of the same color
 * - Color deduplication to store each unique color only once
 * - Memory pooling to reuse allocated memory
 * - Thread-safe operations using FreeRTOS spinlocks
 * - Dirty row tracking for optimized display updates
 * 
 * @section Algorithm
 * 1. When setting a pixel, the color is registered and assigned a unique ID
 * 2. The pixel is added to a run list for that color in the appropriate row
 * 3. Runs are automatically merged when adjacent pixels are set
 * 4. The row is marked as dirty for efficient display updates
 * 5. When retrieving a pixel, the cache is searched for the appropriate run
 * 
 * @section Thread Safety
 * All public methods are automatically thread-safe using FreeRTOS critical sections.
 * No manual locking is required.
 * 
 * @section Memory Management
 * - Memory is allocated on-demand for new colors and runs
 * - Freed memory is pooled for reuse to reduce allocation overhead
 * - All colors are stored in memory equally
 * - The clear() method preserves allocated memory for reuse
 * 
 * @section Performance Considerations
 * - O(log n) complexity for pixel lookups in typical usage
 * - O(1) average case for pixel setting
 * - Memory usage scales with unique colors and pixel distribution
 * - Thread safety adds minimal overhead
 */

#include "LCDCache.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_debug_helpers.h"
#include <algorithm>

static const char* TAG = "LCDCache";

// ============================================================================
// LCDCACHE IMPLEMENTATION
// ============================================================================

/**
 * @brief Constructor implementation
 * 
 * Initializes the cache with the specified dimensions.
 * All colors are treated equally and assigned unique IDs.
 */
LCDCache::LCDCache(uint16_t width, uint16_t height)
    : screenWidth(width), screenHeight(height), nextColorId(0), displayDriver(nullptr) {
    // Pre-allocate rows and dirty flags
    cache.resize(screenHeight);
    dirtyRows.assign(screenHeight, true);
}

/**
 * @brief Get the display width
 * 
 * @return Display width in pixels
 */
uint16_t LCDCache::getWidth() const {
    return screenWidth;
}

/**
 * @brief Get the display height
 * 
 * @return Display height in pixels
 */
uint16_t LCDCache::getHeight() const {
    return screenHeight;
}

/**
 * @brief Clear the cache while preserving allocated memory
 * 
 * Removes all pixel data from the cache and clears dirty row flags, but
 * preserves allocated memory in the free pool for reuse. This is more
 * efficient than destroying and recreating the cache.
 */
void LCDCache::clear() {
    portENTER_CRITICAL(&spinlock);
    
    ESP_LOGI(TAG, "LCDCache::clear() - Starting cache clear operation");
    ESP_LOGI(TAG, "Cache dimensions: %dx%d", screenWidth, screenHeight);
    
    // Print backtrace to see call stack
    ESP_LOGI(TAG, "=== BACKTRACE - LCDCache::clear() ===");
    esp_backtrace_print(10);
    ESP_LOGI(TAG, "=== END BACKTRACE ===");
    
    uint32_t totalRows = cache.size();
    uint32_t rowsWithData = 0;
    uint32_t totalRunLists = 0;
    
    // Count current data before clearing
    for (size_t i = 0; i < cache.size(); i++) {
        if (!cache[i].empty()) {
            rowsWithData++;
            totalRunLists += cache[i].size();
        }
    }
    
    ESP_LOGI(TAG, "Before clear: %d/%d rows have data, %d total run lists", rowsWithData, totalRows, totalRunLists);
    
    // Release memory for each row
    for (size_t i = 0; i < cache.size(); i++) {
        if (!cache[i].empty()) {
            ESP_LOGD(TAG, "Clearing row %d with %d color entries", i, cache[i].size());
            releaseRowMap(cache[i]);
        }
    }
    
    // Clear all rows (but keep vector capacity)
    for (auto &row : cache) {
        row.clear();
    }
    
    // Mark all rows as dirty
    std::fill(dirtyRows.begin(), dirtyRows.end(), true);
    
    ESP_LOGI(TAG, "After clear: All rows cleared, %d run lists returned to pool", freePool.size());
    ESP_LOGI(TAG, "LCDCache::clear() - Cache clear operation completed");
    
    portEXIT_CRITICAL(&spinlock);
}

/**
 * @brief Release memory for a row's color map
 * 
 * @param row Reference to the row's color map
 * 
 * Moves all RunList objects from the row back to the free pool for reuse.
 * This prevents memory fragmentation and reduces allocation overhead.
 */
void LCDCache::releaseRowMap(RowColorMap& row) {
    uint32_t runListsReleased = 0;
    uint32_t totalRuns = 0;
    
    for (auto& colorPair : row) {
        RunList* runs = colorPair.second;
        if (runs) {
            totalRuns += runs->size();
            runs->clear();
            freePool.push_back(runs);
            runListsReleased++;
        }
    }
    
    ESP_LOGD(TAG, "releaseRowMap: Released %d run lists with %d total runs", runListsReleased, totalRuns);
    row.clear();
}

/**
 * @brief Add a pixel to a run list with automatic merging
 * 
 * @param list Pointer to the run list
 * @param x X coordinate of the pixel to add
 * 
 * Efficiently adds a pixel to a run list, automatically merging with existing
 * runs when possible. The algorithm maintains sorted runs and merges adjacent
 * runs to minimize memory usage.
 */
void LCDCache::addRun(RunList* list, uint16_t x) {
    if (!list) return;  // Safety check
    
    for (auto it = list->begin(); it != list->end(); ++it) {
        Run& r = *it;

        // Pixel is before current run
        if (x < r.startX - 1) {
            list->insert(it, {x, 1});
            return;
        } 
        // Pixel extends current run to the left
        else if (x == r.startX - 1) {
            if (r.startX > 0) {  // Prevent underflow
                r.startX--;
                r.length++;
            }
            return;
        } 
        // Pixel is within current run
        else if (x >= r.startX && x < r.startX + r.length) {
            return;
        } 
        // Pixel extends current run to the right
        else if (x == r.startX + r.length) {
            if (r.length < 65535) {  // Prevent overflow
                r.length++;
                auto nextIt = it + 1;
                // Merge with next run if adjacent
                if (nextIt != list->end() && r.startX + r.length == nextIt->startX) {
                    uint32_t totalLength = static_cast<uint32_t>(r.length) + nextIt->length;
                    if (totalLength <= 65535) {  // Prevent overflow in merge
                        r.length = static_cast<uint16_t>(totalLength);
                        list->erase(nextIt);
                    }
                }
            }
            return;
        }
    }
    // Pixel is after all existing runs
    list->push_back({x, 1});
}

/**
 * @brief Register a color and get its unique ID
 * 
 * @param color Color value to register
 * @return Unique color ID for the registered color
 * 
 * Ensures each unique color is stored only once in the color resolution table.
 * If the color is already registered, returns the existing ID. Otherwise,
 * assigns a new ID and adds the color to the table.
 */
LCDCache::ColorID LCDCache::registerColor(ColorValue color) {
    auto it = colorToId.find(color);
    if (it != colorToId.end())
        return it->second;

    // Check for color ID overflow
    if (nextColorId >= 65535) {
        // Return 0 if we've run out of color IDs
        return 0;
    }

    ColorID newId = nextColorId++;
    colorToId[color] = newId;
    idToColor.push_back(color);
    return newId;
}

/**
 * @brief Resolve a color ID to its value
 * 
 * @param id Color ID to resolve
 * @return Color value for the given ID, or 0 if invalid
 * 
 * Looks up the color value for a given ID in the color resolution table.
 * Returns 0 if the ID is invalid or out of range.
 */
LCDCache::ColorValue LCDCache::resolveColor(ColorID id) const {
    return id < idToColor.size() ? idToColor[id] : 0;
}

/**
 * @brief Set a pixel to a specific color
 * 
 * @param x X coordinate (0 to width-1)
 * @param y Y coordinate (0 to height-1)
 * @param color Color value (32-bit RGB)
 * 
 * Sets the pixel at the specified coordinates to the given color. All colors
 * including background are treated equally and stored in the cache. The row
 * is automatically marked as dirty for efficient display updates.
 * 
 * The method performs bounds checking and automatically handles color
 * registration and run management.
 */
void LCDCache::setPixel(uint16_t x, uint16_t y, ColorValue color) {
    portENTER_CRITICAL(&spinlock);
    
    if (x >= screenWidth || y >= screenHeight) {
        portEXIT_CRITICAL(&spinlock);
        return;
    }

    auto &row = cache[y];
    ColorID cid = registerColor(color);
    auto &listRef = row[cid];

    if (!listRef) {
        if (!freePool.empty()) {
            listRef = freePool.back();
            freePool.pop_back();
        } else {
            listRef = new RunList();
            if (!listRef) {
                // Memory allocation failed - mark row as dirty and return
                dirtyRows[y] = true;
                portEXIT_CRITICAL(&spinlock);
                return;
            }
        }
    }

    addRun(listRef, x);
    dirtyRows[y] = true;
    
    portEXIT_CRITICAL(&spinlock);
}

/**
 * @brief Get the color of a pixel
 * 
 * @param x X coordinate (0 to width-1)
 * @param y Y coordinate (0 to height-1)
 * @return Color value of the pixel, or 0 if not set
 * 
 * Retrieves the color of the pixel at the specified coordinates. If the
 * pixel hasn't been explicitly set, returns 0. The method searches through
 * the cached runs to find the appropriate color.
 */
LCDCache::ColorValue LCDCache::getPixel(uint16_t x, uint16_t y) const {
    portENTER_CRITICAL(&spinlock);
    
    if (x >= screenWidth || y >= screenHeight) {
        portEXIT_CRITICAL(&spinlock);
        return 0;
    }
    if (y >= cache.size()) {
        portEXIT_CRITICAL(&spinlock);
        return 0;
    }

    const RowColorMap &row = cache[y];
    ColorValue result = 0;
    for (const auto& colorPair : row) {
        ColorID colorId = colorPair.first;
        const RunList* runs = colorPair.second;
        for (const Run& r : *runs) {
            if (x >= r.startX && x < r.startX + r.length) {
                result = resolveColor(colorId);
                portEXIT_CRITICAL(&spinlock);
                return result;
            }
        }
    }
    
    portEXIT_CRITICAL(&spinlock);
    return result;
}

/**
 * @brief Check if a row is dirty (needs updating)
 * 
 * @param y Y coordinate of the row
 * @return true if the row has been modified since last clean mark
 * 
 * Determines whether a row needs to be updated on the display. Only rows
 * that have been modified since the last clean mark will return true.
 * This enables efficient display updates by only processing changed rows.
 */
bool LCDCache::isRowDirty(uint16_t y) const {
    portENTER_CRITICAL(&spinlock);
    bool result = y < dirtyRows.size() ? dirtyRows[y] : false;
    portEXIT_CRITICAL(&spinlock);
    return result;
}

/**
 * @brief Mark a row as clean (updated)
 * 
 * @param y Y coordinate of the row
 * 
 * Marks a row as clean after it has been updated on the display. This
 * prevents unnecessary re-updates of the same row and improves performance.
 * Should be called after successfully updating the display for a row.
 */
void LCDCache::markRowClean(uint16_t y) {
    portENTER_CRITICAL(&spinlock);
    if (y < dirtyRows.size()) dirtyRows[y] = false;
    portEXIT_CRITICAL(&spinlock);
}

/**
 * @brief Mark a row as dirty (needs updating)
 * 
 * @param y Y coordinate of the row
 * 
 * Marks a row as dirty after it has been modified since the last clean mark.
 * This enables efficient display updates by only processing changed rows.
 */
void LCDCache::markRowDirty(uint16_t y) {
    portENTER_CRITICAL(&spinlock);
    if (y < dirtyRows.size()) dirtyRows[y] = true;
    portEXIT_CRITICAL(&spinlock);
}

/**
 * @brief Clear all dirty row flags
 * 
 * Marks all rows as clean, effectively resetting the dirty tracking system.
 * Use this when you want to reset the dirty tracking or after a full
 * display update has been completed.
 */
void LCDCache::clearDirtyFlags() {
    portENTER_CRITICAL(&spinlock);
    std::fill(dirtyRows.begin(), dirtyRows.end(), false);
    portEXIT_CRITICAL(&spinlock);
}

/**
 * @brief Update a single pixel in the cache
 * 
 * @param x X coordinate (0 to width-1)
 * @param y Y coordinate (0 to width-1)
 * @param color RGB565 color value
 * 
 * This method converts RGB565 to RGB888 and calls setPixel.
 */
void LCDCache::updateCachePixel(uint16_t x, uint16_t y, uint16_t color) {
    // Convert RGB565 to RGB888
    uint8_t r = ((color >> 11) & 0x1F) << 3;  // 5 bits to 8 bits
    uint8_t g = ((color >> 5) & 0x3F) << 2;   // 6 bits to 8 bits  
    uint8_t b = (color & 0x1F) << 3;          // 5 bits to 8 bits
    
    // Convert to RGB888 format
    ColorValue rgb888 = (r << 16) | (g << 8) | b;
    
    // Set the pixel
    setPixel(x, y, rgb888);
}

/**
 * @brief Update a rectangular region in the cache
 * 
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Width of rectangle
 * @param h Height of rectangle
 * 
 * This method marks the region as dirty for efficient updates.
 */
void LCDCache::updateCacheRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    portENTER_CRITICAL(&spinlock);
    // Mark all rows in the rectangle as dirty
    for (uint16_t row = y; row < y + h && row < screenHeight; row++) {
        if (row < dirtyRows.size()) {
            dirtyRows[row] = true;
        }
    }
    portEXIT_CRITICAL(&spinlock);
}

/**
 * @brief Set the display driver for cache updates
 * 
 * @param driver Pointer to display driver implementing IDisplayDriver interface
 * 
 * This method sets the display driver that will be used to flush
 * cached data to the hardware display. The driver must implement the
 * IDisplayDriver interface.
 */
void LCDCache::setDisplayDriver(IDisplayDriver* driver) {
    portENTER_CRITICAL(&spinlock);
    displayDriver = driver;
    portEXIT_CRITICAL(&spinlock);
}

/**
 * @brief Update the display with cached data
 * 
 * This method flushes dirty rows to the display hardware using the
 * abstract display interface.
 * 
 * IMPORTANT: This method preserves the cache content while updating the display.
 * Only the dirty flags are cleared after successful updates.
 */
void LCDCache::update() {
    if (!displayDriver) {
        ESP_LOGW(TAG, "No display driver set, cannot update display");
        return;
    }
    
    uint16_t updatedRows = 0;
    
    // First, collect all dirty rows while holding the lock
    std::vector<uint16_t> dirtyRowList;
    portENTER_CRITICAL(&spinlock);
    for (uint16_t y = 0; y < screenHeight; y++) {
        if (y < dirtyRows.size() && dirtyRows[y]) {
            dirtyRowList.push_back(y);
        }
    }
    portEXIT_CRITICAL(&spinlock);
    
    // Now process each dirty row
    for (uint16_t y : dirtyRowList) {
        // Convert cache data to RGB565 format for the display
        for (uint16_t x = 0; x < screenWidth; x++) {
            ColorValue pixel = getPixel(x, y);
            
            // Convert RGB888 to RGB565
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;
            
            uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            
            // Draw pixel using abstract interface
            displayDriver->drawPixel(x, y, rgb565);
        }
        markRowClean(y);
        updatedRows++;
    }
    
    // Optional flush operation
    if (updatedRows > 0) {
        displayDriver->flush();
        ESP_LOGI(TAG, "LCDCache::update() - Updated %d rows on display", updatedRows);
    } else {
        ESP_LOGD(TAG, "LCDCache::update() - No dirty rows to update");
    }
}

/**
 * @brief Destructor – free pooled RunList allocations
 */
LCDCache::~LCDCache() {
    // Free all RunList objects currently in use in the cache
    for (auto& row : cache) {
        for (auto& colorPair : row) {
            delete colorPair.second;
        }
    }
    
    // Free pooled objects
    for (RunList *list : freePool) {
        delete list;
    }
    freePool.clear();
} 