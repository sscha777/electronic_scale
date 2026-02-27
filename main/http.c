
/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "scspiffs.h"
#include "eve/EVE_target.h"
#include "cJSON.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "hangle.h"
#include "http.h"
#include "sntp.h"
#include "serialLED.h"
#include "tft.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "file_server.h"


RingbufHandle_t xRingbuffer;
PRODUCT_DATA productData;
PROGRESS_DATA taskInfo;
MEASUREMENT_DATA postData;

extern SCALE_SCENARIO newScenario;

#define MAX_HTTP_OUTPUT_BUFFER 4096
char g_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {
    0,
};

uint32_t reqBufferSize = 0; // debug
static const char *TAG = "HTTP_CLIENT";
// static uint32_t countLocalSeverTimeOut = 0; // localtimeout
/*
[REST API Global Value]
*/
// char *server_url = "http://lauren.dev.aend.co.kr";
char server_url[250];
char *endpoint_tag_id = "/api/work";
char *endpoint_time = "/api/date";
char *endpoint_task_progress = "/task/progress";



char *JSON_Types(int type)
{
    if (type == cJSON_Invalid)
        return ("cJSON_Invalid");
    if (type == cJSON_False)
        return ("cJSON_False");
    if (type == cJSON_True)
        return ("cJSON_True");
    if (type == cJSON_NULL)
        return ("cJSON_NULL");
    if (type == cJSON_Number)
        return ("cJSON_Number");
    if (type == cJSON_String)
        return ("cJSON_String");
    if (type == cJSON_Array)
        return ("cJSON_Array");
    if (type == cJSON_Object)
        return ("cJSON_Object");
    if (type == cJSON_Raw)
        return ("cJSON_Raw");
    return NULL;
}

