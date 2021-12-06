#include "TskTransUart.h"

char 						cmdStr[512];
char 						word[64];
char 						dbgStr[200];
const int tskTransUartPrio = 9;
const int tskTransUartStkSize = 16384;

const int tskDbgUartPrio = 8;
const int tskDbgUartStkSize = 8192;

static TaskHandle_t tskTransUartHandle;
static TaskHandle_t tskDbgUartHandle;

extern QueueHandle_t 		tskTopMsgQ;

extern XillUartps*			uartTransSubs;
extern XilIntc* 			intc;
SemaphoreHandle_t 			SemRecvCmd;
QueueHandle_t 				tskDbgUartMsgQ;

void getl(char *line)
{
    int c;
    if(line == NULL)
        return;

    while(1)
    {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if((*line++ = c) == '\r')
            break;
    }
    *line = '\0';
    return;
}

int getWord(const char *cmd, int p, char *word)
{
    while(true)
    {
        if(cmd[p] == ' ' || cmd[p] == '\r' || cmd[p] == '\n')
            p++;
        else if(cmd[p] == '\0')
        {
            word[0] = '\0';
            return p;
        }
        else
            break;
    }
    int i;
    for(i = 0; cmd[p + i] != ' ' && cmd[p + i] != '\r' &&
            cmd[p + i] != '\n' && cmd[p + i] != '\0'; i++)
    {
        word[i] = cmd[p+i];
    }
    word[i] = '\0';
    p += i;
    return p;
}

void Puts(const char *str)
{
	printf(str);
}

void Puts(std::string s)
{
	printf(s.c_str());
}

void cmdHello(char *arg)
{
	Puts("Hi\r\n");
}

void cmdConfigFFT(char *arg)
{
	BaseType_t ret;
	int p = getWord(arg, 0, word);
	u32 param = (u32)atoi(word);
	if((param != 0) && (param != 1)) {
		Puts("Error parameters!\r\n");
		return;
	}
	tskTopMsg_t msg;
	msg = {ConfigIfFft, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Error!\r\n");
		return;
	}
	Puts("OK\r\n");
}

void cmdConfigMode(char *arg)
{
	BaseType_t ret;
	int p = getWord(arg, 0, word);
	u32 param = (u32)atoi(word);
	if((param < 0) || (param > MAX_MODE_NUM)){
		Puts("Error parameters!\r\n");
		return;
	}
	tskTopMsg_t msg;
	msg = {ConfigMode, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Error!\r\n");
		return;
	}
	Puts("OK\r\n");
}

