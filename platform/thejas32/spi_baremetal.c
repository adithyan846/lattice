#include "hal_spi.h"
#include "hal_errno.h"
#include "stdlib.h"
#include "spi.h"
#include "config.h"
#include "gpio.h"

#define THEJAS32_SPI_CLK_HZ  100000000UL

static uint8_t hal_bus_to_vega[HAL_SPI_BUS_COUNT] = {0, 1, 2, 3};
static const uint32_t baud_dividers[] = {4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};

static hal_spi_config_t g_bus_config[HAL_SPI_BUS_COUNT] = {0};
static bool g_bus_initialized[HAL_SPI_BUS_COUNT] = {false};

static void build_control_reg(const hal_spi_config_t *cfg, SPIcntrlRegType *ctrl) {
    ctrl->Bits.PeriphChipSelect = 0;
    ctrl->Bits.PeriphType = 0;
    ctrl->Bits.DataOrder = (cfg->bit_order == HAL_SPI_BIT_ORDER_LSB) ? 1 : 0;
    ctrl->Bits.Mode = cfg->mode;
    ctrl->Bits.SPIrxTxIntr = RX_TX_INTR_DIS;
    ctrl->Bits.CSAAT = 0;
    ctrl->Bits.Dbits = (cfg->word_size == 16) ? 1 : 0;
    ctrl->Bits.Reserved = 0;
}

static uint8_t hz_to_divider_index(uint32_t freq_hz) {
    if (freq_hz == 0) return 2;
    uint32_t target_div = THEJAS32_SPI_CLK_HZ / freq_hz;
    uint8_t best_idx = 0;
    uint32_t best_diff = UINT32_MAX;
    for (uint8_t i = 0; i < 10; i++) {
        uint32_t diff = (baud_dividers[i] > target_div)
                        ? baud_dividers[i] - target_div
                        : target_div - baud_dividers[i];
        if (diff < best_diff) {
            best_diff = diff;
            best_idx = i;
        }
    }
    return best_idx;
}

static uint8_t baud_div_to_index(hal_spi_baud_div_t div) {
    if (div > HAL_SPI_BAUD_DIV_2048) return 2;
    return (uint8_t)div;
}

static inline int spi_wait_status(uint8_t vega_spi, uint32_t mask, int want_set, uint32_t timeout_ms) {
    // Simple calibrated loop counter for bare-metal timeout
    uint32_t max_loops = timeout_ms ? (timeout_ms * 50000) : 0xFFFFFFFF;
    while (max_loops--) {
        uint32_t status = SPIreg(vega_spi).Status;
        if (((status & mask) != 0U) == (want_set != 0)) {
            return 0;
        }
    }
    return -ETIMEDOUT;
}

static inline int spi_assert_cs(uint8_t vega_spi, const hal_spi_config_t *cfg) {
    if (cfg->cs_mode == HAL_SPI_CS_MODE_CSAAT) {
        SPI_set_CSAAT_pin(vega_spi, HIGH);
    } else if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO) {
        GPIO_write_pin(cfg->gpio_pin, 0);
    }
    return 0;
}

static inline int spi_deassert_cs(uint8_t vega_spi, const hal_spi_config_t *cfg) {
    if (cfg->cs_mode == HAL_SPI_CS_MODE_CSAAT) {
        SPI_set_CSAAT_pin(vega_spi, LOW);
    } else if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO) {
        GPIO_write_pin(cfg->gpio_pin, 1);
    }
    return 0;
}