void JSON_Parse(const cJSON *const root)
{
    cJSON *current_element = NULL;

    // //ESP_LOGI(TAG, ">.... ");
    cJSON_ArrayForEach(current_element, root)
    {
        //ESP_LOGD(TAG, "type=%s", JSON_Types(current_element->type));
        //ESP_LOGD(TAG, "current_element->string=%s[%p]", current_element->string, current_element->string);

        if (cJSON_IsInvalid(current_element))
        {
            //ESP_LOGI(TAG, "Invalid");
        }
        else if (cJSON_IsFalse(current_element))
        {
            //ESP_LOGI(TAG, "False");
        }
        else if (cJSON_IsTrue(current_element))
        {
            //ESP_LOGI(TAG, "True");
        }
        else if (cJSON_IsNull(current_element))
        {
            //ESP_LOGI(TAG, "Null");
        }
        else if (cJSON_IsNumber(current_element))
        {
            if (strcmp(current_element->string, "result") == 0)
            {
                int valueint = current_element->valueint;
                taskInfo.req_code = productData.req_code = valueint;
                if (valueint != 200)
                {
                    taskInfo.workerRegister = 0;
                    memset(taskInfo.worker_name, 0, sizeof(taskInfo.worker_name));
                    // memset(taskInfo.group_name, 0, sizeof(taskInfo.group_name));
                    newScenario = MSG_UNREG_SCNARIO;
                    ESP_LOGD(TAG, "unregistor!");
                }
                ESP_LOGD(TAG, "req_code=%d", valueint);
            }
            else if (strcmp(current_element->string, "work_seq") == 0)
            {
                productData.work_seq = taskInfo.work_seq = current_element->valueint;
                ESP_LOGD(TAG, "%s[%d] ", current_element->string, (int)taskInfo.work_seq);
            }
            else if (strcmp(current_element->string, "product_weight") == 0)
            {
                taskInfo.target_weight = productData.product_weight = current_element->valueint;
                ESP_LOGD(TAG, "%s[%d] ", current_element->string, (int)taskInfo.target_weight);
            }
            else if (strcmp(current_element->string, "bowl_weight") == 0)
            {
                taskInfo.bowl_weight = productData.bowl_weight = current_element->valueint;
                if(taskInfo.bowl_weight != 0)
                {
                    taskInfo.bowl_flag = 1;
                }
                else
                {
                    taskInfo.bowl_flag = 0;
                }
                ESP_LOGD(TAG, "%s[%d] ", current_element->string, (int)taskInfo.bowl_weight);
            }
            else if (strcmp(current_element->string, "minimum_weight") == 0)
            {
                taskInfo.min_weight = productData.minimum_weight = current_element->valueint;
                ESP_LOGD(TAG, "%s[%d] ", current_element->string, (int)taskInfo.min_weight);
            }
            else if (strcmp(current_element->string, "maximum_weight") == 0)
            {
                taskInfo.max_weight = productData.maximum_weight = current_element->valueint;
                ESP_LOGD(TAG, "%s[%d] ", current_element->string, (int)taskInfo.max_weight);
            }
            else if (strcmp(current_element->string, "work_quantity") == 0) 
            {
                taskInfo.work_quantity = productData.work_quantity = current_element->valueint;
                ESP_LOGD(TAG, "work_quantity %s[%d] ", current_element->string, (int)taskInfo.work_quantity);
            }
            else if (strcmp(current_element->string, "sort_num") == 0)
            {
                productData.sort_num = current_element->valueint;
                ESP_LOGD(TAG, "%s[%d] ", current_element->string,(int)productData.sort_num);
            }
        }
        else if (cJSON_IsString(current_element))
        {
            const char *valuestring = current_element->valuestring;
            if (strcmp(current_element->string, "message") == 0)
            {
                memset(productData.req_message, 0, sizeof(productData.req_message));
                strcpy(productData.req_message, valuestring);
                ESP_LOGD(TAG, "%s = [%s]", current_element->string, productData.req_message);
            }
            if (strcmp(current_element->string, "work_line_txt") == 0)
            {
                taskInfo.workerRegister = 1;
                memset(productData.work_line_txt, 0, sizeof(productData.work_line_txt));
                strcpy(productData.work_line_txt, valuestring);
                ESP_LOGD(TAG, "%s = [%s]", current_element->string, productData.work_line_txt);
            }
            if (strcmp(current_element->string, "name") == 0)
            {
                memset(productData.name, 0, sizeof(productData.name));
                strcpy(productData.name, valuestring);
                memset(taskInfo.worker_name, 0, sizeof(taskInfo.worker_name));
                strcpy(taskInfo.worker_name, valuestring);
                // hangle_mem_write(taskInfo.worker_name, HG_MEM_WORKER); /*  */
                ESP_LOGD(TAG, "%s = [%s]", current_element->string, productData.name);
            }
            if (strcmp(current_element->string, "work_name") == 0)
            {
                memset(productData.work_name, 0, sizeof(productData.work_name));
                strcpy(productData.work_name, valuestring);
                memset(taskInfo.work_name, 0, sizeof(taskInfo.work_name));
                strcpy(taskInfo.work_name, valuestring);
                // hangle_mem_write(taskInfo.work_name, HG_MEM_WORK_NAME); /*  */
                ESP_LOGD(TAG, "%s = [%s]", current_element->string, taskInfo.work_name);
            }
            else if (strcmp(current_element->string, "work_line") == 0)
            {
                memset(productData.work_line, 0, sizeof(productData.work_line));
                ESP_LOGD(TAG, "work_line [%s]!\n", productData.work_line);
            }
        }
        else if (cJSON_IsArray(current_element))
        {
            //ESP_LOGI(TAG, "Array");
            JSON_Parse(current_element);
        }
        else if (cJSON_IsObject(current_element))
        {
            //ESP_LOGI(TAG, "Object");
            JSON_Parse(current_element);
        }
        else if (cJSON_IsRaw(current_element))
        {
            //ESP_LOGI(TAG, "Raw(Not support)");
        }
    }
}

