

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <esp_spiffs.h>
#include <sys/stat.h>
#include <machine/endian.h>
#include "esp_log.h"
#include "scspiffs.h"
#include "wifi_station.h"
#include "tft.h"
#include "http.h"
#include "ads1232.h"
#include "esp_wifi.h"

static const char *TAG = "spiffs";

SCALE_INFO scaleInfo;

esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5, /* max open sametime */
    .format_if_mount_failed = true};

char *test_ap = "iptime";
char *test_pw = "01052400792";

// URL이 "http://" 또는 "https://"로 시작하는지 확인하는 함수
int startsWithHttp(const char *url) {
    return strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0;
}

// URL에 도메인 이름이 유효하고, "::" 문자를 포함하지 않는지 검사하는 함수
int hasValidDomain(const char *url) {
    const char *startOfDomain = strstr(url, "://");
    if (startOfDomain == NULL) return 0; // "://"을 포함하지 않으면 잘못된 URL

    startOfDomain += 3; // "://" 다음 문자로 이동
    if (*startOfDomain == '\0') return 0; // 도메인 이름이 없음

    // 도메인 이름에서 "::" 문자열 검사
    if (strstr(startOfDomain, "::") != NULL) return 0; // "::" 문자열을 포함하면 잘못된 URL

    // 도메인 이름이 존재하며, 공백 문자나 '/' 전에 최소 한 문자가 있는지 확인
    while (*startOfDomain != '\0' && *startOfDomain != '/' && *startOfDomain != ' ') {
        return 1; // 유효한 도메인 문자를 찾음
    }

    return 0; // 도메인 이름이 유효하지 않음
}

int isValidHttpURL(const char *url) {
    if (url == NULL) return 0;

    if (!startsWithHttp(url)) return 0;

    if (!hasValidDomain(url)) return 0;

    return 1; // 기본적인 검증 통과
}