int hal_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *config) {
    if (bus >= HAL_SPI_BUS_COUNT) return -EINVAL;
    if (!config) return -EINVAL;
    if (config->word_size != 8) return -ENOTSUP;
    if (config->cs_mode == HAL_SPI_CS_MODE_GPIO && config->gpio_pin > 31) return -EINVAL;
    if (config->cs_mode == HAL_SPI_CS_MODE_CSAAT && config->hw_cs > HAL_SPI_CS_HW_3) return -EINVAL;

    uint8_t vega_spi = hal_bus_to_vega[bus];

    SPI_init(vega_spi);

    SPIcntrlRegType ctrl;
    build_control_reg(config, &ctrl);
    SPI_config(vega_spi, ctrl);

    uint8_t div_idx;
    if (config->freq_hz != 0) {
        div_idx = hz_to_divider_index(config->freq_hz);
    } else {
        div_idx = baud_div_to_index(config->baud_div);
    }
    SPI_set_baud(vega_spi, div_idx);

    if (config->cs_mode == HAL_SPI_CS_MODE_GPIO) {
        GPIO_write_pin(config->gpio_pin, 1);
    } else {
        SPI_set_CSAAT_pin(vega_spi, LOW);
    }

    g_bus_config[bus] = *config;
    g_bus_initialized[bus] = true;

    return 0;
}

int hal_spi_deinit(hal_spi_bus_t bus) {
    if (bus >= HAL_SPI_BUS_COUNT) return -EINVAL;
    if (!g_bus_initialized[bus]) return 0;

    uint8_t vega_spi = hal_bus_to_vega[bus];

    SPI_init(vega_spi);

    if (g_bus_config[bus].cs_mode == HAL_SPI_CS_MODE_GPIO) {
        GPIO_write_pin(g_bus_config[bus].gpio_pin, 1);
    }

    g_bus_initialized[bus] = false;
    return 0;
}

