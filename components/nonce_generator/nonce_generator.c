#include "nonce_generator.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "nonce_gen";

// Prime numbers for skewing
static const uint32_t PRIMES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97
};
#define NUM_PRIMES (sizeof(PRIMES) / sizeof(PRIMES[0]))

// Golden ratio constant (approximated as integer fraction)
// PHI = 1.618... ≈ 1618/1000
#define PHI_NUMERATOR 1618
#define PHI_DENOMINATOR 1000

// Maximum 32-bit value
#define MAX_UINT32 0xFFFFFFFF

// Configuration state
static nonce_gen_config_t config = {
    .strategy = NONCE_GEN_SEQUENTIAL,
    .enable_learning = false,
    .job_counter = 0,
    .history_index = 0,
    .history_count = 0
};

esp_err_t nonce_generator_init(nonce_gen_strategy_t strategy)
{
    ESP_LOGI(TAG, "Initializing nonce generator with strategy: %d", strategy);
    
    config.strategy = strategy;
    config.job_counter = 0;
    config.history_index = 0;
    config.history_count = 0;
    config.enable_learning = (strategy == NONCE_GEN_PATTERN_ADAPTIVE);
    
    memset(config.pattern_history, 0, sizeof(config.pattern_history));
    
    ESP_LOGI(TAG, "Nonce generator initialized successfully");
    return ESP_OK;
}

uint32_t nonce_generator_get_starting_nonce(uint32_t difficulty, uint8_t job_id)
{
    uint32_t starting_nonce = 0;
    
    switch (config.strategy) {
        case NONCE_GEN_SEQUENTIAL:
            // Traditional: always 0
            starting_nonce = 0;
            break;
            
        case NONCE_GEN_PRIME_SKEW:
            // Use prime number rotation
            {
                uint32_t prime_index = config.job_counter % NUM_PRIMES;
                uint32_t prime = PRIMES[prime_index];
                
                // Skew based on prime and difficulty
                // Formula: (job_counter * prime * difficulty_factor) & MAX
                uint32_t difficulty_factor = (difficulty > 6) ? difficulty : 1;
                starting_nonce = (config.job_counter * prime * difficulty_factor) & MAX_UINT32;
                
                ESP_LOGD(TAG, "Prime skew: job=%lu, prime=%lu, nonce=0x%08lX", 
                         config.job_counter, prime, starting_nonce);
            }
            break;
            
        case NONCE_GEN_GOLDEN_RATIO:
            // Partition nonce space using golden ratio
            {
                // Calculate position in golden ratio sequence
                // nonce = (job_counter * MAX_UINT32 * PHI_NUM / PHI_DEN) mod MAX_UINT32
                uint64_t temp = (uint64_t)config.job_counter * (uint64_t)MAX_UINT32;
                temp = (temp * (uint64_t)PHI_NUMERATOR) / (uint64_t)PHI_DENOMINATOR;
                starting_nonce = (uint32_t)(temp & MAX_UINT32);
                
                ESP_LOGD(TAG, "Golden ratio: job=%lu, nonce=0x%08lX", 
                         config.job_counter, starting_nonce);
            }
            break;
            
        case NONCE_GEN_PATTERN_ADAPTIVE:
            // Use learned patterns
            if (config.history_count > 0) {
                // Calculate average pattern from history
                uint64_t sum = 0;
                for (int i = 0; i < config.history_count && i < 10; i++) {
                    sum += config.pattern_history[i];
                }
                uint32_t avg_pattern = (uint32_t)(sum / config.history_count);
                
                // Use prime skew but influenced by learned pattern
                uint32_t prime_index = config.job_counter % NUM_PRIMES;
                uint32_t prime = PRIMES[prime_index];
                
                // Combine pattern learning with prime distribution
                starting_nonce = ((avg_pattern >> 8) + (config.job_counter * prime)) & MAX_UINT32;
                
                ESP_LOGD(TAG, "Pattern adaptive: job=%lu, avg_pattern=0x%08lX, nonce=0x%08lX",
                         config.job_counter, avg_pattern, starting_nonce);
            } else {
                // Fall back to prime skew until patterns learned
                uint32_t prime_index = config.job_counter % NUM_PRIMES;
                starting_nonce = (config.job_counter * PRIMES[prime_index]) & MAX_UINT32;
            }
            break;
            
        default:
            starting_nonce = 0;
            break;
    }
    
    config.job_counter++;
    return starting_nonce;
}

void nonce_generator_report_success(uint32_t nonce)
{
    if (!config.enable_learning) {
        return;
    }
    
    ESP_LOGI(TAG, "Successful nonce reported: 0x%08lX", nonce);
    
    // Store in circular buffer
    config.pattern_history[config.history_index] = nonce;
    config.history_index = (config.history_index + 1) % 10;
    
    if (config.history_count < 10) {
        config.history_count++;
    }
}

void nonce_generator_analyze_pattern(uint32_t nonce, nonce_pattern_t *pattern)
{
    if (pattern == NULL) {
        return;
    }
    
    // Count leading zeros
    pattern->leading_zeros = 0;
    for (int i = 31; i >= 0; i--) {
        if (nonce & (1U << i)) {
            break;
        }
        pattern->leading_zeros++;
    }
    
    // Count trailing zeros
    pattern->trailing_zeros = 0;
    for (int i = 0; i < 32; i++) {
        if (nonce & (1U << i)) {
            break;
        }
        pattern->trailing_zeros++;
    }
    
    // Count ones
    pattern->one_count = 0;
    for (int i = 0; i < 32; i++) {
        if (nonce & (1U << i)) {
            pattern->one_count++;
        }
    }
    
    // Count transitions
    pattern->transitions = 0;
    uint8_t prev_bit = (nonce & 1);
    for (int i = 1; i < 32; i++) {
        uint8_t bit = (nonce >> i) & 1;
        if (bit != prev_bit) {
            pattern->transitions++;
        }
        prev_bit = bit;
    }
}

const nonce_gen_config_t* nonce_generator_get_config(void)
{
    return &config;
}

void nonce_generator_set_strategy(nonce_gen_strategy_t strategy)
{
    ESP_LOGI(TAG, "Changing strategy from %d to %d", config.strategy, strategy);
    config.strategy = strategy;
    config.enable_learning = (strategy == NONCE_GEN_PATTERN_ADAPTIVE);
    
    if (strategy != NONCE_GEN_PATTERN_ADAPTIVE) {
        // Clear learned patterns
        config.history_count = 0;
        config.history_index = 0;
        memset(config.pattern_history, 0, sizeof(config.pattern_history));
    }
}
