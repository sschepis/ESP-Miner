#include "unity.h"
#include "nonce_generator.h"

TEST_CASE("Check nonce generator initialization", "[nonce_generator]")
{
    esp_err_t ret = nonce_generator_init(NONCE_GEN_SEQUENTIAL);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    const nonce_gen_config_t *config = nonce_generator_get_config();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(NONCE_GEN_SEQUENTIAL, config->strategy);
    TEST_ASSERT_EQUAL(false, config->enable_learning);
}

TEST_CASE("Check sequential mode always returns 0", "[nonce_generator]")
{
    nonce_generator_init(NONCE_GEN_SEQUENTIAL);
    
    uint32_t nonce1 = nonce_generator_get_starting_nonce(10, 0);
    uint32_t nonce2 = nonce_generator_get_starting_nonce(10, 1);
    uint32_t nonce3 = nonce_generator_get_starting_nonce(10, 2);
    
    TEST_ASSERT_EQUAL_UINT32(0, nonce1);
    TEST_ASSERT_EQUAL_UINT32(0, nonce2);
    TEST_ASSERT_EQUAL_UINT32(0, nonce3);
}

TEST_CASE("Check prime skew mode generates different nonces", "[nonce_generator]")
{
    nonce_generator_init(NONCE_GEN_PRIME_SKEW);
    
    uint32_t nonce1 = nonce_generator_get_starting_nonce(10, 0);
    uint32_t nonce2 = nonce_generator_get_starting_nonce(10, 1);
    uint32_t nonce3 = nonce_generator_get_starting_nonce(10, 2);
    
    // Should be different from each other
    TEST_ASSERT_NOT_EQUAL(nonce1, nonce2);
    TEST_ASSERT_NOT_EQUAL(nonce2, nonce3);
    TEST_ASSERT_NOT_EQUAL(nonce1, nonce3);
    
    // First nonce should be 0 (0 * 2 * 10 = 0)
    TEST_ASSERT_EQUAL_UINT32(0, nonce1);
}

TEST_CASE("Check golden ratio mode generates different nonces", "[nonce_generator]")
{
    nonce_generator_init(NONCE_GEN_GOLDEN_RATIO);
    
    uint32_t nonce1 = nonce_generator_get_starting_nonce(10, 0);
    uint32_t nonce2 = nonce_generator_get_starting_nonce(10, 1);
    uint32_t nonce3 = nonce_generator_get_starting_nonce(10, 2);
    
    // Should be different from each other
    TEST_ASSERT_NOT_EQUAL(nonce1, nonce2);
    TEST_ASSERT_NOT_EQUAL(nonce2, nonce3);
    TEST_ASSERT_NOT_EQUAL(nonce1, nonce3);
    
    // First nonce should be 0 (0 * ... = 0)
    TEST_ASSERT_EQUAL_UINT32(0, nonce1);
}

TEST_CASE("Check pattern adaptive mode falls back to prime skew", "[nonce_generator]")
{
    nonce_generator_init(NONCE_GEN_PATTERN_ADAPTIVE);
    
    const nonce_gen_config_t *config = nonce_generator_get_config();
    TEST_ASSERT_EQUAL(true, config->enable_learning);
    TEST_ASSERT_EQUAL(0, config->history_count);
    
    // Should fall back to prime skew when no patterns learned
    uint32_t nonce1 = nonce_generator_get_starting_nonce(10, 0);
    uint32_t nonce2 = nonce_generator_get_starting_nonce(10, 1);
    
    TEST_ASSERT_NOT_EQUAL(nonce1, nonce2);
}

TEST_CASE("Check pattern learning stores successful nonces", "[nonce_generator]")
{
    nonce_generator_init(NONCE_GEN_PATTERN_ADAPTIVE);
    
    nonce_generator_report_success(0x12345678);
    nonce_generator_report_success(0x9ABCDEF0);
    
    const nonce_gen_config_t *config = nonce_generator_get_config();
    TEST_ASSERT_EQUAL(2, config->history_count);
    TEST_ASSERT_EQUAL_UINT32(0x12345678, config->pattern_history[0]);
    TEST_ASSERT_EQUAL_UINT32(0x9ABCDEF0, config->pattern_history[1]);
}

