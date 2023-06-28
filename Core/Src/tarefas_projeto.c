/*
 * tarefas_projeto.c
 *
 *  Created on: Jun 26, 2023
 *      Author: icaro
 */

#include "tarefas_projeto.h"

/* Tarefas Periódicas */
extern TaskHandle_t taskTemperHandle;
extern TaskHandle_t taskPresenHandle;
extern TaskHandle_t taskGerLuzHandle;
extern TaskHandle_t taskMonSegHandle;

/* Tarefas Aperiódicas */
extern TaskHandle_t taskLuzPreHandle;
extern TaskHandle_t taskLigAlaHandle;

/* Variáveis Globais */
TickType_t lastAltTimeD;
TickType_t OUTDTime;
volatile uint8_t estadoOUTD = GPIO_PIN_RESET;

/* Semáforos */
extern SemaphoreHandle_t netMutex;

/* Variáveis Compartilhadas */
char networkMsgBuffer[NET_MSG_BUFFER + 1];

/* Funções auxiliares */
void ledToggle(void *pvParameters)
{
	TickType_t timeStamp;
	char clockWatchBuf[CLOCK_WATCH_MAX_CHAR + 1];

	while(1)
	{
		timeStamp = xTaskGetTickCount();
		toClockTime(timeStamp, clockWatchBuf);

		printf("[TASK Debug] [time: %s] DEBUG!\n", clockWatchBuf);
		HAL_GPIO_TogglePin(OUTA_GPIO_Port, OUTA_Pin);
		vTaskDelay(500 * (configTICK_RATE_HZ / 1000));

		HAL_GPIO_TogglePin(OUTA_GPIO_Port, OUTA_Pin);
		HAL_GPIO_TogglePin(OUTB_GPIO_Port, OUTB_Pin);
		vTaskDelay(500 * (configTICK_RATE_HZ / 1000));

		HAL_GPIO_TogglePin(OUTB_GPIO_Port, OUTB_Pin);
		HAL_GPIO_TogglePin(OUTC_GPIO_Port, OUTC_Pin);
		vTaskDelay(500 * (configTICK_RATE_HZ / 1000));

		HAL_GPIO_TogglePin(OUTC_GPIO_Port, OUTC_Pin);
		HAL_GPIO_TogglePin(OUTD_GPIO_Port, OUTD_Pin);
		vTaskDelay(500 * (configTICK_RATE_HZ / 1000));

		HAL_GPIO_TogglePin(OUTD_GPIO_Port, OUTD_Pin);
	}
}

void toClockTime(TickType_t sysTicks, char* clk_string)
{
	uint16_t seconds, minutes, hours;

	seconds = (sysTicks / configTICK_RATE_HZ) % 60;
	minutes = (sysTicks / (configTICK_RATE_HZ * 60)) % 60;
	hours = (sysTicks / (configTICK_RATE_HZ * 3600)) % 24;

	snprintf(clk_string, CLOCK_WATCH_MAX_CHAR+1,"%02u: %02u: %02u", hours, minutes, seconds);
}

void initRandomValues(void)
{
	time_t t_random;
	srand((unsigned) &t_random);
}

int getRandomValue(int min, int max)
{
	int value = (rand() % (min - max + 1)) + min;

	return value;
}

/* Tarefas Periódicas */

void lerTemperatura(void* pvParameters)
{
	TickType_t timeStamp;
	int temperValue;
	char clockWatchBuf[CLOCK_WATCH_MAX_CHAR + 1];

	while(1)
	{
		timeStamp = xTaskGetTickCount();
		toClockTime(timeStamp, clockWatchBuf);
		temperValue = getRandomValue(23, 28);

		printf("[TASK Temper] [time: %s] Temperatura: %02d\n", clockWatchBuf, temperValue);

		/* Início da seção crítica */
		/* Fim da seção crítica */

		vTaskDelay(2000 * (configTICK_RATE_HZ / 1000));
	}
}

void monitorarPresenca(void* pvParameters)
{
	/* Monitoramento de Presença feito com o BTNA */
	TickType_t timeStamp;
	char clockWatchBuf[CLOCK_WATCH_MAX_CHAR + 1];

	GPIO_PinState button_state = GPIO_PIN_RESET;
	uint8_t machine_state = 0;

	while(1)
	{
		timeStamp = xTaskGetTickCount();
		toClockTime(timeStamp, clockWatchBuf);
		button_state = HAL_GPIO_ReadPin(BTNA_GPIO_Port, BTNA_Pin);

		switch(machine_state)
		{
		case 0:
			printf("[TASK Presen] [time: %s] Presença Não Detectada\n", clockWatchBuf);

			if (button_state == GPIO_PIN_SET)
			{
				machine_state = 1;
			}
			break;

		case 1:
			printf("[TASK Presen] [time: %s] Presença Detectada\n", clockWatchBuf);
			xTaskNotifyGive(taskGerLuzHandle);
			xTaskNotifyGive(taskLuzPreHandle);

			if (button_state == GPIO_PIN_RESET)
			{
				machine_state = 0;
			}
			break;

		default:
			break;
		}

		vTaskDelay(250 * (configTICK_RATE_HZ / 1000));
	}
}