void convStr2Date(const char *date, struct tm *stime)
{
    char buf[5] = {
        0,
    },
         buf1[5] = {
             0,
         },
         buf2[5] = {
             0,
         };
    int i;
    int strCnt = 0;

    for (i = 0; i < 10; i++)
    {
        if (i < 4)
        {
            buf[i] = date[i];
            if (i == 3)
            {
                stime->tm_year = (atoi(buf) - 1900);
            }
        }
        else if (i == 5 || i == 6)
        {
            buf1[strCnt++] = date[i];
            if (i == 6)
            {
                stime->tm_mon = atoi(buf1);
                strCnt = 0;
            }
        }

        if (i == 8 || i == 9)
        {
            buf2[strCnt++] = date[i];
            if (i == 9)
            {
                stime->tm_mday = atoi(buf2);
            }
        }
    }
}

void convStr2time(const char *time, struct tm *stime)
{
    char tbuf[15] = {
        0,
    },
         tbuf1[15] = {
             0,
         },
         tbuf2[15] = {
             0,
         };
    int strCnt = 0;

    int i;
    for (i = 0; i < 14; i++)
    {
        if (i < 2)
        {
            tbuf[i] = time[i];
            if (i == 1)
            {
                stime->tm_hour = atoi(tbuf);
            }
        }
        else if (i == 6 || i == 7)
        {
            tbuf1[strCnt++] = time[i];
            if (i == 7)
            {
                stime->tm_min = atoi(tbuf1);
                strCnt = 0;
            }
        }

        if (i == 12 || i == 13)
        {
            tbuf2[strCnt++] = time[i];
            if (i == 13)
            {
                stime->tm_sec = atoi(tbuf2);
            }
        }
    }
}

void JSON_ParseTime(const cJSON *const root)
{
    struct tm sTime;
    cJSON *current_element = NULL;

    cJSON_ArrayForEach(current_element, root)
    {
        if (cJSON_IsInvalid(current_element))
        {
            //ESP_LOGI(TAG, "Invalid");
        }
        else if (cJSON_IsFalse(current_element))
        {
            //ESP_LOGI(TAG, "False");
        }
        else if (cJSON_IsTrue(current_element))
        {
            //ESP_LOGI(TAG, "True");
        }
        else if (cJSON_IsNull(current_element))
        {
            //ESP_LOGI(TAG, "Null");
        }
        else if (cJSON_IsNumber(current_element))
        {
            if (strcmp(current_element->string, "result") == 0)
            {
                int valueint = current_element->valueint;
                if (valueint != 200)
                {
                    //ESP_LOGI(TAG, "time get error!");
                }
                //ESP_LOGI(TAG, "req_code=%d", valueint);
            }
        }
        else if (cJSON_IsString(current_element))
        {
            const char *valuestring = current_element->valuestring;
            if (strcmp(current_element->string, "date") == 0)
            {
                convStr2Date(valuestring, &sTime);
                scSetDay(sTime);
                //ESP_LOGI(TAG, "%s %d-%d-%d", current_element->string, sTime.tm_year, sTime.tm_mon, sTime.tm_mday);
            }
            if (strcmp(current_element->string, "time") == 0)
            {
                convStr2time(valuestring, &sTime);
                scSetTime(sTime);
                //ESP_LOGI(TAG, "%s %d-%d-%d", current_element->string, sTime.tm_hour, sTime.tm_min, sTime.tm_sec);
            }
        }
        else if (cJSON_IsArray(current_element))
        {
            //ESP_LOGI(TAG, "Array");
            JSON_ParseTime(current_element);
        }
        else if (cJSON_IsObject(current_element))
        {
            //ESP_LOGI(TAG, "Object");
            JSON_ParseTime(current_element);
        }
        else if (cJSON_IsRaw(current_element))
        {
            //ESP_LOGI(TAG, "Raw(Not support)");
        }
    }
}