int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;
    if (!tx && !rx) return -EINVAL;
    if (len == 0) return 0;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    const hal_spi_config_t *cfg = &g_bus_config[bus];

    spi_assert_cs(vega_spi, cfg);

    for (uint32_t i = 0; i < len; i++) {
        uint8_t tx_byte = tx ? tx[i] : 0x00;
        uint8_t rx_byte = 0;
        int ret;

        ret = spi_wait_status(vega_spi, SPI_BUSY_BIT, 0, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        ret = spi_wait_status(vega_spi, SPI_TX_HOLD_EMPTY_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        SPIreg(vega_spi).TxData = tx_byte;

        ret = spi_wait_status(vega_spi, SPI_RX_COMPLETE_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        rx_byte = SPIreg(vega_spi).RxData & 0xFF;
        if (rx) rx[i] = rx_byte;
    }

    spi_deassert_cs(vega_spi, cfg);
    return 0;
}

int hal_spi_write_read(hal_spi_bus_t bus, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len, uint32_t timeout_ms) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;
    if (!tx && tx_len > 0) return -EINVAL;
    if (!rx && rx_len > 0) return -EINVAL;
    if (tx_len == 0 && rx_len == 0) return 0;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    const hal_spi_config_t *cfg = &g_bus_config[bus];
    int ret = 0;

    spi_assert_cs(vega_spi, cfg);

    for (uint32_t i = 0; i < tx_len; i++) {
        ret = spi_wait_status(vega_spi, SPI_BUSY_BIT, 0, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        ret = spi_wait_status(vega_spi, SPI_TX_HOLD_EMPTY_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        SPIreg(vega_spi).TxData = tx[i];

        ret = spi_wait_status(vega_spi, SPI_RX_COMPLETE_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        (void)SPIreg(vega_spi).RxData;
    }

    for (uint32_t i = 0; i < rx_len; i++) {
        ret = spi_wait_status(vega_spi, SPI_BUSY_BIT, 0, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        ret = spi_wait_status(vega_spi, SPI_TX_HOLD_EMPTY_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        SPIreg(vega_spi).TxData = 0xFF;

        ret = spi_wait_status(vega_spi, SPI_RX_COMPLETE_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        rx[i] = SPIreg(vega_spi).RxData & 0xFF;
    }

    spi_deassert_cs(vega_spi, cfg);
    return ret;
}

int hal_spi_write(hal_spi_bus_t bus, const uint8_t *data, uint32_t len, uint32_t timeout_ms) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;
    if (!data) return -EINVAL;
    if (len == 0) return 0;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    const hal_spi_config_t *cfg = &g_bus_config[bus];
    int ret = 0;

    spi_assert_cs(vega_spi, cfg);

    for (uint32_t i = 0; i < len; i++) {
        ret = spi_wait_status(vega_spi, SPI_BUSY_BIT, 0, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        ret = spi_wait_status(vega_spi, SPI_TX_HOLD_EMPTY_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        SPIreg(vega_spi).TxData = data[i];

        ret = spi_wait_status(vega_spi, SPI_RX_COMPLETE_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        (void)SPIreg(vega_spi).RxData;
    }

    spi_deassert_cs(vega_spi, cfg);
    return ret;
}

int hal_spi_read(hal_spi_bus_t bus, uint8_t *data, uint32_t len, uint32_t timeout_ms) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;
    if (!data) return -EINVAL;
    if (len == 0) return 0;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    const hal_spi_config_t *cfg = &g_bus_config[bus];
    int ret = 0;

    spi_assert_cs(vega_spi, cfg);

    for (uint32_t i = 0; i < len; i++) {
        ret = spi_wait_status(vega_spi, SPI_BUSY_BIT, 0, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        ret = spi_wait_status(vega_spi, SPI_TX_HOLD_EMPTY_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        SPIreg(vega_spi).TxData = 0x00;

        ret = spi_wait_status(vega_spi, SPI_RX_COMPLETE_BIT, 1, timeout_ms);
        if (ret != 0) { spi_deassert_cs(vega_spi, cfg); return ret; }

        data[i] = SPIreg(vega_spi).RxData & 0xFF;
    }

    spi_deassert_cs(vega_spi, cfg);
    return ret;
}

int hal_spi_set_cs(hal_spi_bus_t bus, bool assert) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    const hal_spi_config_t *cfg = &g_bus_config[bus];

    if (cfg->cs_mode == HAL_SPI_CS_MODE_CSAAT) {
        SPI_set_CSAAT_pin(vega_spi, assert ? HIGH : LOW);
    } else if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO) {
        GPIO_write_pin(cfg->gpio_pin, assert ? 0 : 1);
    }

    return 0;
}

int hal_spi_set_frequency(hal_spi_bus_t bus, uint32_t freq_hz) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;
    if (freq_hz == 0 || freq_hz > THEJAS32_SPI_CLK_HZ / 4) return -EINVAL;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    uint8_t div_idx = hz_to_divider_index(freq_hz);
    SPI_set_baud(vega_spi, div_idx);

    g_bus_config[bus].freq_hz = freq_hz;
    return 0;
}

int hal_spi_set_baud_div(hal_spi_bus_t bus, hal_spi_baud_div_t div) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    uint8_t div_idx = baud_div_to_index(div);
    SPI_set_baud(vega_spi, div_idx);

    g_bus_config[bus].freq_hz = 0;
    g_bus_config[bus].baud_div = div;
    return 0;
}

int hal_spi_set_mode(hal_spi_bus_t bus, hal_spi_mode_t mode) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;
    if (mode > HAL_SPI_MODE_3) return -EINVAL;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    US cword = SPIreg(vega_spi).Control.hword;
    cword &= ~(0x3 << 4);
    cword |= (mode << 4);
    SPIreg(vega_spi).Control.hword = cword;
    __asm__ __volatile__ ("fence");

    g_bus_config[bus].mode = mode;
    return 0;
}

int hal_spi_set_bit_order(hal_spi_bus_t bus, hal_spi_bit_order_t order) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;

    uint8_t vega_spi = hal_bus_to_vega[bus];
    US cword = SPIreg(vega_spi).Control.hword;
    cword &= ~(1 << 3);
    cword |= ((order == HAL_SPI_BIT_ORDER_LSB) ? 1 : 0) << 3;
    SPIreg(vega_spi).Control.hword = cword;
    __asm__ __volatile__ ("fence");

    g_bus_config[bus].bit_order = order;
    return 0;
}

int hal_spi_check_overrun(hal_spi_bus_t bus) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return -ENODEV;
    return SPI_check_overrun(hal_bus_to_vega[bus]);
}

bool hal_spi_is_busy(hal_spi_bus_t bus) {
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return false;
    return (SPIreg(hal_bus_to_vega[bus]).Status & SPI_BUSY_BIT) != 0;
}