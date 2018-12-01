/**
 * \file  hspi_config.h
 * \brief SD Card Device Descriptor
 */
#ifndef __HSPI_DEV_H
#define __HSPI_DEV_H

#include <esp/spi_regs.h>

/**
 * \brief SD card SPI descriptor
 */
typedef struct {
    uintptr_t sdhc : 1;
} hspi_dev_t;

typedef hspi_dev_t sdcard_t;

#define MAKE_CLOCK(div,cnt) \
    ( VAL2FIELD(SPI_CLOCK_DIV_PRE, (div)-1) \
    | VAL2FIELD(SPI_CLOCK_COUNT_NUM, (cnt)-1) \
    | VAL2FIELD(SPI_CLOCK_COUNT_HIGH, (cnt) / 2) \
    )

/**
 * \brief SPI clock settings.
 * \param dev Slave device descriptor
 * \return a value that #hspi_select will write into the CLOCK register
 */
static inline uint32_t hspi_dev_clock(hspi_dev_t dev)
{
    return MAKE_CLOCK(1,4);
}

/**
 * \brief SPI transfer mode
 * \param dev Slave device descriptor
 * \return 0..3
 *   0 - clock idle state is low, data are captured on the clock's leading edge (low-to-high transition)
 *   1 - clock idle state is low, data are captured on the clock's trailing edge (high-to-low transition)
 *   2 - clock idle state is high, data are captured on the clock's leading edge (high-to-low transition)
 *   3 - clock idle state is high, data are captured on the clock's trailing edge (low-to-high transition)
 * \see for example, http://dlnware.com/theory/SPI-Transfer-Modes for details
 */
static inline uint32_t hspi_dev_transfer_mode(hspi_dev_t dev)
{
    return 0;
}

/**
 * \brief Bit order used by the slave device
 * \param dev Slave device descriptor
 * \return `true` if device expects MSB bit order and `false` for an LSB device.
 *
 * \note While ESP SPI allows different bit order settings for input and output
 *       the driver configures them to be the same. This might change one day
 *       if there is a use case for it.
 */
static inline bool hspi_dev_is_msb(hspi_dev_t dev)
{
    // use MSB bit order
    return true;
}

/**
 * \brief Hardware vs software CS0 pin control
 * \param dev Slave device descriptor
 * \return `true` if the application will control CS0 pin via software
 */
static inline bool hspi_dev_software_cs(hspi_dev_t dev)
{
    return true;
}

/**
 * \brief Whether device uses the same pin for both input and output
 * \param dev Slave device descriptor
 * \return `true` if device has only one pin that used for both input and putput.
 *
 * \see ST7735 for an example
 */
static inline bool hspi_dev_shared_io(hspi_dev_t dev)
{
    return false;
}

/**
 * \brief Saves "SD card is an SDHC card" flag in the card descriptor
 * \param card Pointer to the card descriptor.
 */
static inline void sdcard_set_sdhc_flag(sdcard_t * card, bool is_sdhc)
{
    card->sdhc = is_sdhc ? 1 : 0;
}

/**
 * \brief Queries whether the card behind the provided descriptor is an SDHC one
 * \param card Card descriptor
 * \return `true` if SD card and an SDHC card.
 */
static inline bool sdcard_is_sdhc(sdcard_t card)
{
    return card.sdhc;
}

#endif