esp_err_t getTagData(uint8_t tag[5])
{
    esp_err_t ret = ESP_OK;

    char buf[500] = {
        0,
    };
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   // Buffer to store response of http request
    int content_length = 0;

    strcpy(server_url, (char *)scaleInfo.server);
    sprintf(buf, "%s%s/%x:%x:%x:%x", server_url, endpoint_tag_id, tag[0], tag[1], tag[2], tag[3]);
    ESP_LOGI(TAG,"%s\n", buf);

    esp_http_client_config_t config = {
        .url = buf,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 0);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        ret = ESP_FAIL;
    } else {
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
            ret = ESP_FAIL;
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) 
            {
                ESP_LOGD(TAG, "HTTP GET Status = %d, content_length = %"PRIu64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
                ESP_LOGD(TAG, "%s \n",output_buffer);
                // JSON 데이터 파싱
                cJSON *root = cJSON_Parse(output_buffer);
                if (root != NULL)
                {
                    JSON_Parse(root);
                    cJSON_Delete(root);
                }
                else
                {
                    ESP_LOGE(TAG, "Failed to parse JSON data.");
                }
                ret = ESP_OK;
            } 
            else 
            {
                ESP_LOGE(TAG, "Failed to read response");
                ret = ESP_FAIL;
            }
        }
    }
    esp_http_client_close(client);

    return (ret);
}


// http://lauren.dev.aend.co.kr/api/date
esp_err_t getTimeLocal(void)
{
    esp_err_t ret = ESP_OK;
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    char buf[500] = {
        0,
    };
    int content_length = 0;

    strcpy(server_url, (char *)scaleInfo.server);
    sprintf(buf, "%s%s", server_url, endpoint_time);
    
    esp_http_client_config_t config = {
        .url = buf,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 0);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        ret = ESP_FAIL;
    } else {
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
            ret = ESP_FAIL;
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) 
            {
                ESP_LOGD(TAG, "HTTP GET Status = %d, content_length = %"PRIu64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
                ESP_LOGD(TAG, "%s \n",output_buffer);
                // JSON 데이터 파싱
                cJSON *root = cJSON_Parse(output_buffer);
                if (root != NULL)
                {
                    JSON_ParseTime(root);
                    cJSON_Delete(root);
                }
                else
                {
                    ESP_LOGE(TAG, "Failed to parse JSON data.");
                }
                ret = ESP_OK;
            } 
            else 
            {
                ESP_LOGE(TAG, "Failed to read response");
                ret = ESP_FAIL;
            }
        }
    }
    esp_http_client_close(client);
    return (ret);
}



#if 1
void JSON_Parse2(const cJSON *const root)
{
    cJSON *current_element = NULL;

    cJSON_ArrayForEach(current_element, root)
    {
        ESP_LOGD(TAG, "j2 type=%s", JSON_Types(current_element->type));
        ESP_LOGD(TAG, "j2 current_element->string=%s[%p]", current_element->string, current_element->string);

        if (cJSON_IsInvalid(current_element))
        {
            ESP_LOGD(TAG, "Invalid");
        }
        else if (cJSON_IsFalse(current_element))
        {
            ESP_LOGD(TAG, "False");
        }
        else if (cJSON_IsTrue(current_element))
        {
            ESP_LOGD(TAG, "True");
        }
        else if (cJSON_IsNull(current_element))
        {
            ESP_LOGD(TAG, "Null");
        }
        else if (cJSON_IsNumber(current_element))
        {
            if (strcmp(current_element->string, "result") == 0)
            {
                int valueint = current_element->valueint;
                taskInfo.post_req_code = valueint;
                ESP_LOGD(TAG, "post_req_code=%d", valueint);
            }
            else if (strcmp(current_element->string, "production_cnt") == 0)
            {
                taskInfo.production_total_cnt = productData.production_cnt = current_element->valueint;
                taskInfo.current_product_cnt++;
                ESP_LOGE(TAG, "%s[%d] ", current_element->string, (int)taskInfo.production_total_cnt); // debug
            }
            else if (strcmp(current_element->string, "sort_num") == 0)
            {
                productData.sort_num = current_element->valueint;
                if (taskInfo.sort_num != productData.sort_num)
                {
                    taskInfo.seqChangeFlag = 1;
                    ESP_LOGE(TAG, "%s[%d] ", current_element->string, (int)productData.sort_num);
                }
                ESP_LOGD(TAG, "%s[%d] ", current_element->string, (int)productData.sort_num);
            }
        }
        else if (cJSON_IsString(current_element))
        {
            const char *valuestring = current_element->valuestring;

            if (strcmp(current_element->string, "work_status") == 0)
            {
                memset(taskInfo.work_status, 0, 2);
                strcpy(taskInfo.work_status, valuestring);
                ESP_LOGD(TAG, "%s = [%s]", current_element->string, taskInfo.work_status);
            }
        }
        else if (cJSON_IsArray(current_element))
        {
            //ESP_LOGI(TAG, "Array");
            JSON_Parse2(current_element);
        }
        else if (cJSON_IsObject(current_element))
        {
            //ESP_LOGI(TAG, "Object");
            JSON_Parse2(current_element);
        }
        else if (cJSON_IsRaw(current_element))
        {
            //ESP_LOGI(TAG, "Raw(Not support)");
        }
    }
}
#endif

