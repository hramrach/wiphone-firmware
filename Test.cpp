/*
Copyright © 2019, 2020, 2021 HackEDA, Inc.
Licensed under the WiPhone Public License v.1.0 (the "License"); you
may not use this file except in compliance with the License. You may
obtain a copy of the License at
https://wiphone.io/WiPhone_Public_License_v1.0.txt.

Unless required by applicable law or agreed to in writing, software,
hardware or documentation distributed under the License is distributed
on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language
governing permissions and limitations under the License.
*/

#include "Test.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"

#define TEST_BLOCKS_SD                4           // or 2048 for deeper test
#define TEST_BLOCKS_SPIFFS            4


double taylor_pi(int n) {
  // Taylor method for Pi approximation
  // Source: https://stackoverflow.com/a/32672747/5407270
  double sum = 0.0;
  int sign = 1;
  for (int i = 0; i < n; ++i) {
    sum += sign/(2.0*i+1.0);
    sign *= -1;
  }
  return 4.0*sum;
}

void print_system_info() {
  Serial.print("ESP32 SDK: ");
  Serial.println(ESP.getSdkVersion());
  Serial.print("ESP32 CPU FREQ: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println("MHz");
  Serial.print("ESP32 APB FREQ: ");
  Serial.print(getApbFrequency() / 1000000.0, 1);
  Serial.println("MHz");
  Serial.print("ESP32 FLASH SIZE: ");
  Serial.print(ESP.getFlashChipSize() / (1024.0 * 1024), 2);
  Serial.println("MB");
  Serial.print("ESP32 RAM SIZE: ");
  Serial.print(ESP.getHeapSize() / 1024.0, 2);
  Serial.println("KB");
  Serial.print("ESP32 FREE RAM: ");
  Serial.print(ESP.getFreeHeap() / 1024.0, 2);
  Serial.println("KB");
  Serial.print("ESP32 MAX RAM ALLOC: ");
  Serial.print(ESP.getMaxAllocHeap() / 1024.0, 2);
  Serial.println("KB");
  Serial.print("ESP32 FREE PSRAM: ");
  Serial.print(ESP.getFreePsram() / 1024.0, 2);
  Serial.println("KB");
  // print uptime

  //size_t free_heap = esp_get_free_heap_size();
  //size_t free_iram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  //size_t free_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  //size_t free_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);
  //log_d("Free    total heap memory: %d", free_heap);
  //log_d("Free          IRAM memory: %d", free_iram);
  //log_d("Free  8bit-capable memory: %d", free_8bit);
  //log_d("Free 32bit-capable memory: %d", free_32bit);

  //log_d("Task Name\tStatus\tPrio\tHWM\tTask\tAffinity\n");
  //char stats_buffer[1024];
  //vTaskList(stats_buffer);
  //log_d("%s", stats_buffer);

  // vTaskGetRunTimeStats(stats_buffer);
  // log_d("%s", stats_buffer);

}

void print_memory(void) {
  log_d("Memory: \r\n\ttotal:%d, internal:%d, spram:%d, Dram:%d",
        esp_get_free_heap_size(),
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
}

void test_cpu() {
  for (int i=14; i<15; i++) {
    int time = millis();
    double pi = taylor_pi(exp(i));
    log_d("%.11f %d %.1fs", pi, (int)exp(i), (millis()-time)/1000.);
  }
}

void test_memory() {
  // Linear array expansion memory test
  bool success = true;
  LinearArray<char, LA_INTERNAL_RAM> arr;
  arr.add('a');
  arr.add('z');
  do {
    arr[arr.size() - 1] = '\0';
    if (arr.size() == 32768 || arr.size() == 2097152) {
      uint32_t hash = hash_murmur(&arr[0]);
      Serial.printf("Hash(\"az...aza\") = 0x%08x (%d chars)\n", hash, arr.size() - 1);
      if (hash != 0x68e52bd8 && hash != 0xd94dfa9a) {
        success = false;
      }
    }
    arr[arr.size() - 1] = 'z';
  } while (arr.extend(&arr[0], arr.size()));
  Serial.print("MEMORY TEST: ");
  Serial.println(success ? "PASSED" : "FAILED");
}

void test_ring_buffer() {
  log_d("RING BUFFER TEST");
  log_d("================");

  RingBuffer<char> ring(5);
  char str[ring.capacity()];
  char *dyn;
  bool correct;

  // Test 1: a
  ring.put('a');
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "a") && !ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 2: ab
  ring.put('b');
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "ab") && !ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 3: abcd
  ring.put('c');
  ring.put('d');
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "abcd") && !ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 4: abcde
  ring.put('e');
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "abcde") && ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 5: abcdeZ
  ring.put('Z');
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "abcde") && ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 6: bcdeZ
  ring.get();
  ring.put('Z');
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "bcdeZ") && ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 7: ZYX
  ring.get();
  ring.put('Y');
  ring.get();
  ring.get();
  ring.put('X');
  ring.get();
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "ZYX") && !ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 8: YX
  ring.get();
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "YX") && !ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 9: YXab
  ring.put('a');
  ring.put('b');
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "YXab") && !ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 10: cdXab
  ring.put('c');
  ring.get();
  ring.put('d');
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "Xabcd") && ring.full() && !ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 11: ""
  ring.get();
  ring.get();
  ring.get();
  ring.get();
  ring.get();
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "") && !ring.full() && ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  // Test 12: ""
  ring.put('c');
  ring.reset();
  ring.getCopy(str);
  dyn = ring.getCopy();
  correct = !strcmp(dyn, str) && !strcmp(str, "") && !ring.full() && ring.empty();
  log_d("%4s: %s", correct ? "OK" : "FAIL", str);
  free(dyn);

  log_d("================");
}