void spiffs_init(void)
{
    char line[255] = {
        0,
    };
    /*
        scale info init and param read
    */
    scaleInfo.id = 0;
    memset(&taskInfo.tag_serial[0], 0, 5);
    scaleInfo.wifi_status = 0;
    scaleInfo.scale_factor = 0;
    scaleInfo.err.scale_factor_err = 0;
    scaleInfo.err.wifi_err = 0;
    scaleInfo.err.overload = 0;
    scaleInfo.scale_mode = W_PLUS_SCALE_MODE;
    scaleInfo.touch_cali_flag = 0;
    scaleInfo.scale_sampling = W_DEFAULT_SAMPLING; // 2
    scaleInfo.standardWeight = 1000;               // 분동 100g//500
    scaleInfo.maxFactor = 500.0;
    scaleInfo.maxweight = 999999;
    /*hx711 load cell  hx711_calibrate() 설정 변경 해야함 */

    //ESP_LOGI(TAG, "Initializing SPIFFS\n");

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            //ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            //ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            //ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        //ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        //ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // ----- read scale factor

    //ESP_LOGI(TAG, "factor Reading tft");
    FILE *f = fopen("/spiffs/scale_factor.txt", "r");
    if (f == NULL)
    {
        ////ESP_LOGE(TAG, "scale_factor.txt to open file for reading fail");
        scaleInfo.err.scale_factor_err = FILE_RE_ERR;
        saveScaleFactor(214); // debug defualt
        return;
    }
    fgets(line, sizeof(line), f);
    fclose(f);
    scaleInfo.scale_factor = atof(line);
    //ESP_LOGI(TAG, "Read from factor file: '%s'", line);

    // ----- read ssid setting

    memset(line, 0, 64);
    //ESP_LOGI(TAG, "ssid info read");
    FILE *f1 = fopen("/spiffs/ssid.txt", "r");

  
    if (f1 == NULL)
    {
        ////ESP_LOGE(TAG, "ssid.txt to open file for reading fail -> creat defalt file!");
        scaleInfo.err.wifi_err = FILE_RE_ERR;
        saveWifiApname(EXAMPLE_ESP_WIFI_SSID); // debug
        return;
    }
    fgets(line, sizeof(line), f1);
    fclose(f1);
    memcpy(scaleInfo.wifi_info.sta.ssid, line, 32);
    //ESP_LOGI(TAG, "Read ssid.txt: %s", scaleInfo.wifi_info.sta.ssid);

    // ----- read pw setting

    memset(line, 0, 64);
    //ESP_LOGI(TAG, "pw info read");
    FILE *f2 = fopen("/spiffs/pw.txt", "r");

    if (f2 == NULL)
    {
        ////ESP_LOGE(TAG, "pw.txt to open file for reading fail");
        scaleInfo.err.wifi_err = FILE_RE_ERR;
        saveWifipw(EXAMPLE_ESP_WIFI_PASS); // debug
        return;
    }
    fgets(line, sizeof(line), f2);
    fclose(f2);
    memcpy(scaleInfo.wifi_info.sta.password, line, 64);
    //ESP_LOGI(TAG, "Read pw.txt: %s", scaleInfo.wifi_info.sta.password);

    /* degug */
    memcpy(scaleInfo.wifi_info.sta.ssid, test_ap, 32);
    memcpy(scaleInfo.wifi_info.sta.password, test_pw, 64);
    // ----- read touch parameter

    memset(line, 0, 64);
    //ESP_LOGI(TAG, "touch.txt touch read");
    FILE *f3 = fopen("/spiffs/touch.txt", "r");
    if (f3 == NULL)
    {
        ////ESP_LOGE(TAG, "touch.txt is not found! run calibration screen!");
        scaleInfo.touch_cali_flag = 1;
    }
    else
    {
        // fgets(line, 24, f3);
        fscanf(f3, "%ld %ld %ld %ld %ld %ld", &calibrationvalues.TouchTransform_X[0], &calibrationvalues.TouchTransform_X[1], &calibrationvalues.TouchTransform_X[2], &calibrationvalues.TouchTransform_X[3], &calibrationvalues.TouchTransform_X[4], &calibrationvalues.TouchTransform_X[5]);
        fclose(f3);
        // memcpy(calibrationvalues.TouchTransform_Bytes, line, 24);
        // ESP_LOG_BUFFER_HEX(TAG, line, 24);
    }
    // ----- read plus/minus setting
    memset(line, 0, 64);
    char eth[10] = {
        0,
    };
    //ESP_LOGI(TAG, "scale mode");
    FILE *f4 = fopen("/spiffs/config.ini", "r");
    if (f4 == NULL)
    {
        ////ESP_LOGE(TAG, "config.ini to open file for reading fail");
        saveDefaultConfig(); // debug
        return;
    }
    // fgets(line, sizeof(line), f4);
    fscanf(f4, "%s %s", line, eth);
    fclose(f4);

    if (strcmp(line, "minus") == 0)
    {
        scaleInfo.scale_mode = W_MINUS_SCALE_MODE;
    }
    else
    {
        scaleInfo.scale_mode = W_PLUS_SCALE_MODE;
    }
    ////ESP_LOGE(TAG, ">>>> %s !!", line);
    ////ESP_LOGE(TAG, ">>>> %s !!", eth);
    if (strcmp(eth, "ethernet") == 0)
    {
        scaleInfo.ethernetConnect = 1;
    }
    else
    {
        scaleInfo.ethernetConnect = 0;
    }

    ESP_LOG_BUFFER_HEX(TAG, line, 24);

    // ----- read server setting

    memset(line, 0, 255);
    memset(scaleInfo.server, 0, sizeof(scaleInfo.server));
    //ESP_LOGI(TAG, "server");
    FILE *f5 = fopen("/spiffs/server.txt", "r");

    if (f5 == NULL)
    {
        ////ESP_LOGE(TAG, "config.ini to open file for reading fail");
        saveDefaultServer(); // debug
        return;
    }
    fgets(line, sizeof(line), f5);
    fclose(f5);

    if(isValidHttpURL(line)){
        strcpy(scaleInfo.server, line);
    }
    else{
        saveDefaultServer(); // debug
        return;
    }
    
    ESP_LOGI(TAG, "server = %s", scaleInfo.server);

    //----- read sampling speed ------

    memset(line, 0, 255);
    FILE *f6 = fopen("/spiffs/sampling_speed.txt", "r");
    if (f6 == NULL)
    {
        ////ESP_LOGE(TAG, "sampling_speed.txt to open file for reading fail");
        saveSamplingSpeed(2); // debug defualt
        return;
    }
    fgets(line, sizeof(line), f6);
    fclose(f6);
    scaleInfo.scale_sampling = atof(line);
    //ESP_LOGI(TAG, "sampling speed : %d", scaleInfo.scale_sampling);

    //----- read pass timedelay ------

    memset(line, 0, 255);
    FILE *f7 = fopen("/spiffs/passdelay.txt", "r");
    if (f7 == NULL)
    {
        ////ESP_LOGE(TAG, "passdelay.txt to open file for reading fail");
        if (scaleInfo.scale_mode == W_PLUS_SCALE_MODE)
        {
            savepassdelay(10); // debug defualt
        }
        else
        {
            savepassdelay(60); // debug defualt
        }
        return;
    }
    fgets(line, sizeof(line), f7);
    fclose(f7);
    taskInfo.w_passTime = atof(line);
    //ESP_LOGI(TAG, "pass delay : %d", taskInfo.w_passTime);

    //----- cheatting timedelay ------
    memset(line, 0, 255);
    FILE *f8 = fopen("/spiffs/cheattingtimeout.txt", "r");
    if (f8 == NULL)
    {
        ////ESP_LOGE(TAG, "cheattingtimeout.txt to open file for reading fail");
        saveCheattingTimeOut(DEFAULT_CHATTING_TIMEOUT); // debug defualt
        return;
    }
    fgets(line, sizeof(line), f8);
    fclose(f8);
    taskInfo.cheatingTimeout = atoi(line);
    // ESP_LOGI(TAG, "cheatting delay : %d", taskInfo.cheatingTimeout);
    // All done, unmount partition and disable SPIFFS
    // esp_vfs_spiffs_unregister(NULL);
    // //ESP_LOGI(TAG, "SPIFFS unmounted");

    //----- auto zero 설정 ------
    uint8_t saveFlag = 0;
    memset(line, 0, 255);
    char line3[5] = {
        0,
    };
    char line31[5] = {
        0,
    };
    char line32[5] = {
        0,
    };
    FILE *f9 = fopen("/spiffs/auto_cal.txt", "r");
    if (f9 == NULL)
    {
        //ESP_LOGE(TAG, "auto_cal.txt to open file for reading fail");
        saveAutoCalibration(DEFAULT_NUM_AUTO_SERO, DEFAULT_ZERO_CALI_BAND, DEFAULT_STABLE_BAND,DEFAULT_NUM_STABLE_BUFFER); // debug defualt
        return;
    }
    fscanf(f9, "%s %s %s %s", line, line3, line31, line32);
    fclose(f9);
    taskInfo.numAutoZero = atoi(line);
    if(taskInfo.numAutoZero > 1000000)
    {
        taskInfo.numAutoZero = DEFAULT_NUM_AUTO_SERO;
        saveFlag = 1;
    }
    taskInfo.zero_cali_band = atof(line3);
    if(taskInfo.zero_cali_band > 1000000)
    {
        taskInfo.zero_cali_band = DEFAULT_ZERO_CALI_BAND;
        saveFlag = 1;
    }
    taskInfo.stable_band01 = atoi(line31);
    if(taskInfo.stable_band01 > 1000000)
    {
        taskInfo.stable_band01 = DEFAULT_STABLE_BAND;
        saveFlag = 1;
    }
    taskInfo.numStableBuffer = atoi(line32);
    if(taskInfo.numStableBuffer > 32 || taskInfo.numStableBuffer < 1)
    {
        taskInfo.numStableBuffer = DEFAULT_NUM_STABLE_BUFFER;
        saveFlag = 1;
    }
    if(saveFlag == 1)
    {
        saveAutoCalibration(taskInfo.numAutoZero, taskInfo.zero_cali_band, taskInfo.stable_band01,taskInfo.numStableBuffer);
    }
    
    taskInfo.zero_band = (int32_t)((taskInfo.zero_cali_band / 10) * scaleInfo.scale_factor);
    taskInfo.stable_band = (int32_t)((taskInfo.stable_band01 / 10) * scaleInfo.scale_factor);
    //ESP_LOGI(TAG, "numAutoZero : %d", taskInfo.numAutoZero);
    //ESP_LOGI(TAG, "numStableBuffer : %d", taskInfo.numStableBuffer);
    //ESP_LOGI(TAG, "zero_cali_band : %f", taskInfo.zero_cali_band);
    //ESP_LOGI(TAG, "stable_band01 : %d", taskInfo.stable_band01);
    //ESP_LOGI(TAG, "zero_band : %d", taskInfo.zero_band);
    //ESP_LOGI(TAG, "stable_band : %d", taskInfo.stable_band);

 /*----- standard weight 설정 ------

----------------------------------*/
    uint8_t errFlag = 0;
    memset(line, 0, 255);
    char line4[16] = {
        0,
    };
    
    char line4_1[16] = {0,
    };
    FILE *f10 = fopen("/spiffs/standardweight.txt", "r");
    if (f10 == NULL)
    {
        ////ESP_LOGE(TAG, "standardweight.txt to open file for reading fail");
        savestandardWeight(DEFAULT_MAX_FACTOR, DEFAULT_STANDARD_WEIGHT, DEFALUT_LOADCELL_SPEC); // debug defualt
        return;
    }
    fscanf(f10, "%s %s %s", line, line4, line4_1);
    fclose(f10);
    scaleInfo.maxFactor = atof(line);
    scaleInfo.standardWeight = atof(line4);
    scaleInfo.loadCellSpec = atoi(line4_1);    
    if (scaleInfo.maxFactor < 100 || scaleInfo.maxFactor > 10000)
    {
        scaleInfo.maxFactor = DEFAULT_MAX_FACTOR;
        errFlag = 1;
    }
    if (scaleInfo.standardWeight < 100 || scaleInfo.standardWeight > 90000)
    {
        scaleInfo.standardWeight = DEFAULT_STANDARD_WEIGHT;
        errFlag = 1;
    }

    if(scaleInfo.loadCellSpec < 100 || scaleInfo.loadCellSpec > 9999999)
    {
        scaleInfo.loadCellSpec = DEFALUT_LOADCELL_SPEC;
        errFlag = 1;
    }
    if(errFlag == 1)
    {
        savestandardWeight(scaleInfo.maxFactor, scaleInfo.standardWeight,scaleInfo.loadCellSpec);
        errFlag = 0;
    }
    
    //ESP_LOGI(TAG, "max_factor : %6.1f", scaleInfo.maxFactor);
    //ESP_LOGI(TAG, "standard_weight : %6.1f", scaleInfo.standardWeight);
    //ESP_LOGI(TAG, "loadCellSpec : %d", scaleInfo.loadCellSpec);
}

