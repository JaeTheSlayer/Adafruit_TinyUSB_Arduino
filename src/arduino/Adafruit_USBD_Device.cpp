/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "tusb_option.h"

#if CFG_TUD_ENABLED

#include "Adafruit_TinyUSB_API.h"

#include "Adafruit_USBD_CDC.h"
#include "Adafruit_USBD_Device.h"

// USB Information can be defined in variant file e.g pins_arduino.h
#include "Arduino.h"

/* VID, PID, Manufacturer and Product name:
 * - For most ports: USB_VID, USB_PID, USB_MANUFACTURER, USB_PRODUCT are
 * defined.
 * - For ESP32: Default USB_MANUFACTURER is Espressif (instead of Adafruit),
 * ARDUINO_BOARD as USB_PRODUCT
 * - For mbed core: BOARD_VENDORID, BOARD_PRODUCTID, BOARD_MANUFACTURER,
 * BOARD_NAME are defined
 */

#ifndef USB_VID
#define USB_VID 0x1532  // Razer's VID
#endif

#ifndef USB_PID
#define USB_PID 0x007C  // Razer Basilisk V2 PID
#endif

#ifndef USB_MANUFACTURER
#define USB_MANUFACTURER "Razer"
#endif

#ifndef USB_PRODUCT
#define USB_PRODUCT "Razer Basilisk V2"
#endif

#ifndef USB_LANGUAGE
#define USB_LANGUAGE 0x0409 // default is English
#endif

#ifndef USB_CONFIG_POWER
#define USB_CONFIG_POWER 100
#endif

enum { STRID_LANGUAGE = 0, STRID_MANUFACTURER, STRID_PRODUCT, STRID_SERIAL };

Adafruit_USBD_Device TinyUSBDevice;

Adafruit_USBD_Device::Adafruit_USBD_Device(void) {
#if defined(ARDUINO_ARCH_ESP32) && ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
  // auto begin for ESP32 USB OTG Mode with CDC on boot
  begin(0);
#endif
}

void Adafruit_USBD_Device::setConfigurationBuffer(uint8_t *buf,
                                                  uint32_t buflen) {
  if (buflen < _desc_cfg_maxlen) {
    return;
  }

  memcpy(buf, _desc_cfg, _desc_cfg_len);
  _desc_cfg = buf;
  _desc_cfg_maxlen = buflen;
}

void Adafruit_USBD_Device::setID(uint16_t vid, uint16_t pid) {
  _desc_device.idVendor = vid;
  _desc_device.idProduct = pid;
}

void Adafruit_USBD_Device::setVersion(uint16_t bcd) {
  _desc_device.bcdUSB = bcd;
}

void Adafruit_USBD_Device::setDeviceVersion(uint16_t bcd) {
  _desc_device.bcdDevice = bcd;
}

void Adafruit_USBD_Device::setLanguageDescriptor(uint16_t language_id) {
  _desc_str_arr[STRID_LANGUAGE] = (const char *)((uint32_t)language_id);
}

void Adafruit_USBD_Device::setManufacturerDescriptor(const char *s) {
  _desc_str_arr[STRID_MANUFACTURER] = s;
}

void Adafruit_USBD_Device::setProductDescriptor(const char *s) {
  _desc_str_arr[STRID_PRODUCT] = s;
}

void Adafruit_USBD_Device::setSerialDescriptor(const char *s) {
  _desc_str_arr[STRID_SERIAL] = s;
}

// Add a string descriptor to the device's pool
// Return string index
uint8_t Adafruit_USBD_Device::addStringDescriptor(const char *s) {
  if (_desc_str_count >= STRING_DESCRIPTOR_MAX || s == NULL) {
    return 0;
  }

  uint8_t index = _desc_str_count++;
  _desc_str_arr[index] = s;
  return index;
}

void Adafruit_USBD_Device::task(void) { tud_task(); }

bool Adafruit_USBD_Device::mounted(void) { return tud_mounted(); }

bool Adafruit_USBD_Device::suspended(void) { return tud_suspended(); }

bool Adafruit_USBD_Device::ready(void) { return tud_ready(); }

bool Adafruit_USBD_Device::remoteWakeup(void) { return tud_remote_wakeup(); }

bool Adafruit_USBD_Device::detach(void) { return tud_disconnect(); }

bool Adafruit_USBD_Device::attach(void) { return tud_connect(); }

