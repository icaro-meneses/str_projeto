/*
 * tarefas_projeto.h
 *
 *  Created on: Jun 26, 2023
 *      Author: icaro
 */

#ifndef INC_TAREFAS_PROJETO_H_
#define INC_TAREFAS_PROJETO_H_

#include "main.h"

#define CLOCK_WATCH_MAX_CHAR	10

/* Template */
void ledToggle(void *pvParameters);

/* Projeto */
void toClockTime(TickType_t sysTicks, char* clk_string);
void initRandomValues(void);
int getRandomValue(int min, int max);

/* Tarefas Periódicas */
void lerTemperatura(void* pvParameters);
void monitorarPresenca(void* pvParameters);
void gerenciarLuzes(void* pvParameters);
void monitorarSeguranca(void* pvParameters);
void enviarDadosViaRede(void* pvParameters);

/* Tarefas Aperiódicas */
void ligarAlarme(void* pvParameters);
void ligarLuzPresenca(void* pvParameters);

#endif /* INC_TAREFAS_PROJETO_H_ */