/*
    디폴터 스케일 벡터
 */
void saveScaleFactor(float factor)
{
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/scale_factor.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open scale factor file for writing");
        scaleInfo.err.scale_factor_err = FILE_WR_ERR;
        return;
    }

    fprintf(f, "%6.2f\n", factor);
    fclose(f);
    //ESP_LOGI(TAG, "File written");
}
/*
 디폴터 샘플링 스피드
 */
void saveSamplingSpeed(uint8_t sampling)
{
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/sampling_speed.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open sampling speed file for writing");
        return;
    }

    fprintf(f, "%d\n", sampling);
    fclose(f);
    //ESP_LOGI(TAG, "sampling  %d File written", sampling);
}
/*
    디폴터 AP 이름
*/
void saveWifiApname(char *ap)
{
    //ESP_LOGI(TAG, "ssid file open");
    FILE *f = fopen("/spiffs/ssid.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open ssid file for writing");
        scaleInfo.err.wifi_err = FILE_WR_ERR;
        return;
    }
    char str[50];
    sprintf(str, "%s", ap);
    fprintf(f, "%s", str);
    fclose(f);
    //ESP_LOGI(TAG, "%s\n", str);
}

void saveWifipw(char *pw)
{
    //ESP_LOGI(TAG, "pw file open");
    FILE *f = fopen("/spiffs/pw.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open pw file for writing");
        scaleInfo.err.wifi_err = FILE_WR_ERR;
        return;
    }
    char str[20];
    sprintf(str, "%s", pw);
    fprintf(f, "%s", str);
    fclose(f);
    //ESP_LOGI(TAG, "pw = %s\n", str);
}