TEST_CASE("Check pattern analysis with all zeros", "[nonce_generator]")
{
    nonce_pattern_t pattern;
    
    nonce_generator_analyze_pattern(0x00000000, &pattern);
    TEST_ASSERT_EQUAL_UINT8(32, pattern.leading_zeros);
    TEST_ASSERT_EQUAL_UINT8(32, pattern.trailing_zeros);
    TEST_ASSERT_EQUAL_UINT8(0, pattern.one_count);
    TEST_ASSERT_EQUAL_UINT8(0, pattern.transitions);
}

TEST_CASE("Check pattern analysis with single bit set", "[nonce_generator]")
{
    nonce_pattern_t pattern;
    
    // Test with 0x00000001 (31 leading zeros, 0 trailing)
    nonce_generator_analyze_pattern(0x00000001, &pattern);
    TEST_ASSERT_EQUAL_UINT8(31, pattern.leading_zeros);
    TEST_ASSERT_EQUAL_UINT8(0, pattern.trailing_zeros);
    TEST_ASSERT_EQUAL_UINT8(1, pattern.one_count);
    
    // Test with 0x80000000 (0 leading zeros, 31 trailing)
    nonce_generator_analyze_pattern(0x80000000, &pattern);
    TEST_ASSERT_EQUAL_UINT8(0, pattern.leading_zeros);
    TEST_ASSERT_EQUAL_UINT8(31, pattern.trailing_zeros);
    TEST_ASSERT_EQUAL_UINT8(1, pattern.one_count);
}

TEST_CASE("Check pattern analysis with alternating bits", "[nonce_generator]")
{
    nonce_pattern_t pattern;
    
    // 0xAAAAAAAA = 10101010101010101010101010101010
    nonce_generator_analyze_pattern(0xAAAAAAAA, &pattern);
    TEST_ASSERT_EQUAL_UINT8(0, pattern.leading_zeros);
    TEST_ASSERT_EQUAL_UINT8(0, pattern.trailing_zeros);
    TEST_ASSERT_EQUAL_UINT8(16, pattern.one_count);
    TEST_ASSERT_EQUAL_UINT8(31, pattern.transitions); // 31 transitions in alternating pattern
}

TEST_CASE("Check strategy switching", "[nonce_generator]")
{
    nonce_generator_init(NONCE_GEN_SEQUENTIAL);
    
    const nonce_gen_config_t *config = nonce_generator_get_config();
    TEST_ASSERT_EQUAL(NONCE_GEN_SEQUENTIAL, config->strategy);
    
    nonce_generator_set_strategy(NONCE_GEN_PRIME_SKEW);
    config = nonce_generator_get_config();
    TEST_ASSERT_EQUAL(NONCE_GEN_PRIME_SKEW, config->strategy);
    
    nonce_generator_set_strategy(NONCE_GEN_PATTERN_ADAPTIVE);
    config = nonce_generator_get_config();
    TEST_ASSERT_EQUAL(NONCE_GEN_PATTERN_ADAPTIVE, config->strategy);
    TEST_ASSERT_EQUAL(true, config->enable_learning);
}

TEST_CASE("Check pattern history circular buffer", "[nonce_generator]")
{
    nonce_generator_init(NONCE_GEN_PATTERN_ADAPTIVE);
    
    // Add 12 nonces (more than buffer size of 10)
    for (uint32_t i = 0; i < 12; i++) {
        nonce_generator_report_success(i * 0x11111111);
    }
    
    const nonce_gen_config_t *config = nonce_generator_get_config();
    
    // Should have max 10 entries
    TEST_ASSERT_EQUAL(10, config->history_count);
    
    // Should have wrapped around, so first entries are now 2 and 3
    TEST_ASSERT_EQUAL_UINT32(10 * 0x11111111, config->pattern_history[0]);
    TEST_ASSERT_EQUAL_UINT32(11 * 0x11111111, config->pattern_history[1]);
}

TEST_CASE("Check clearing pattern history on strategy change", "[nonce_generator]")
{
    nonce_generator_init(NONCE_GEN_PATTERN_ADAPTIVE);
    
    // Add some patterns
    nonce_generator_report_success(0x12345678);
    nonce_generator_report_success(0x9ABCDEF0);
    
    const nonce_gen_config_t *config = nonce_generator_get_config();
    TEST_ASSERT_EQUAL(2, config->history_count);
    
    // Switch to non-learning strategy
    nonce_generator_set_strategy(NONCE_GEN_PRIME_SKEW);
    
    config = nonce_generator_get_config();
    TEST_ASSERT_EQUAL(0, config->history_count);
    TEST_ASSERT_EQUAL(false, config->enable_learning);
}
