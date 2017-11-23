
/*
 * ble_characteristic.cpp
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_srv_common.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "nrf_ble_gatt.h"

#include "ble/ble_interface.h"
#include "ble/gn_ble_advertising.h"
#include "ble/ble_service.h"
#include "app_timer.h"

#include "app/events.h"
#include "peripheral/gpio_interface.h"
#include "debug/DEBUG.h"


#define NRF_LOG_MODULE_NAME BLE
NRF_LOG_MODULE_REGISTER();

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2    /**< Reply when unsupported features are requested. */

#define APP_BLE_OBSERVER_PRIO           3                                       //!< Application's BLE observer priority. You shouldn't need to modify this value.
#define APP_BLE_CONN_CFG_TAG            1                                       //!< A tag identifying the SoftDevice BLE configuration.

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)                  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)                   /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define MEM_BUFF_SIZE                   512
#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */



namespace BLE_SERVER
{

static uint16_t       m_conn_handle = BLE_CONN_HANDLE_INVALID;                  /**< Handle of the current connection. */
//static nrf_ble_gatt_t m_gatt;                                                   /**< GATT module instance. */


NRF_BLE_GATT_DEF(m_gatt);

static Service m_serviceList[MAX_NUMBER_SERVICES] = {0};
static uint8_t m_serviceCount = 0;
static bool m_isConnected = false;
static bool m_txEvent     = false;
static BLEPowerLevel m_powerLevel = BLE_POWER_0_DB;

static void ble_stack_init();
static void gatt_init();
static void conn_params_init();
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
static void conn_params_error_handler(uint32_t nrf_error);
static void ble_evt_dispatch(ble_evt_t const * p_ble_evt);



/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}



void addService(Service* service, uint8_t serviceID) {
	m_serviceList[serviceID] = *service;
	m_serviceCount++;
}


Service* getService(uint8_t serviceID) {
  return &m_serviceList[serviceID];
}

bool isConnected()
{
  return m_isConnected;
}

void waitForTx()
{
  while (!m_txEvent)
  {
    sd_app_evt_wait();
  }
  m_txEvent = false;
}

void fwd_ble_evt(ble_evt_t const * p_ble_evt, void* context)
{
  ble_evt_t* p_ble_evt_data = const_cast<ble_evt_t*>(p_ble_evt);
  EVENTS::eventPut(BLE_EVENT_EVENT, p_ble_evt_data, sizeof(*p_ble_evt));
}

/**@brief Function for handling the Application's BLE stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
void on_ble_evt(ble_evt_t const * p_ble_evt, void* context)
{
    uint32_t err_code;

    //ble_evt_t * p_ble_evt = (ble_evt_t*) data.p_data;

    switch (p_ble_evt->header.evt_id)
    {

        case BLE_GAP_EVT_CONNECTED:
            INFO("Event: Device connected to BLE");
            EVENTS::eventPut(BLE_CONNECTED_EVENT);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            m_isConnected = true;
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_DISCONNECTED:
            INFO("Event: Deviced disconnected from BLE");
            EVENTS::eventPut(BLE_DISCONNECTED_EVENT);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            m_isConnected = false;
            break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                                   BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                                   NULL,
                                                   NULL);
            ERROR_CHECK(err_code);
            break; // BLE_GAP_EVT_SEC_PARAMS_REQUEST

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_SYS_ATTR_MISSING

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            INFO("Event: GATT Client Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            ERROR_CHECK(err_code);
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            INFO("Event: GATT Server Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_TIMEOUT

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            ERROR_CHECK(err_code);
            break; // BLE_EVT_USER_MEM_REQUEST

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {

            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;
            memset(&auth_reply, 0, sizeof(ble_gatts_rw_authorize_reply_params_t));

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            INFO("RW Request: %d", req.request.read.handle);

            if (req.type == BLE_GATTS_AUTHORIZE_TYPE_READ)
            {
              auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
            }
            else if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
            {
              auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
            }

            err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       &auth_reply);
            ERROR_CHECK(err_code);

        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
          m_txEvent = true;
          break;

        case BLE_GATTS_EVT_WRITE:
          //EVENTS::eventPut(BLE_WRITE_EVENT);
          break;

        default:
            // No implementation needed.
            break;
    }

}





/**@brief Function for initializing the GATT module.
 */
void gatt_init(void)
{
    uint32_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module that
 *          are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply
 *       setting the disconnect_on_fail config parameter, but instead we use the event
 *       handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        uint32_t err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    uint32_t err_code = ble_conn_params_init(&cp_init);
    ERROR_CHECK(err_code);
}


void setPower(BLEPowerLevel powerLevel)
{
  if (powerLevel != m_powerLevel)
  {
    INFO("SETTING - TX Power: %ddB", powerLevel);
    uint32_t err_code = sd_ble_gap_tx_power_set(powerLevel);
    m_powerLevel = powerLevel;
    ERROR_CHECK(err_code);
  }
}



/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
void ble_evt_dispatch(ble_evt_t const * p_ble_evt, void* context)
{
  //ble_evt_t * p_ble_evt = (ble_evt_t*) data.p_data;

  INFO("Event: %d dispatching", p_ble_evt->header.evt_id);
  for (int i = 0; i < MAX_NUMBER_SERVICES; i++)
  {
    if (m_serviceList[i].isRunning()) { m_serviceList[i].eventHandler(p_ble_evt); }
  }

}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{


    // Initialize the SoftDevice handler module.
    //SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    uint32_t err_code;
    err_code = nrf_sdh_enable_request();
    ERROR_CHECK(err_code);

    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;

    //err_code = nrf_sdh_ble_app_ram_start_get(&ram_start);
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    ERROR_CHECK(err_code);


    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    ERROR_CHECK(err_code);

    // Subscribe for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, on_ble_evt, NULL);
    NRF_SDH_BLE_OBSERVER(m_ble_dispatcher, APP_BLE_OBSERVER_PRIO, ble_evt_dispatch, NULL);
    //EVENTS::registerEventHandler(BLE_EVENT_EVENT, on_ble_evt);
    //EVENTS::registerEventHandler(BLE_EVENT_EVENT, ble_evt_dispatch);

}

gn_char_error_t setCharacteristic(uint8_t serviceID, uint8_t charID, void* p_data, uint16_t length)
{
  return getService(serviceID)->getCharacteristic(charID)->set(p_data, length);
}



void setWriteHandler(uint8_t serviceID, uint8_t charID, char_write_handler_t writeHandler)
{
  getService(serviceID)->getCharacteristic(charID)->setWriteHandler(writeHandler);
}


void init(void) {
    ble_stack_init();
    BLE_ADVERTISING::params_init();
    gatt_init();
    conn_params_init();
}
}
