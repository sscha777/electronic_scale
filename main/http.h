#ifndef __HTTP_H__
#define __HTTP_H__

#define FOOD_LENGTH 50
#define ORDER_CODE_LENGTH 20
// #define GROUP_NAME_LENGTH 40
#define GROUP_CODE_LENGTH 10
#define WORKER_LENGTH 20

#define TASK_NAME_LENGTH 50
#define PRGS_CODE_LENGTH 20
#define PRGS_DATA_LENGTH 30
#define INVT_CODE_LENGTH 30
#define WORKER_GROUP_CODE_LENGTH 10
#define LIFE_TIME_LENGTH 25
#define FOOD_CATEGORY_NAME 20
#define FOOD_NAME_LENGTH 20

#ifndef MIN
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

/**
 * @brief req data
 * 응답 신호 그대로 저장
 */
typedef struct _PRODUCT_DATA
{
  uint32_t req_code;
  char req_message[50];
  char product_name[50];
  uint32_t work_seq;
  uint32_t sort_num;
  char work_no[15];
  char work_name[50];
  char name[50];
  uint32_t product_weight;
  uint32_t defactive_weight;
  uint32_t minimum_weight;
  uint32_t maximum_weight;
  uint32_t bowl_weight;
  uint32_t work_quantity;
  uint32_t production_cnt;
  char work_line[4];
  char work_line_txt[50];
} PRODUCT_DATA;

extern PRODUCT_DATA productData;

/**
 * @brief get data
 *
 */
typedef struct _MEASUREMENT_DATA
{
  uint32_t weight;
  uint8_t tag_id[12];
  uint32_t work_seq;
} MEASUREMENT_DATA;

extern MEASUREMENT_DATA postData;

typedef struct _PROGRESS_DATA
{
  // uint8_t date[PRGS_DATA_LENGTH];
  uint32_t req_code;
  uint32_t post_req_code;
  uint32_t sort_num;
  uint32_t work_seq;
  uint32_t prev_seq;
  char prev_work_name[TASK_NAME_LENGTH];
  uint32_t target_weight;
  uint32_t min_weight;
  uint32_t max_weight;
  uint32_t total_weight;
  uint32_t defactive_weight;
  uint32_t minusTotalWeight;     // 마이너스저울 최종값
  uint32_t production_total_cnt; // 전체생산량 서버에서 받아옴
  uint32_t current_product_cnt;  // 이자리 현재 생산량
  uint32_t work_quantity;
  uint8_t work_step;
  uint8_t workerRegister;
  char work_name[TASK_NAME_LENGTH];
  char worker_name[TASK_NAME_LENGTH];
  // char group_name[GROUP_NAME_LENGTH];
  // uint8_t LED_step;
  uint8_t tag_flag;
  uint8_t tag_serial[10];
  char work_status[2];      // 'I'(진행중),'E'(완료)
  char messge[128];         // 리퀘스트 메세지 저장용
  uint8_t calFlag;      // 영점 조정시 사용하는 플레그
  float cali_tmp_factor;    // 영점조정시 100g 팩트 조정시 사용
  float bowl_weight;        // 저울무게 셋팅 영점 조정에서 설정
  int32_t intBowlWeight;    // 저울무게 백터로 곱셈 비교 연산에 사용
  uint32_t postWeight;      // 보낸 무게 총무게 계산시 사용
  uint32_t w_passTime;      // 측정 안정화 시간 10ms
  uint8_t timeServerMode;   // 0xff: 실패 0: 로컬 1: 인터넷
  uint8_t seqChangeFlag;    // 응답 seq  바뀜 플레그
  uint16_t cheatingTimeout; // 치팅 유효 검사 구간 1s 단위 (0면 치팅 검사 안함) plus 저울에서만
  uint32_t numAutoZero;     // auto zero 조정 100ms 단위 max 100 10초
  uint8_t numStableBuffer;  // 안정화 버퍼 갯수 //max 32
  float zero_cali_band;     // 영점 조정 밴드 // 0.1g 단위 max 1000 100g
  int32_t zero_band;        // 영점 조정 밴드
  int32_t stable_band01;    // 안정화 밴드 0.1g 단위 1000 100g
  int32_t stable_band;      // 안정화 밴드
  uint8_t stable_flag;      // 안정화 플레그
  uint8_t zero_flag;        // 영점 플레그
  uint8_t bowl_flag;        // 볼 플레그
  // uint16_t cheatingDelay;   // 치팅시 딜레이 화면 표시 구간 100ms단위 Plus저울에서만
} PROGRESS_DATA;