esp_err_t postWorkData(void)
{
    
    // POST
    //"{ \"production_weight\": 500, \"tag_id\": \"5E:92:DC:90\", \"work_seq\": 80}"
    // http://lauren.dev.aend.co.kr/api/production/reg
    // char *url_data = "http://lauren.dev.aend.co.kr/api/production/reg";
    // char *url_data = "http://192.168.0.34:8080/api/production/reg";

    esp_err_t err = ESP_OK;
    char url_data[500] = {
        0,
    };
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   // Buffer to store response of http request
    char post_data[MAX_HTTP_OUTPUT_BUFFER] = {0}; 
    int content_length = 0;
    char *url_sub = "/api/production/reg\0";

    strcpy(url_data, scaleInfo.server);
    strcat(url_data, url_sub);
    ESP_LOGD(TAG, "%s ", url_data);
    esp_http_client_config_t config = {
        .url = url_data,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    sprintf(post_data, "{\"production_weight\":%ld,\"tag_id\":\"%x:%x:%x:%x\",\"work_seq\":%ld}",
            postData.weight, taskInfo.tag_serial[0],
            taskInfo.tag_serial[1], taskInfo.tag_serial[2],
            taskInfo.tag_serial[3], taskInfo.work_seq);

    esp_http_client_set_url(client, url_data);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                taskInfo.sort_num = productData.sort_num; /*  */
                taskInfo.total_weight += postData.weight;
                ESP_LOGD(TAG, "HTTP POST Status = %d, content_length = %"PRIu64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));

                ESP_LOGD(TAG, "%s \n",output_buffer);
                cJSON *root = cJSON_Parse(output_buffer);
                JSON_Parse2(root);
                cJSON_Delete(root);
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_cleanup(client);

    return err;
}