void cmdConfigStartTime(char *arg)
{
	BaseType_t ret;
	u32 param;
	int lastP;
	int p;
	tskTopMsg_t msg;

	// Year
	p = getWord(arg, 0, word);
	param = (u32)atoi(word);
	if(param < 2021){
		Puts("Error parameter year!\r\n");
		return;
	}
	msg = {ConfigStartYear, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Year Error!\r\n");
		return;
	}
	lastP = p;

	// Month
	p = getWord(arg, p, word);
	if(p == lastP){
		Puts("Error parameter Number! No parameter after year.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 1) || (param > 12)){
		Puts("Error parameter month!\r\n");
		return;
	}
	msg = {ConfigStartMonth, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Month Error!\r\n");
		return;
	}
	lastP = p;


	// Day
	p = getWord(arg, p, word);
	if(p == lastP){
		Puts("Error parameter Number! No parameter after month.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 1) || (param > 31)){
		Puts("Error parameter day!\r\n");
		return;
	}
	msg = {ConfigStartDay, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Day Error!\r\n");
		return;
	}
	lastP = p;

	// Hour
	p = getWord(arg, p, word);
	if(p == lastP){
		Puts("Error parameter Number! No parameter after Day.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 0) || (param > 23)){
		Puts("Error parameter hour!\r\n");
		return;
	}
	msg = {ConfigStartHour, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Hour Error!\r\n");
		return;
	}
	lastP = p;

	// Minute
	p = getWord(arg, p, word);
	if(p == lastP){
		Puts("Error parameter Number! No parameter after hour.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 0) || (param > 59)){
		Puts("Error parameter minute!\r\n");
		return;
	}
	msg = {ConfigStartMin, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Minute Error!\r\n");
		return;
	}
	lastP = p;

	// Second
	p = getWord(arg, p, word);
	if((p == lastP) || (p == (lastP +1))){
		Puts("Error parameter Number! No parameter after minute.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 0) || (param > 59)){
		Puts("Error parameter second!\r\n");
		return;
	}
	msg = {ConfigStartSecond, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Second Error!\r\n");
		return;
	}

	Puts("OK\r\n");
}

void cmdConfigEndTime(char *arg)
{
	BaseType_t ret;
	u32 param;
	int lastP;
	int p;
	tskTopMsg_t msg;

	// Year
	p = getWord(arg, 0, word);
	param = (u32)atoi(word);
	if(param < 2021){
		Puts("Error parameter year!\r\n");
		return;
	}
	msg = {ConfigEndYear, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Year Error!\r\n");
		return;
	}
	lastP = p;

	// Month
	p = getWord(arg, p, word);
	if(p == lastP){
		Puts("Error parameter Number! No parameter after year.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 1) || (param > 12)){
		Puts("Error parameter month!\r\n");
		return;
	}
	msg = {ConfigEndMonth, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Month Error!\r\n");
		return;
	}
	lastP = p;


	// Day
	p = getWord(arg, p, word);
	if(p == lastP){
		Puts("Error parameter Number! No parameter after month.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 1) || (param > 31)){
		Puts("Error parameter day!\r\n");
		return;
	}
	msg = {ConfigEndDay, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Day Error!\r\n");
		return;
	}
	lastP = p;

	// Hour
	p = getWord(arg, p, word);
	if(p == lastP){
		Puts("Error parameter Number! No parameter after Day.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 0) || (param > 23)){
		Puts("Error parameter hour!\r\n");
		return;
	}
	msg = {ConfigEndHour, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Hour Error!\r\n");
		return;
	}
	lastP = p;

	// Minute
	p = getWord(arg, p, word);
	if(p == lastP){
		Puts("Error parameter Number! No parameter after hour.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 0) || (param > 59)){
		Puts("Error parameter minute!\r\n");
		return;
	}
	msg = {ConfigEndMin, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Minute Error!\r\n");
		return;
	}
	lastP = p;

	// Second
	p = getWord(arg, p, word);
	if((p == lastP) || (p == (lastP +1))){
		Puts("Error parameter Number! No parameter after minute.\r\n");
		return;
	}
	param = (u32)atoi(word);
	if((param < 0) || (param > 59)){
		Puts("Error parameter second!\r\n");
		return;
	}
	msg = {ConfigEndSecond, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Second Error!\r\n");
		return;
	}

	Puts("OK\r\n");
}

void cmdConfigSecondsIncrease(char *arg)
{
	BaseType_t ret;
	int p = getWord(arg, 0, word);

	u32 param = (u32)atoi(word);
	if(param<=0) {
		Puts("Error parameters!\r\n");
		return;
	}
	tskTopMsg_t msg;
	msg = {ConfigSecondsIncrease, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Error!\r\n");
		return;
	}
	Puts("OK\r\n");
}

void cmdSetStart(char *arg)
{
	BaseType_t ret;
	int p = getWord(arg, 0, word);

	u32 param = (u32)atoi(word);
	if((param != 0) && (param != 1)) {
		Puts("Error parameters!\r\n");
		return;
	}
	tskTopMsg_t msg;
	msg = {ConfigIfFft, param};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Error!\r\n");
		return;
	}
	msg = {SetStart, 1};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Error!\r\n");
		return;
	}
	Puts("OK\r\n");
}

void cmdSetStop(char *arg)
{
	BaseType_t ret;
	tskTopMsg_t msg;
	msg = {SetStart, 0};
	ret = xQueueSend(tskTopMsgQ, (void *)&msg, (TickType_t)10);
	if(ret != pdPASS){
		Puts("Config Error!\r\n");
		return;
	}
	Puts("OK\r\n");
}


void cmdHelp(char *arg)
{
	Puts("\r\n");
    Puts("############################################\r\n");
    Puts("########     SDP DAQ Sub-System     ########\r\n");
    Puts("############################################\r\n");
    Puts("\r\n");
    Puts("/************* Operation Mode *************/\r\n");
    Puts("Command: CONFIGMODE MODE_ID\r\n");
    Puts("\r\n");
    Puts("MODE_ID: \r\n");
    Puts("0       Idle\r\n");
    Puts("1       Test ADC\r\n");
    Puts("2       Test GPS\r\n");
	Puts("3       Test SD Card\r\n");
	Puts("4       Test FFT\r\n");
	Puts("5       Test Acquisition Data\r\n");
	Puts("6       Test Acquisition FFT\r\n");
    Puts("\r\n");
    Puts("\r\n");
    Puts("/**************** Start Time **************/\r\n");
    Puts("Command: CONFIGST YEAR MONTH DAY HOUR MINUTE SECOND\r\n");
    Puts("\r\n");
    Puts("YEAR: 2021~9999\r\n");
    Puts("MONTH: 1~12\r\n");
    Puts("DAY: 1~31\r\n");
    Puts("HOUR: 0~23\r\n");
    Puts("MINUTE: 0~59\r\n");
    Puts("SECOND: 0~59\r\n");
    Puts("\r\n");

    Puts("\r\n");
    Puts("/***************** End Time ***************/\r\n");
    Puts("Command: CONFIGET YEAR MONTH DAY HOUR MINUTE SECOND\r\n");
    Puts("\r\n");
    Puts("YEAR: 2021~9999\r\n");
    Puts("MONTH: 1~12\r\n");
    Puts("DAY: 1~31\r\n");
    Puts("HOUR: 0~23\r\n");
    Puts("MINUTE: 0~59\r\n");
    Puts("SECOND: 0~59\r\n");
    Puts("\r\n");

	Puts("\r\n");
    Puts("/**************** Config Si ***************/\r\n");
    Puts("Command: CONFIGSI COUNT\r\n");
    Puts("\r\n");
    Puts("COUNT: \r\n");
	puts("Time Count for Seconds per file");
    Puts("\r\n");

    Puts("\r\n");
    Puts("/**************** Set Start ***************/\r\n");
    Puts("Command: SETSTART FLAG\r\n");
    Puts("\r\n");
    Puts("FLAG: \r\n");
    Puts("0       Disable FFT\r\n");
    Puts("1       Enable FFT\r\n");
    Puts("\r\n");

    Puts("\r\n");
    Puts("/***************** Set Stop ***************/\r\n");
    Puts("Command: SETSTOP\r\n");
    Puts("\r\n");
    Puts("Stop the acquisition immediately\r\n");
    Puts("\r\n");
}


const int cmdNum = 8;
FuncAndCmd funcmd[cmdNum] = {
    {cmdHello,       		"HELLO"},
	{cmdConfigMode,   		"CONFIGMODE"},
	{cmdHelp,				"HELP"},
	{cmdConfigStartTime,	"CONFIGST"},
	{cmdConfigEndTime,		"CONFIGET"},
	{cmdConfigSecondsIncrease,"CONFIGSI"},
	{cmdSetStart,			"SETSTART"},
	{cmdSetStop,			"SETSTOP"}
};

void uartTransSubsIsr(void *CallBackRef, u32 Event, u32 EventData)
{
	int ret;
    BaseType_t wake = 0;
    u32 IsrStatus;

    if (Event == XUARTPS_EVENT_RECV_DATA)
    {
		ret = xSemaphoreGiveFromISR(SemRecvCmd, &wake);
		portYIELD_FROM_ISR(wake);
    }

}

static void tskTransUart(void *arg)
{
    int rtn;
	Puts("###############################\r\n"); vTaskDelay(50);
	Puts("# SDP DAQ-SubSys Uart Console #\r\n"); vTaskDelay(50);
	Puts("###############################\r\n"); vTaskDelay(50);
    fflush(stdin);
    tskTopMsg_t msg2Top;

	while(1)
	{
		// Receive command from transmission sub-system
		xSemaphoreTake(SemRecvCmd, portMAX_DELAY);
        getl(cmdStr);
        int i;
        int p = getWord(cmdStr, 0, word);
        if(word[0] != '\0')
        {
            for(i = 0; i < cmdNum; i++)
            {
                if(!strcmp(funcmd[i].cmd, word))
                {
                    funcmd[i].func(cmdStr + p);
                    break;
                }
            }
            if(i >= cmdNum)
            {
            	sprintf(dbgStr, "Undefined command %s.\r\n", word);
            	Puts(dbgStr);
                vTaskDelay(50);
            }
        }
        fflush(stdin);



        vTaskDelay(50);
	}
}


static void tskDbgUart(void *arg)
{
	int ret;
	tskDbgUartMsg_t recvedMsg;
	while(1)
	{
		// Receive message from other task
		if(xQueueReceive(tskDbgUartMsgQ, &recvedMsg, portMAX_DELAY))
		{
			dbgStr[0] = '\0';
			char* ptr = (char*)(recvedMsg.address);
			for(int i = 0; i < recvedMsg.size; i++){
				dbgStr[i] = *ptr++;
			}
			dbgStr[recvedMsg.size] = '\0';
			Puts(dbgStr);
		}

		 vTaskDelay(50);
	}
}

void tskTransUartInit()
{
	int ret;
    SemRecvCmd = xSemaphoreCreateBinary();
    configASSERT(SemRecvCmd);

    tskDbgUartMsgQ = xQueueCreate(30, sizeof(tskDbgUartMsg_t));
	configASSERT(tskDbgUartMsgQ);

    uartTransSubs = new XillUartps(XPAR_PS7_UART_1_DEVICE_ID, 115200, intc, XPAR_XUARTPS_1_INTR, uartTransSubsIsr);
    uartTransSubs->XilUartpsSetIntMask(XUARTPS_IXR_RXOVR);
    uartTransSubs->XilUartpsSetFifoThreshold(10);

    ret = xTaskCreate(tskTransUart, "tskTransUart", tskTransUartStkSize, NULL, tskTransUartPrio, &tskTransUartHandle);
    configASSERT(ret == pdPASS);

    ret = xTaskCreate(tskDbgUart, "tskDbgUart", tskDbgUartStkSize, NULL, tskDbgUartPrio, &tskDbgUartHandle);
    configASSERT(ret == pdPASS);
}