extern PROGRESS_DATA taskInfo;

#define TIME_SERVER_NOT_FOUND 0xff
#define TIME_SERVER_CONNECTING 0
#define TIME_SERVER_LOCAL 1
#define TIME_SERVER_SNTP 2

#define W_PASSTIME_INTERVAL 10
#define W_PASSTIME_MINUS 50

#define DEFAULT_CHATTING_TIMEOUT 10 // 1초 단위 0 치팅없슴
#define DEFAULT_NUM_AUTO_SERO 8     //
#define DEFAULT_NUM_STABLE_BUFFER 4 //
#define DEFAULT_ZERO_CALI_BAND 20   // 0.5g 디폴터
#define DEFAULT_STABLE_BAND 100     // 10g 디폴터

#define DEFAULT_STANDARD_WEIGHT 500 // 500g 기준
#define DEFAULT_MAX_FACTOR 300      // 300 기준
#define DEFAULT_MAX_WEIGHT 999999   // 999999g
#define DEFALUT_LOADCELL_SPEC 10000  // 10000g

/*workInfo.work_state */
typedef enum
{
  NO_JOB_NO_WORKER = 0, /* 초기값 */
  NO_JOB,               /* 작업자등록 일 없슴*/
  W_TAGGING,            /* 태그 인식*/
  W_TAGGING_OK,         /* tagging ok */
  W_TAGGING_ERR,        /* tagging error*/
  W_WAITING_START,      /* 물건 올리긴전 */
  W_START,              /* 측정 시작 */
  W_OVER,               /* 무개 넘침 */
  W_UNDER,              /* 무개 적음 */
  W_BAND_OK,            /* 무개 적정 */
  W_COMPLETE,           /* 무개 완료 */
  W_CHEATING,           /* 치팅방지 구간 */
  W_POST,               /* POST 일 끝 */
  W_END_CONFORM,        /* 일끝 메세지 확인*/
  W_DEFACTIVE_REG,      /* 불량등록*/
  W_DEFACTIVE_OK,       /* 불량 메세지 확인*/
  W_POST_DEFACTIVE,     /* 불량 날림*/
  W_JOB_END
} WORK_STATE;

esp_err_t getTagData(uint8_t tag[5]);
esp_err_t postWorkData(void);
esp_err_t postfaultyData(void);
esp_err_t getTimeLocal(void);

void http_task(void *pvParameters);

esp_err_t start_file_server(const char *base_path);
float getUnitMinMax(void); // debug
#if 0

//5E:92:DC:90


201	
Created
401	
Unauthorized
403	
Forbidden
404	
잘못된 접속 경로입니다.

500	
오류가 발생하였습니다.

501	
등록되지 않은 태그 정보입니다.

502	
등록되지 않은 작업 일련번호입니다.

503	
생산 완료된 작업입니다.

{
  "result": 200,
  "message": "정상",
  "data": {
    "workInfo": {
      "rowno": 1,
      "work_seq": 82,
      "work_no": "20220620000001",
      "work_name": "apple_100g",
      "name": "css",
      "product_weight": 100,
      "minimum_weight": 95,
      "maximum_weight": 105,
      "work_quantity": 10,
      "production_cnt": 0,
      "work_line": "4",
      "work_line_txt": "작업라인04"
    }
  }
}

typedef struct _PROGRESS_DATA
{
  uint32_t idx;
  char name[TASK_NAME_LENGTH];
  uint8_t code[PRGS_CODE_LENGTH];
  uint8_t date[PRGS_DATA_LENGTH];
  uint8_t seq;
  uint8_t status;
  uint8_t inventory_code[INVT_CODE_LENGTH];
  uint8_t worker_group_code[WORKER_GROUP_CODE_LENGTH];
  uint32_t container_weight;
  uint32_t stock_perfect_weight;
  uint32_t stock_defect_weight;
  uint32_t product_target_weight;
  uint32_t product_min_weight;
  uint32_t product_max_weight;
  uint32_t quantity_target;  /* target amount 목표 수량*/
  uint32_t quantity_current; /* cur_amount 현재 진행된 수량*/
  uint8_t started;
  uint8_t finished;
  uint8_t created[LIFE_TIME_LENGTH];
  uint8_t updated[LIFE_TIME_LENGTH];
  uint8_t food_category_code;
  uint8_t food_category_name[FOOD_CATEGORY_NAME];
  uint8_t food_code[6];
  uint8_t food_name[20];
  uint32_t food_unit_weight;
  uint32_t inventory_stock;
  uint32_t product_each_pcs;
  uint32_t total_weight;
  //uint8_t LED_step;
} PROGRESS_DATA;