esp_err_t postfaultyData(void)
{
    // POST
    //"{ \"production_weight\": 500, \"tag_id\": \"5E:92:DC:90\", \"work_seq\": 80}"
    // char *url_data = "http://lauren.dev.aend.co.kr/api/faulty/reg";
    // char *url_data = "http://192.168.0.34:8080/api/faulty/reg";
    esp_err_t err = ESP_OK;
    char url_data[500] = {
        0,
    };
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   // Buffer to store response of http request
    char post_data[MAX_HTTP_OUTPUT_BUFFER] = {0}; 
    int content_length = 0;
    char *url_sub = "/api/faulty/reg\0";

    strcpy(url_data, scaleInfo.server);
    strcat(url_data, url_sub);
    ESP_LOGD(TAG, "%s", url_data);
    esp_http_client_config_t config = {
        .url = url_data,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    sprintf(post_data, "{\"production_weight\":%ld,\"tag_id\":\"%x:%x:%x:%x\",\"work_seq\":%ld}",
            taskInfo.defactive_weight, taskInfo.tag_serial[0],
            taskInfo.tag_serial[1], taskInfo.tag_serial[2],
            taskInfo.tag_serial[3], taskInfo.work_seq);

    // esp_http_client_set_url(client, "http://lauren.dev.aend.co.kr/api/faulty/reg");
    esp_http_client_set_url(client, url_data);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                taskInfo.sort_num = productData.sort_num; /*  */
                taskInfo.total_weight += postData.weight;
                ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRIu64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
                ESP_LOGD(TAG, "%s \n",output_buffer);
                cJSON *root = cJSON_Parse(output_buffer);
                JSON_Parse2(root);
                cJSON_Delete(root);
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_cleanup(client);
    return err;
}

void http_task(void *pvParameters)
{
    // static getTimeLimit = 0;
    DELAY_MS(5000);
    // Create Ring Buffer
    // No Split

    xRingbuffer = xRingbufferCreate(8192, RINGBUF_TYPE_NOSPLIT);
    // Allow_Split
    // xRingbuffer = xRingbufferCreate(1024, RINGBUF_TYPE_ALLOWSPLIT);
    // Check everything was created
    configASSERT(xRingbuffer);

    while (1)
    {
        if (scaleInfo.wifi_status)
        {
            break;
        }
        DELAY_MS(1000);
    }

    /* Start the file server after network stack is ready */
    ESP_ERROR_CHECK(start_file_server("/spiffs"));

    if (ESP_OK == getTimeLocal())
    {
        taskInfo.timeServerMode = TIME_SERVER_LOCAL;
    }
    else
    {
        taskInfo.timeServerMode = TIME_SERVER_CONNECTING;
        ESP_LOGE(TAG, "time get error!!");
    }

    while (1)
    {
        if (taskInfo.tag_flag == 1)
        {
            taskInfo.tag_flag = 0;
            taskInfo.work_step = W_TAGGING;
            sendLedDisp(LED_G_ALL_ON, 500);
            if (taskInfo.timeServerMode == TIME_SERVER_LOCAL)
            {
                if (ESP_OK == getTagData(taskInfo.tag_serial))
                {
                    taskInfo.work_step = W_TAGGING_OK;
                }
                else
                {
                    ESP_LOGE(TAG, "getTag Error");
                    taskInfo.work_step = W_TAGGING_ERR;
                    // EVE_sound_play(0x52, AUDIO_VOLUME); //cawbell debug
                }
            }
        }

        if (taskInfo.work_step == W_COMPLETE)
        {
            taskInfo.work_step = W_CHEATING;
            ESP_LOGI(TAG, "W_CHEATING");
        }
        else if (taskInfo.work_step == W_POST)
        {
            ESP_LOGI(TAG, "W_POST");
            if (ESP_OK == postWorkData())
            {
                if (strcmp(taskInfo.work_status, "E") == 0 || taskInfo.post_req_code == 503)
                {
                    taskInfo.work_quantity = 0;
                    taskInfo.production_total_cnt = 0;
                    taskInfo.current_product_cnt = 0;
                    taskInfo.work_status[0] = 0;
                    taskInfo.work_step = W_END_CONFORM;
                    ESP_LOGI(TAG, "task complet [%d]", (int)taskInfo.production_total_cnt);
                }
                else
                {
                    ESP_LOGI(TAG, " %d Post OK!", (int)taskInfo.postWeight);
                    taskInfo.work_step = W_START;
                }
            }
            else
            {
                taskInfo.work_step = NO_JOB;
                ESP_LOGI(TAG, "Post err seq(%d)!", (int)taskInfo.work_seq);
            }
        }
        else if (taskInfo.work_step == W_END_CONFORM)
        {
            ESP_LOGI(TAG, "W_END_CONFORM");
            taskInfo.work_step = W_DEFACTIVE_REG;
            taskInfo.bowl_flag = 0;
            taskInfo.bowl_weight = 0;
            newScenario = FAILWEIGHT_SCENARIO;
        }
        else if (taskInfo.work_step == W_POST_DEFACTIVE)
        {
            if (ESP_OK == postfaultyData())
            {
                taskInfo.work_step = NO_JOB_NO_WORKER;
                ESP_LOGI(TAG, "task complet [%d]", (int)taskInfo.production_total_cnt);
            }
            else
            {
                taskInfo.work_step = NO_JOB;
                ESP_LOGI(TAG, "Fail [%d]", (int)taskInfo.production_total_cnt);
            }
        }

        DELAY_MS(10);
    }
}
