/**
 * Copyright (c) 2017 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdint.h>
#include <string.h>
#include "nrf.h"
#include "nordic_common.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "fds.h"
#include "app_timer.h"
#include "app_error.h"
//#include "fds_example.h"
#include "nrf_delay.h"
#include "nrf_assert.h"

#include "flash_interface.h"
#include "debug/DEBUG.h"

/*
#define NRF_LOG_MODULE_NAME flash
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
*/

// A tag identifying the SoftDevice BLE configuration.
#define APP_BLE_CONN_CFG_TAG    1


// Array to map FDS return values to strings.
char const * fds_err_str[] =
{
    "FDS_SUCCESS",
    "FDS_ERR_OPERATION_TIMEOUT",
    "FDS_ERR_NOT_INITIALIZED",
    "FDS_ERR_UNALIGNED_ADDR",
    "FDS_ERR_INVALID_ARG",
    "FDS_ERR_NULL_ARG",
    "FDS_ERR_NO_OPEN_RECORDS",
    "FDS_ERR_NO_SPACE_IN_FLASH",
    "FDS_ERR_NO_SPACE_IN_QUEUES",
    "FDS_ERR_RECORD_TOO_LARGE",
    "FDS_ERR_NOT_FOUND",
    "FDS_ERR_NO_PAGES",
    "FDS_ERR_USER_LIMIT_REACHED",
    "FDS_ERR_CRC_CHECK_FAILED",
    "FDS_ERR_BUSY",
    "FDS_ERR_INTERNAL",
};

// Array to map FDS events to strings.
static char const * fds_evt_str[] =
{
    "FDS_EVT_INIT",
    "FDS_EVT_WRITE",
    "FDS_EVT_UPDATE",
    "FDS_EVT_DEL_RECORD",
    "FDS_EVT_DEL_FILE",
    "FDS_EVT_GC",
};



//@brief   Function for initializing the SoftDevice and enabling the BLE stack.
static void ble_stack_init(void)
{
    ret_code_t rc;
    uint32_t   ram_start = 0;

    // Enable the SoftDevice.
    rc = nrf_sdh_enable_request();
    ERROR_CHECK(rc);

    rc = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    ERROR_CHECK(rc);

    rc = nrf_sdh_ble_enable(&ram_start);
    ERROR_CHECK(rc);

}



//@brief   Initialize the timer.
static void timer_init(void)
{
    ret_code_t err_code = app_timer_init();
    ERROR_CHECK(err_code);
}


//@brief   Initialize logging.
static void log_init(void)
{
  uint32_t err_code;

  err_code = NRF_LOG_INIT(NULL);
  ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();

  NRF_LOG_INFO("Logging:");
  NRF_LOG_INFO("");
}


//@brief   Sleep until an event is received.
static void power_manage(void)
{
    (void) sd_app_evt_wait();
}



void Flash_Record::read(uint16_t file_id, uint16_t key_id, void* p_data, uint32_t len)
{
  fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};

  uint32_t rc = fds_record_find(file_id, key_id, &desc, &tok);

  if (rc == FDS_SUCCESS) {
    // A config file is in flash. Let's update it.
    fds_flash_record_t record = {0};

    // Open the record and read its contents.
    uint32_t rc = fds_record_open(&desc, &record);
    ERROR_CHECK(rc);

    // Copy the configuration from flash into m_dummy_cfg.
    memcpy(p_data, record.p_data, len);

    INFO("Reading record");
    // Close the record when done reading.
    rc = fds_record_close(&desc);
    ERROR_CHECK(rc);

  }
  else { INFO("Data not written yet!"); }

}


bool Flash_Record::doesExist(uint16_t file_id, uint16_t key_id)
{
  fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};

  uint32_t rc = fds_record_find(file_id, key_id, &desc, &tok);

  return (rc == FDS_SUCCESS) ? true : false;
}


void Flash_Record::write(uint16_t file_id, uint16_t key_id, void* p_data, uint32_t len)
{
  fds_record_t record = {0};
  record.file_id           = file_id;
  record.key               = key_id;
  record.data.p_data       = p_data;
  // The length of a record is always expressed in 4-byte units (words).
  record.data.length_words = (len + 3) / sizeof(uint32_t);

  fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};
  uint32_t rc = fds_record_find(file_id, key_id, &desc, &tok);

  if (rc == FDS_SUCCESS)
  {
    INFO("Updating record - ID: %d", desc.record_id);
    rc = fds_record_update(&desc, &record);
    ERROR_CHECK(rc);
    FDS::waitForUpdate();
  }
  else
  {
    INFO("Writing record - ID: %d", desc.record_id);
    rc = fds_record_write(&desc, &record);
    ERROR_CHECK(rc);
    FDS::waitForWrite();
  }
}