void saveTouch(int32_t tc[6])
{
    //ESP_LOGI(TAG, "touch file open");
    FILE *f = fopen("/spiffs/touch.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open touch file for writing");
        return;
    }

    ESP_LOG_BUFFER_HEX(TAG, tc, 6);
    fprintf(f, "%ld %ld %ld %ld %ld %ld", tc[0], tc[1], tc[2], tc[3], tc[4], tc[5]);
    fclose(f);
    //ESP_LOGI(TAG, "save xy = %d, %d, %d, %d, %d, %d", tc[0], tc[1], tc[2], tc[3], tc[4], tc[5]);
}

/*
plus mode  저울 초기값
 */

void saveDefaultConfig(void)
{
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/config.ini", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open scale factor file for writing");
        return;
    }
    char *s = "plus";
    char *s1 = "wifi";
    fprintf(f, "%s %s", s, s1);
    fclose(f);
    //ESP_LOGI(TAG, "File written");
}

/*
default server ip
*/
void saveDefaultServer(void)
{
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/server.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open scale factor file for writing");
        return;
    }
    char *s = "http://192.168.0.2:8080";
    fprintf(f, "%s", s);
    fclose(f);
    //ESP_LOGI(TAG, "File written");
}
/*
 디폴터 시간 딜레이
 약 20ms 단위 정확한 시간은 아님
 */