PROGRESS_DATA taskInfo;

#endif

#endif

/**
 * @brief
 *
 */
#if 0

/* ?????? */
{
	"user" : {
		"idx" : 2,
		"uuid" : {
			"type" : "Buffer",
			"data" : [ 129, 210, 107, 159, 36, 5, 71, 55, 164, 16, 233, 121, 33, 232, 16, 229 ]
		},
		"login_id" : "",
		"login_pw" : "",
		"type" : 3,
		"name" : "?????",
		"email" : "",
		"phone" : "",
		"tag_serial" : "5E92DC90",
		"visible" : 0,
		"created" : "2022-02-18T21:24:20.000Z",
		"updated" : "2022-02-22T19:44:26.000Z"
	},
	"worker" :
		{
			"group_code" : "LINE0001",
			"group_name" : "???????01"
		}
}

type AutoGenerated struct {
	User struct {
		Idx  int `json:"idx"`
		UUID struct {
			Type string `json:"type"`
			Data []int  `json:"data"`
		} `json:"uuid"`
		LoginID   string    `json:"login_id"`
		LoginPw   string    `json:"login_pw"`
		Type      int       `json:"type"`
		Name      string    `json:"name"`
		Email     string    `json:"email"`
		Phone     string    `json:"phone"`
		TagSerial string    `json:"tag_serial"`
		Visible   int       `json:"visible"`
		Created   time.Time `json:"created"`
		Updated   time.Time `json:"updated"`
	} `json:"user"`
	Worker struct {
		GroupCode string `json:"group_code"`
		GroupName string `json:"group_name"`
	} `json:"worker"`
}

/* ??? ????*/
{
	"result" : "0", "error": {}
}
type AutoGenerated struct {
	Result string `json:"result"`
	Error  struct {
	} `json:"error"`
}
#endif

/**
 * @brief 서버 1 프로토콜
 *
 */
#if 0

<<<<< POST >>>>>

응답 : 
{
    "result":   "응답코드",
    "message":  "응답메세지",
    "data": {
            "work_seq":"작업 일련번호",
            "work_name":"작업명",
            "product_weight":"제품무게(g)",
            "minimum_weight":"최소무게(g)",
            "maximum_weight":"최대무게(g)",
            "work_quantity":"작업수량",
        "production_cnt":"총 생산수량",
        "work_status":"작업 상태 (W:대기, I:생산중, E:완료)",
    }
}

<request>
productionReqDTO 

{
  "production_weight": 500,
  "tag_id": "4E:F8:D8:90",
  "work_seq": 48
}

<Responses>
Code	Description
200	
정상

Example Value
Model
{
  "data": {},
  "message": "string",
  "result": 0
}

<<<<< GET >>>>>
get 예제
http://lauren.dev.aend.co.kr/api/work/4E%3AF8%3AD8%3A90

{
  "result": 200,
  "message": "정상",
  "data": {
    "workInfo": {
      "work_seq": 6,
      "work_no": "20220520000003",
      "work_name": "외계인이메로나_300g",
      "product_weight": 300,
      "minimum_weight": 200,
      "maximum_weight": 600,
      "work_quantity": 100,
      "production_cnt": 0,
      "work_line": "1",
      "work_line_txt": "작업라인01"
    }
  }
}


응답 : 
{
    "result":   "응답코드",
    "message":  "응답메세지",
    "data": {
        "workInfo": {
            "work_seq":"작업 일련번호",
            "work_no":"작업번호",
            "work_name":"작업명",
            "product_weight":"제품무게(g)",
            "minimum_weight":"최소무게(g)",
            "maximum_weight":"최대무게(g)",
            "work_quantity":"작업수량",
            "production_cnt":"총 생산수량",
            "work_line":"작업라인(CODE)",
            "work_line_txt":"작업라인(NAME)",
        }
    }
}

ProductionReqDTO{
description:	
생산 DTO

production_weight*	string
example: 500
생산 무게

tag_id*	string
example: 4E:F8:D8:90
태그 아이디

work_seq*	string
example: 48
작업 일련번호

}


Response{
data	{
}

message	string
result	integer($int32)
}

#endif