// # # # # # # # # # # # # # # # # # # # # # # # # # # # #  TEST FILESYSTEMS  # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if(!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file) {
    if(file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels) {
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

bool createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\r\n", path);
  bool result;
  if ((result = fs.mkdir(path))) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir FAILED");
  }
  return result;
}

bool removeDir(fs::FS &fs, const char * path) {
  Serial.printf("Removing Dir: %s\r\n", path);
  bool result;
  if ((result = fs.rmdir(path))) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir FAILED");
  }
  return result;
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write FAILED");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append FAILED");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename FAILED");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if(fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete FAILED");
  }
}

void testFileIO(fs::FS &fs, const char *path, int writeBlocks) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < writeBlocks; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\r\n", writeBlocks * 512, end);
  file.close();
}

bool testFilesystem(fs::FS &fs, int writeBlocks) {
  listDir(fs, "/", 0);
  createDir(fs, "/mydir");
  listDir(fs, "/", 0);
  removeDir(fs, "/mydir");
  listDir(fs, "/", 2);
  writeFile(fs, "/hello.txt", "Hello ");
  appendFile(fs, "/hello.txt", "World!\r\n");
  readFile(fs, "/hello.txt");
  deleteFile(fs, "/foo.txt");
  renameFile(fs, "/hello.txt", "/foo.txt");
  readFile(fs, "/foo.txt");
  if (writeBlocks > 0) {
    testFileIO(fs, "/test.txt", writeBlocks);
  }
  return true;    // TODO
}