void gerenciarLuzes(void* pvParameters)
{
	/* A cada 1 minutos alterna a luz A -> OUTA */
	/* A cada 2 minutos alterna a luz B -> OUTB */
	/* A cada 4 minutos alterna a luz C -> OUTC */
	/* A cada 8 minutos alterna a luz D -> OUTD */
	/* Se a presença for detectada no cômodo D,
	 * deve ser ligado a luz D, desabilitando a
	 * alternância da luz até a presença sumir */
	TickType_t timeStamp;
	char clockWatchBuf[CLOCK_WATCH_MAX_CHAR + 1];

	BaseType_t notificacaoPresenca;

	TickType_t OUTATime = configTICK_RATE_HZ * 60;		// 1 minuto em ticks
	TickType_t OUTBTime = configTICK_RATE_HZ * 120;		// 2 minutos em ticks
	TickType_t OUTCTime = configTICK_RATE_HZ * 240;		// 4 minutos em ticks
	OUTDTime = configTICK_RATE_HZ * 480;				// 8 minutos em ticks

	TickType_t lastAltTimeA = xTaskGetTickCount();
	TickType_t lastAltTimeB = xTaskGetTickCount();
	TickType_t lastAltTimeC = xTaskGetTickCount();
	lastAltTimeD = xTaskGetTickCount();

	while(1)
	{
		timeStamp = xTaskGetTickCount();
		toClockTime(timeStamp, clockWatchBuf);

		// Alternar o OUTA a cada 1 minuto
		if ((timeStamp - lastAltTimeA) >= OUTATime)
		{
			HAL_GPIO_TogglePin(OUTA_GPIO_Port, OUTA_Pin);
			printf("[TASK gerLuz] [time: %s] Passou 1 minuto: Alternando LUZ A\n", clockWatchBuf);
			lastAltTimeA = xTaskGetTickCount();
		}

		// Alternar o OUTB a cada 2 minutos
		if ((timeStamp - lastAltTimeB) >= OUTBTime)
		{
			HAL_GPIO_TogglePin(OUTB_GPIO_Port, OUTB_Pin);
			printf("[TASK gerLuz] [time: %s] Passaram 2 minutos: Alternando LUZ B\n", clockWatchBuf);
			lastAltTimeB = xTaskGetTickCount();
		}

		// Alternar o OUTC a cada 4 minutos
		if ((timeStamp - lastAltTimeC) >= OUTCTime)
		{
			HAL_GPIO_TogglePin(OUTC_GPIO_Port, OUTC_Pin);
			printf("[TASK gerLuz] [time: %s] Passaram 4 minutos: Alternando LUZ C\n", clockWatchBuf);
			lastAltTimeC = xTaskGetTickCount();
		}

		/* Alternar o OUTD a cada 4 minutos ou se houver presença no
		 * cômodo D, ligue OUTD sem alternar, caso esteja desligado */
		notificacaoPresenca = ulTaskNotifyTake(pdTRUE, 0);

		if (!notificacaoPresenca && (timeStamp - lastAltTimeD) >= OUTDTime)
		{
			estadoOUTD = ~estadoOUTD;
			HAL_GPIO_WritePin(OUTD_GPIO_Port, OUTD_Pin, estadoOUTD);
			printf("[TASK gerLuz] [time: %s] Passaram %lu minutos: Alternando LUZ D\n",
					clockWatchBuf, (OUTDTime / configTICK_RATE_HZ) / 60);
			lastAltTimeD = xTaskGetTickCount();

			OUTDTime = configTICK_RATE_HZ * 480;
		}

		vTaskDelay(100 * (configTICK_RATE_HZ / 1000));
	}
}