// Keep track of the progress of a delete_all operation.
static struct
{
    bool delete_next;   //!< Delete next record.
    bool pending;       //!< Waiting for an fds FDS_EVT_DEL_RECORD event, to delete the next record.
} m_delete_all;

// Flag to check fds initialization.
bool volatile FDS::m_initialised = false;
bool volatile FDS::m_writeEvent = false;
bool volatile FDS::m_updateEvent = false;



//@brief   Process a delete all command. Delete records, one by one, until no records are left.

void delete_all_process(void)
{
    if (   m_delete_all.delete_next
        & !m_delete_all.pending)
    {
        INFO("Deleting next record.");

        //m_delete_all.delete_next = record_delete_next();
        if (!m_delete_all.delete_next)
        {
            INFO("No records left to delete.");
        }
    }
}

void FDS::event_handler(fds_evt_t const * p_evt)
{
    INFO("Event: %s received (%s)",
                  fds_evt_str[p_evt->id],
                  fds_err_str[p_evt->result]);

    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_evt->result == FDS_SUCCESS)
            {
                m_initialised = true;
            }
            break;

        case FDS_EVT_WRITE:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                INFO("Record ID:\t0x%04x",  p_evt->write.record_id);
                INFO("File ID:\t0x%04x",    p_evt->write.file_id);
                INFO("Record key:\t0x%04x", p_evt->write.record_key);
                m_writeEvent = true;
            }
        } break;

        case FDS_EVT_UPDATE:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                INFO("Record ID:\t0x%04x",  p_evt->write.record_id);
                INFO("File ID:\t0x%04x",    p_evt->write.file_id);
                INFO("Record key:\t0x%04x", p_evt->write.record_key);
                m_updateEvent = true;
            }
        } break;

        case FDS_EVT_DEL_RECORD:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                INFO("Record ID:\t0x%04x",  p_evt->del.record_id);
                INFO("File ID:\t0x%04x",    p_evt->del.file_id);
                INFO("Record key:\t0x%04x", p_evt->del.record_key);
            }
            m_delete_all.pending = false;
        } break;

        default:
            break;
    }
}


void FDS::init()
{
  // Register first to receive an event when initialization is complete.
  (void) fds_register(event_handler);

  INFO("Initializing fds...");

  uint32_t rc = fds_init();
  ERROR_CHECK(rc);

  waitForInitialisation();
}


//@brief   Wait for fds to initialize.
void FDS::waitForInitialisation(void)
{
    while (!m_initialised)
    {
        power_manage();
    }
}

//@brief   Wait for fds to initialize.
void FDS::waitForWrite(void)
{
    while (!m_writeEvent)
    {
        power_manage();
    }
    m_writeEvent = false;
}

//@brief   Wait for fds to initialize.
void FDS::waitForUpdate(void)
{
    while (!m_updateEvent)
    {
        power_manage();
    }
    m_updateEvent = false;
}


void FDS::clean()
{
  fds_gc();
}

void FDS::status()
{
  INFO("Reading flash usage statistics...");

  fds_stat_t stat = {0};

  uint32_t rc = fds_stat(&stat);
  ERROR_CHECK(rc);

  INFO("Found %d valid records.", stat.valid_records);
  INFO("Found %d dirty records (ready to be garbage collected).", stat.dirty_records);

}




/*
int main(void)
{


  // Dummy configuration data.
  static configuration_t m_dummy_cfg;
  m_dummy_cfg.config1_on  = false;
  m_dummy_cfg.config2_on  = true;
  m_dummy_cfg.boot_count  = 0x0;


    log_init();
    ble_stack_init();
    timer_init();

    NRF_LOG_INFO("fds example started!")


    FDS::init();
    FDS::status();
    FDS::clean();

    Flash_Record configRecord(CONFIG_FILE, CONFIG_REC_KEY, &m_dummy_cfg, sizeof(m_dummy_cfg));
    configRecord.read();
    m_dummy_cfg.boot_count++;
    NRF_LOG_INFO("Boot Count: %d", m_dummy_cfg.boot_count);
    configRecord.update();

    uint32_t data[] = { 12 };
    Flash_Record dataRecord(CONFIG_FILE, CONFIG_REC_KEY+1, data, sizeof(data[0]));
    dataRecord.read();
    NRF_LOG_INFO("Data: %d", data[0]);
    data[0] = data[0] * 2;
    dataRecord.update();


    // Enter main loop.
    for (;;)
    {

        if (!NRF_LOG_PROCESS())
        {
            power_manage();
        }

        delete_all_process();
    }

}
*/

/**
 * @}
 */
