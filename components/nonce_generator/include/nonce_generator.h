#ifndef NONCE_GENERATOR_H_
#define NONCE_GENERATOR_H_

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Configuration options
typedef enum {
    NONCE_GEN_SEQUENTIAL = 0,    // Traditional: always start at 0
    NONCE_GEN_PRIME_SKEW,         // Prime number based distribution
    NONCE_GEN_GOLDEN_RATIO,       // Golden ratio partitioning
    NONCE_GEN_PATTERN_ADAPTIVE    // Pattern learning with adaptation
} nonce_gen_strategy_t;

// Pattern analysis structure
typedef struct {
    uint8_t leading_zeros;
    uint8_t trailing_zeros;
    uint8_t one_count;
    uint8_t transitions;
} nonce_pattern_t;

// Configuration structure
typedef struct {
    nonce_gen_strategy_t strategy;
    bool enable_learning;
    uint32_t job_counter;
    
    // For pattern adaptive mode
    uint32_t pattern_history[10];  // Last 10 successful nonces
    uint8_t history_index;
    uint8_t history_count;
} nonce_gen_config_t;

/**
 * Initialize the nonce generator
 * @param strategy The generation strategy to use
 * @return ESP_OK on success
 */
esp_err_t nonce_generator_init(nonce_gen_strategy_t strategy);

/**
 * Generate starting nonce for next job
 * @param difficulty Current mining difficulty
 * @param job_id Current job identifier
 * @return Starting nonce value (0 to 0xFFFFFFFF)
 */
uint32_t nonce_generator_get_starting_nonce(uint32_t difficulty, uint8_t job_id);

/**
 * Report successful nonce for pattern learning
 * @param nonce The successful nonce value
 */
void nonce_generator_report_success(uint32_t nonce);

/**
 * Analyze nonce binary pattern
 * @param nonce Nonce value to analyze
 * @param pattern Output pattern structure
 */
void nonce_generator_analyze_pattern(uint32_t nonce, nonce_pattern_t *pattern);

/**
 * Get current configuration
 * @return Pointer to current configuration
 */
const nonce_gen_config_t* nonce_generator_get_config(void);

/**
 * Set generation strategy
 * @param strategy New strategy to use
 */
void nonce_generator_set_strategy(nonce_gen_strategy_t strategy);

#endif /* NONCE_GENERATOR_H_ */