void monitorarSeguranca(void* pvParameters)
{
	/* Monitoramento de Presença feito com o BTNB */
	TickType_t timeStamp;
	char clockWatchBuf[CLOCK_WATCH_MAX_CHAR + 1];

	GPIO_PinState button_state = GPIO_PIN_RESET;
	uint8_t machine_state = 0;

	while(1)
	{
		timeStamp = xTaskGetTickCount();
		toClockTime(timeStamp, clockWatchBuf);
		button_state = HAL_GPIO_ReadPin(BTNB_GPIO_Port, BTNB_Pin);

		switch(machine_state)
		{
		case 0:
			printf("[TASK monSeg] [time: %s] Intruso Não Detectado\n", clockWatchBuf);

			if (button_state == GPIO_PIN_SET)
			{
				machine_state = 1;
			}
			break;

		case 1:
			printf("[TASK monSeg] [time: %s] Intruso Detectado!\n", clockWatchBuf);
			xTaskNotifyGive(taskLigAlaHandle);

			if (button_state == GPIO_PIN_RESET)
			{
				machine_state = 0;
			}
			break;

		default:
			break;
		}

		vTaskDelay(250 * (configTICK_RATE_HZ / 1000));
	}
}

void enviarDadosViaRede(void* pvParameters)
{

}

/* Tarefas Aperiódicas */
void ligarAlarme(void* pvParameters)
{
	TickType_t timeStamp;
	char clockWatchBuf[CLOCK_WATCH_MAX_CHAR + 1];

	GPIO_PinState button_state = GPIO_PIN_RESET;
	uint8_t machine_state = 0;
	uint8_t alarmLock = 0;

	BaseType_t notificacaoSeguranca;

	TaskStatus_t segurancaTaskInfo;

	while(1)
	{
		timeStamp = xTaskGetTickCount();
		toClockTime(timeStamp, clockWatchBuf);

		notificacaoSeguranca = ulTaskNotifyTake(pdTRUE, 0);
		vTaskGetInfo(taskMonSegHandle, &segurancaTaskInfo, pdTRUE, eInvalid);

		if(notificacaoSeguranca)
		{
			/* Suspende a tarefa de monitorarSeguranca até apertar o
			 * botao de seguranca outra vez */
			printf("[TASK LigAla] [time: %s] Notificação de [%s] -> Ligar Alarme -> Suspender sensor\n",
					clockWatchBuf, segurancaTaskInfo.pcTaskName);
			vTaskSuspend(taskMonSegHandle);
			vTaskDelay(10 * (configTICK_RATE_HZ / 1000));
			alarmLock = pdTRUE;
		}

		if(alarmLock)
		{
			button_state = HAL_GPIO_ReadPin(BTNB_GPIO_Port, BTNB_Pin);

			switch(machine_state)
			{
			case 0:
				printf("[TASK LigAla] [time: %s] ALARME ATIVO!\n", clockWatchBuf);

				if (button_state == GPIO_PIN_SET)
				{
					/*Se o botão de segurança for pressionado, volta a
					 * monitorar segurança outra vez e desliga o alarme */
					printf("[TASK LigAla] [time: %s] ALARME DESLIGADO!\n", clockWatchBuf);
					machine_state = 1;
				}
				break;

			case 1:
				if (button_state == GPIO_PIN_RESET)
				{
					alarmLock = pdFALSE;
					vTaskResume(taskMonSegHandle);
					vTaskDelay(10 * (configTICK_RATE_HZ / 1000));
					machine_state = 0;
				}
				break;

			default:
				break;
			}
		}

		vTaskDelay(250 * (configTICK_RATE_HZ / 1000));
	}
}

void ligarLuzPresenca(void* pvParameters)
{
	TickType_t timeStamp;
	char clockWatchBuf[CLOCK_WATCH_MAX_CHAR + 1];

	BaseType_t notificacaoPresenca;

	TaskStatus_t presencaTaskInfo;

	while(1)
	{
		timeStamp = xTaskGetTickCount();
		toClockTime(timeStamp, clockWatchBuf);

		notificacaoPresenca = ulTaskNotifyTake(pdTRUE, 0);
		vTaskGetInfo(taskPresenHandle, &presencaTaskInfo, pdTRUE, eInvalid);

		if(notificacaoPresenca && !estadoOUTD)
		{
			//Reiniciar o último registro de tempo
			lastAltTimeD = xTaskGetTickCount();

			estadoOUTD = ~estadoOUTD;
			printf("[TASK luzPre] [time: %s] Notificação de [%s] -> LIGAR Luz D\n",
					clockWatchBuf, presencaTaskInfo.pcTaskName);
			HAL_GPIO_WritePin(OUTD_GPIO_Port, OUTD_Pin, estadoOUTD);

			OUTDTime = configTICK_RATE_HZ * 60;
		}

		vTaskDelay(250 * (configTICK_RATE_HZ / 1000));
	}
}