void test_sd_card(int writeBlocks) {
  Serial.println("-------------------- SD card test --------------------");

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("- error: no SD card attached");
    goto remount;
  } else {
    Serial.print("- SD card type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("- SD card size: %lluMB\r\n", cardSize);

    if (!testFilesystem(SD, TEST_BLOCKS_SD)) {
      goto remount;
    }

    Serial.printf("- total space: %lluMB\r\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("- used space: %lluMB\r\n", SD.usedBytes() / (1024 * 1024));
  }
  Serial.println("-------------------- ------------ --------------------");

  return;

remount:
  SD.end();
  if (!SD.begin(SD_CARD_CS_PIN, SPI, SD_CARD_FREQUENCY)) {
    Serial.println("Card remount FAILED");
  } else {
    Serial.println("Card remounted!");
  }
  return;
}

bool test_internal_flash(int writeBlocks) {
  Serial.println("-------------------- Internal flash test --------------------");
  bool res = testFilesystem(SPIFFS, TEST_BLOCKS_SPIFFS);

  {
    // Test INI files
    IniFile iniFile("/counter.dat");
    iniFile.load();
    if (!iniFile.isEmpty()) {
      iniFile.show();
    } else {
      iniFile[0]["counter"] = "0";      // unsafe initialization
    }
    uint32_t cnt = atoi(iniFile[0].getValueSafe("counter", "0"));
    char buff[11];
    sprintf(buff, "%d", cnt + 1);
    iniFile[0]["counter"] = buff;
    if (!iniFile.isEmpty()) {
      iniFile.show();
      iniFile.store();
    }
  }

  {
    // Test INI files
    // TODO: remove this in production
    IniFile ini("/dummy.ini");
    ini.load();
    if (!ini.isEmpty() && ini.length() > 500000) {
      ini.remove();
    }
    int ns = ini.addSection();
    ini[ns]["greeting"] = "Hello, World!";
    ini.show();
    ini.store();
  }

  Serial.println("-------------------- ------------ --------------------");

  return res;
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # #  TEST THREAD  # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void test_thread(void *pvParam) {
  char pszNonce[] = "5aec56209ef1e575ebf23149fee3d257925d1d1b";
  char pszCNonce[] = "";
  char pszUser[] = "andriy";
  char pszRealm[] = "sip2sip.info";
  char pszPass[] = "secret";
  char pszAlg[] = "md5";
  char szNonceCount[9] = "";
  char pszMethod[] = "INVITE";
  char pszQop[] = "";
  char pszURI[] = "sip:echo@conference.sip2sip.info";

  HASHHEX HA1;
  HASHHEX HA2 = "";
  HASHHEX Response;

  DigestCalcHA1(pszAlg, pszUser, pszRealm, pszPass, pszNonce, pszCNonce, HA1);
  log_d("HA1 = %s", HA1);
  DigestCalcResponse(HA1, pszNonce, szNonceCount, pszCNonce, pszQop, pszMethod, pszURI, HA2, Response);
  log_d("Response = %s", Response);

  char test[] = "abcdefghijklmnopqrstuvwxyz01234567890";
  log_d("Test = %s", test);
  md5Compress(test, strlen(test), test);
  log_d("Hash = %s", test);

  const uint32_t sz = 65535 >> 1;
  char *testDyn1, *testDyn2, *testDyn3;
  //HASHHEX res;
  char *res;
  res = (char *) pvPortMalloc(33);

  // fill testDyn1, testDyn2, testDyn3
  testDyn1 = (char *) pvPortMalloc(sz + 1);
  if (testDyn1) {
    char c = 'a';
    for (int i = 0; i < sz; i++) {
      testDyn1[i] = c++;
      if (c > 'z') {
        c = 'a';
      }
    }
    testDyn1[sz] = '\0';
  } else {
    log_d("testDyn1: not inited");
  }

  testDyn2 = (char *) pvPortMalloc(sz + 1);
  if (testDyn2) {
    char c = 'a';
    for (int i = 0; i < sz; i++) {
      testDyn2[i] = c++;
      if (c > 'z') {
        c = 'a';
      }
    }
    testDyn2[sz] = '\0';
  } else {
    log_d("testDyn2: not inited");
  }

  testDyn3 = (char *) pvPortMalloc(sz + 1);
  if (testDyn3) {
    char c = 'a';
    for (int i = 0; i < sz; i++) {
      testDyn3[i] = c++;
      if (c > 'z') {
        c = 'a';
      }
    }
    testDyn3[sz] = '\0';
  } else {
    log_d("testDyn3: not inited");
  }

  // Continuous test
  uint32_t c = 0;
  while (1) {
    if (testDyn1) {
      memset(res, '\0', sizeof(res) - 1);
      md5Compress(testDyn1, strlen(testDyn1), res);
      if (strncmp(res, "6f1270a284aa3d42702d2b0f18afdc5b", HASHHEXLEN >> 1)) {
        log_d("ERROR: hash1 = %s", res);
        log_d("cnt = %d", c);
        break;
      }
    }

    if (testDyn2) {
      memset(res, '\0', sizeof(res) - 1);
      md5Compress(testDyn2, strlen(testDyn2), res);
      if (strcmp(res, "6f1270a284aa3d42702d2b0f18afdc5b")) {
        log_d("ERROR: hash2 = %s", res);
        log_d("cnt = %d", c);
        break;
      }
    }

    if (testDyn3) {
      memset(res, '\0', sizeof(res) - 1);
      md5Compress(testDyn3, strlen(testDyn3), res);
      if (strcmp(res, "6f1270a284aa3d42702d2b0f18afdc5b")) {      // 612e36c3369713772e6b59a4b7d24b54
        log_d("ERROR: hash3 = %s", res);
        log_d("cnt = %d", c);
        break;
      }
    }

//    // Draw random circles
//    uint32_t r = Random.random();
//    uint16_t col = r >> 3;
//    if (GETRED(col) < GETGREEN(col)) {
//      if (GETRED(col) < GETBLUE(col))
//        col |= RED;
//      else
//        col |= BLUE;
//    } else {
//      if (GETGREEN(col) < GETBLUE(col))
//        col |= GREEN;
//      else
//        col |= BLUE;
//    }
//    gui.circle(r % TFT_WIDTH, (r >> 8) % TFT_HEIGHT, ((r >> 16) % 64) + 1, col);      // TODO: remove macros

    c++;
    if (!(c % 10000)) {
      log_d("cnt = %d", c);
    }

  }

  log_d("freeing");
  if (res) {
    vPortFree(res);
  }
  if (testDyn1) {
    vPortFree(testDyn1);
  }
  if (testDyn2) {
    vPortFree(testDyn2);
  }
  if (testDyn3) {
    vPortFree(testDyn3);
  }


//  char test2[] = "abcdefghijklmnopqrstuvwxyz012345";
//  Serial.print("Test = ");
//  Serial.print(test2);
//  Serial.println();
//  md5Compress(test2, strlen(test2), test2);
//  Serial.print("Hash = ");
//  Serial.print(test2);
//  Serial.println();
//
//  Serial.print("Test = ");
//  Serial.print(test2);
//  Serial.println();
//  md5Compress(test2, strlen(test2), hash);
//  Serial.print("Hash = ");
//  Serial.print(hash);
//  Serial.println();

  vTaskDelete(NULL);
}

void start_test_thread() {
  log_d("Creating thread");
  UBaseType_t priority;
  //priority = 5;
  priority = tskIDLE_PRIORITY;
  xTaskCreate(&test_thread, "test_thread", 4096, NULL, priority, NULL);
  log_d("- done creating thread");
}

void tinySipUnitTest() {
#ifdef TINY_SIP_DEBUG
  TinySIP sip;
  sip.unitTest();
#endif // TINY_SIP_DEBUG
}

/* Description:
 *     retrieve WiFi information for device certification.
 */
void test_wifi_info() {
  // Development: get WiFi power
  int8_t power;
  log_d("WiFi info:");
  if (esp_wifi_get_max_tx_power(&power) == ESP_OK) {
    log_d("- max. transmit power: %d", power);
  } else {
    log_d("- error: max. power not retireved");
  }
  wifi_country_t country;
  if (esp_wifi_get_country(&country) == ESP_OK) {
    log_d("- country.cc: %s", country.cc);
    log_d("- country.nchan: %d", country.nchan);
    log_d("- country.schan: %d", country.schan);
  } else {
    log_d("- error: wifi country not retrieved");
  }
  uint8_t bitmap;
  if (esp_wifi_get_protocol(WIFI_IF_STA, &bitmap) == ESP_OK) {
    log_d("- WIFI_PROTOCOL_11B = %d", bitmap & WIFI_PROTOCOL_11B);
    log_d("- WIFI_PROTOCOL_11G = %d", bitmap & WIFI_PROTOCOL_11G);
    log_d("- WIFI_PROTOCOL_11N = %d", bitmap & WIFI_PROTOCOL_11N);
  } else {
    log_d("- error: wifi protocol not retrieved");
  }
  wifi_bandwidth_t wifi_bandwidth;
  if (esp_wifi_get_bandwidth(WIFI_IF_STA, &wifi_bandwidth) == ESP_OK) {
    log_d("- wifi bandwidth: %s", (wifi_bandwidth == WIFI_BW_HT20) ? "20" : (wifi_bandwidth == WIFI_BW_HT40) ? "40" : "unk");
  } else {
    log_d("- error: wifi bandwidth not retrieved");
  }
  wifi_sta_list_t sta_list;
  if (esp_wifi_ap_get_sta_list(&sta_list) == ESP_OK) {
    log_d("- phy_11b: %d", sta_list.sta[0].phy_11b);
    log_d("- phy_11g: %d", sta_list.sta[0].phy_11g);
    log_d("- phy_11n: %d", sta_list.sta[0].phy_11n);
    log_d("- phy_lr:  %d", sta_list.sta[0].phy_lr);
  } else {
    log_d("- error: AP sta_list not retrieved");
  }
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # #  HTTP CLIENT  # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void test_http(void *pvParam) {
  const char host[] = "sip2sip.info";      // works with address

  // Show resolved IP address just for the sake of it
  IPAddress ipAddr = resolveDomain(host);
  if ((uint32_t) ipAddr != 0) {
    log_d("Resolved: %s", ipAddr.toString().c_str());
  }

  uint32_t cnt = 0;
  while(1) {
    WiFiClient tcp;
    Serial.printf("HTTP: %d\r\n", ++cnt);
    if (tcp.connect(host, 80)) {
      Serial.printf("On the Web! Socket: %d\r\n", tcp.fd());

      // HTTP header
      tcp.print("GET / HTTP/1.1\r\n");
      tcp.print("Host: sip2sip.info\r\n");
      tcp.print("User-Agent: tinySIP\r\n");
      tcp.print("Accept: text/html\r\n");
      tcp.print("\r\n");

      uint32_t nothing = 0;
      while (tcp.connected() & nothing < 1000) {
        vTaskDelay(10 / portTICK_PERIOD_MS);   // 10 ms wait
        int32_t avail, rcvd;
        uint8_t buff[1024];
        if ((avail = tcp.available()) > 0) {
          while (avail > 0) {
            rcvd = tcp.read(buff, 1023);
            if (rcvd <= 0) {
              break;
            }
            avail -= rcvd;
            Serial.print("TCP received: ");
            Serial.println(rcvd);
            char* p = strstr((const char*)buff, "\r\n");
            if (p > 0) {
              char buff2[1024];
              strncpy(buff2, (const char*)buff, (size_t)(p - (char *)&buff));
              buff2[(size_t)(p - (char *)&buff)] = '\0';
              Serial.printf("Line:\r\n%s\r\nFull:\r\n%s\r\n", buff2, (const char *) buff);
            } else {
              Serial.println("No CRNL found");
            }
          }
        } else {
          nothing++;
        }
      }
      Serial.println("HTTP - DONE");
    } else {
      Serial.println("HTTP - FAILED connection");
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);   // 1 s wait
  }
  vTaskDelete(NULL);
}

void start_http_client() {
  if (wifiState.isConnected()) {
    xTaskCreate(&test_http, "test_http", 8192, NULL, tskIDLE_PRIORITY, NULL);
  } else {
    Serial.println("WiFi not connected");
  }
}

// Test random numbers
void test_random() {
  int cnt = 1000;
  uint32_t res;
  uint32_t started;

  log_d("Random test:");

  started = millis();
  for (int i=0; i<cnt; i++) {
    res ^= Random.random();
  }
  log_d("%lu millis, res = %08x", millis()-started, res);
}

// Description:
//     Print run-time stats for threads to serial.
//     See the custom vTaskGetRunTimeStats example here:
//         https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/freertos.html
//         https://esp32.com/viewtopic.php?t=3674  (prints additional info)
//     In order for this to work on Arduino, need to add the flag:
//         -DCONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
//     to the `build.extra_flags` in platform.txt or boards.txt
#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
void showRunTimeStats() {
  TaskStatus_t *pxTaskStatusArray;
  volatile UBaseType_t uxArraySize, i;
  uint32_t ulTotalRunTime;

  // Take a snapshot of the number of tasks in case it changes while this function is executing.
  uxArraySize = uxTaskGetNumberOfTasks();
  log_d("Tasks count: %lu", uxArraySize);

  // Allocate a TaskStatus_t structure for each task.
  pxTaskStatusArray = (TaskStatus_t *) pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
  if (pxTaskStatusArray != NULL) {
    // Generate raw status information about each task.
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

    // For each populated position in the pxTaskStatusArray array, print the data in human-readable form
    for (i=0; i<uxArraySize; i++) {
      float fStatsAsPercentage = ulTotalRunTime > 0 ? (float) pxTaskStatusArray[i].ulRunTimeCounter * 100 / ulTotalRunTime : 0.0;
      log_d("%s\t\t%lu\t\t%.1f%%", pxTaskStatusArray[i].pcTaskName,
            pxTaskStatusArray[i].ulRunTimeCounter,
            fStatsAsPercentage);
    }

    // Free the dynamically allocated memory
    vPortFree(pxTaskStatusArray);
  }
}
#endif

bool easteregg_tests(char lastKeys[], bool anyPressed) {
  if (!memcmp(lastKeys + 2, "001**", 5)) {    // **100##   +
    log_d("Easter egg = 100: starting an HTTP client");
    start_http_client();
  } else if (!memcmp(lastKeys + 2, "901**", 5)) {    // **109##
    log_d("Easter egg = 109: SD card test");
    test_sd_card(0);
  } else if (!memcmp(lastKeys + 2, "011**", 5)) {    // **110##
    log_d("Easter egg = 110: Internal flash test");
    test_internal_flash(4);
  } else if (!memcmp(lastKeys + 2, "111**", 5)) {    // **111##   +
    log_d("Easter egg = 111: tinySIP unit test");
    tinySipUnitTest();
  } else if (!memcmp(lastKeys + 2, "211**", 5)) {    // **112##
    log_d("Easter egg = 112: memory test");
    test_memory();
  } else if (!memcmp(lastKeys + 2, "311**", 5)) {    // **113##
    log_d("Easter egg = 113: test CPU");
    test_cpu();
  } else if (!memcmp(lastKeys + 2, "411**", 5)) {    // **114##
    log_d("Easter egg = 114: wifi info");
    test_wifi_info();
  } else if (!memcmp(lastKeys + 2, "321**", 5)) {    // **123##   +
    log_d("Easter egg = 123: starting a test thread");
    start_test_thread();
    anyPressed = false;     // this Easter egg updates the screen -> do not update the screen
  }
  return anyPressed;
}