void savepassdelay(uint8_t delay)
{
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/passdelay.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open sampling speed file for writing");
        return;
    }

    fprintf(f, "%d\n", delay);
    fclose(f);
    //ESP_LOGI(TAG, "delay  %d File written", delay);
}

/*
 디폴터 치팅 타임 셋팅
 약 1초 단위
  */
void saveCheattingTimeOut(uint8_t timeOut)
{
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/cheattingtimeout.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open cheattingTimeout file for writing");
        return;
    }

    fprintf(f, "%d\n", timeOut);
    fclose(f);
    //ESP_LOGI(TAG, "timeOut  %d File written", timeOut);
}

/*
 auto_time : 8,16,32,64 단위로 시간 설정
 auto_cali_band : 1(10g) 2(5g) 5(2g) 10(1g) 20(0.5) 50(0.2) 0로 인식하는 범위 설정 디폴터 20
  */
void saveAutoCalibration(uint32_t auto_time, uint32_t auto_cali_band, int32_t st_band, uint8_t stableBufCnt)
{
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/auto_cal.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open auto calibration file for writing");
        return;
    }

    fprintf(f, "%ld %ld %ld %d\n", auto_time, auto_cali_band, st_band, stableBufCnt);
    fclose(f);
    //ESP_LOGI(TAG, "auto_time :  %ld auto_cali_band : %ld auto_cali_band %ld  nStableuffer %ld stable_band File written", auto_time, auto_cali_band, st_band, stableBufCnt);
}

/*
    scaleInfo.maxForce 와 scaleInfo.stadardweight 를 저장
*/
void savestandardWeight(float maxFactor, float stadardweight, uint32_t loadcellSpec)
{
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/standardweight.txt", "w");
    if (f == NULL)
    {
        //ESP_LOGE(TAG, "fail to open standardWeight file for writing");
        return;
    }

    fprintf(f, "%6.1f %6.1f %ld\n", maxFactor, stadardweight, loadcellSpec);
    fclose(f);
    //ESP_LOGI(TAG, "standardWeight : %6.1f loadcellSpec : %d standardweight.txt File written", standardWeight,loadcellSpec);
}