void Adafruit_USBD_Device::clearConfiguration(void) {
  tusb_desc_device_t const desc_dev = {.bLength = sizeof(tusb_desc_device_t),
                                       .bDescriptorType = TUSB_DESC_DEVICE,
                                       .bLength = sizeof(tusb_desc_device_t),
                                       .bDescriptorType = TUSB_DESC_DEVICE,
                                       .bcdUSB = 0x0300,  // USB 3.0
                                       .bDeviceClass = 0x00,  // Device class (0 if defined at interface level)
                                       .bDeviceSubClass = 0x00,  // Device subclass
                                       .bDeviceProtocol = 0x00,  // Device protocol
                                       .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
                                       .idVendor = 0x1532,  // Razer Inc. vendor ID
                                       .idProduct = USB_PID,  // Razer Basilisk V2 product ID
                                       .bcdDevice = 0x0100,  // Device release number
                                       .iManufacturer = STRID_MANUFACTURER,
                                       .iProduct = STRID_PRODUCT,
                                       .iSerialNumber = STRID_SERIAL,
                                       .bNumConfigurations = 0x01};

  _desc_device = desc_dev;

  // Config number, interface count, string index, total length,
  // attribute (bit 7 set to 1), power in mA.
  // Note: Total Length Interface Number will be updated later
  uint8_t const dev_cfg[sizeof(tusb_desc_configuration_t)] = {
      TUD_CONFIG_DESCRIPTOR(1, 0, 0, sizeof(tusb_desc_configuration_t),
                            TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP | TU_BIT(7),
                            100),
  };

  memcpy(_desc_cfg_buffer, dev_cfg, sizeof(tusb_desc_configuration_t));
  _desc_cfg = _desc_cfg_buffer;
  _desc_cfg_maxlen = sizeof(_desc_cfg_buffer);
  _desc_cfg_len = sizeof(tusb_desc_configuration_t);

  _itf_count = 0;
  _epin_count = _epout_count = 1;

  memset(_desc_str_arr, 0, sizeof(_desc_str_arr));
  _desc_str_arr[STRID_LANGUAGE] = (const char *)((uint32_t)USB_LANGUAGE);
  _desc_str_arr[STRID_MANUFACTURER] = USB_MANUFACTURER;
  _desc_str_arr[STRID_PRODUCT] = USB_PRODUCT;
  _desc_str_arr[STRID_SERIAL] = nullptr;
  // STRID_SERIAL is platform dependent

  _desc_str_count = 4;
}

// Add interface descriptor
// - Interface number will be updated to match current count
// - Endpoint number is updated to be unique
bool Adafruit_USBD_Device::addInterface(Adafruit_USBD_Interface &itf) {
  uint8_t *desc = _desc_cfg + _desc_cfg_len;
  uint16_t const len = itf.getInterfaceDescriptor(
      _itf_count, desc, _desc_cfg_maxlen - _desc_cfg_len);

  if (!len) {
    return false;
  }

  _desc_cfg_len += len;

  // Update configuration descriptor
  tusb_desc_configuration_t *config = (tusb_desc_configuration_t *)_desc_cfg;
  config->wTotalLength = _desc_cfg_len;
  config->bNumInterfaces = _itf_count;

  return true;
}

bool Adafruit_USBD_Device::begin(uint8_t rhport) {
  clearConfiguration();

  // Serial is always added by default
  // Use Interface Association Descriptor (IAD) for CDC
  // As required by USB Specs IAD's subclass must be common class (2) and
  // protocol must be IAD (1)
  _desc_device.bDeviceClass = TUSB_CLASS_MISC;
  _desc_device.bDeviceSubClass = MISC_SUBCLASS_COMMON;
  _desc_device.bDeviceProtocol = MISC_PROTOCOL_IAD;

#if defined(ARDUINO_ARCH_ESP32)
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
  // follow USBCDC cdc descriptor
  uint8_t itfnum = allocInterface(2);
  uint8_t strid = addStringDescriptor("Razer Basilik V2");
  uint8_t const desc_cdc[TUD_CDC_DESC_LEN] = {
      TUD_CDC_DESCRIPTOR(itfnum, strid, 0x81, 8, 0x02, 0x83, 8)};

  memcpy(_desc_cfg_buffer + _desc_cfg_len, desc_cdc, sizeof(desc_cdc));
  _desc_cfg_len += sizeof(desc_cdc);

  // Update configuration descriptor
  tusb_desc_configuration_t *config = (tusb_desc_configuration_t *)_desc_cfg;
  config->wTotalLength = _desc_cfg_len;
  config->bNumInterfaces = _itf_count;
#endif

  // USB Init ESP32 USB Peripheral
  esp_usb_init();
#endif

  return tud_init(rhport);
}

uint16_t tud_descriptor_device_cb(void) {
  return (uint16_t)(uintptr_t)TinyUSBDevice.getDeviceDescriptor();
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index; // for multiple configurations
  return TinyUSBDevice.getConfigurationDescriptor();
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  return TinyUSBDevice.getStringDescriptor(index, langid);
}

#endif // CFG_TUD_ENABLED
