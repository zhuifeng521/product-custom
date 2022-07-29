/*
 * @Author: xuan.li xuan.li@transsion.com
 * @Date: 2022-07-24 10:54:01
 * @LastEditors: xuan.li xuan.li@transsion.com
 * @LastEditTime: 2022-07-29 21:43:14
 * @FilePath: \product-custom\product_custom.c
 * @Description:
 *
 * Copyright (c) 2022 by xuan.li xuan.li@transsion.com, All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "product_custom.h"

unsigned int userEveBuf[MSG_MBB_MAX_LEN];

pthread_mutex_t userMutex;
sem_t fullMsg;
sem_t emptyMsg;
/**
 * @brief
 *
 */
void userEve_init(void)
{
    memset(userEveBuf, 0, sizeof(userEveBuf));
    pthread_mutex_init(&userMutex, NULL);
    sem_init(&fullMsg, 0, MSG_MBB_MAX_LEN);
    sem_init(&emptyMsg, 0, 0);
#if 0
    if(!pthread_mutex_init(&userMutex,NULL) || !sem_init(&fullMsg,0,MSG_MBB_MAX_LEN) || !sem_init(&emptyMsg,0,0))
    {
        return ;
    }
#endif
}
/**
 * @brief
 *
 * @param msg
 */
void userEve_setMsg(unsigned int msg)
{
    for (int i = 0; i < MSG_MBB_MAX_LEN; i++)
    {
        if (userEveBuf[i] == 0)
        {
            sem_wait(&fullMsg);
            pthread_mutex_lock(&userMutex);
            printf("userEve_setMsg = %d\n", msg);
            userEveBuf[i] = msg;
            pthread_mutex_unlock(&userMutex);
            sem_post(&emptyMsg);
            break;
        }
    }
}
/**
 * @brief
 *
 * @return unsigned int
 */
unsigned int userEve_sendMsg(void)
{
    unsigned int tempMsg;
    while (1)
    {
        for (int i = 0; i < MSG_MBB_MAX_LEN; i++)
        {
            if (userEveBuf[i] != 0)
            {
                tempMsg = userEveBuf[i];
                sem_wait(&emptyMsg);
                pthread_mutex_lock(&userMutex);
                printf("tempMsg = %d\n", tempMsg);
                userEveBuf[i] = 0;
                pthread_mutex_unlock(&userMutex);
                sem_post(&fullMsg);
                return tempMsg;
            }
        }
    }
}
/**
 * @brief
 *
 * @param data
 */
void userEve_handlerMsg(void *data)
{
    printf("userEve_handlerMsg = %d\n", userEve_sendMsg());
    for (;;)
    {
        unsigned int msg = userEve_sendMsg();
        switch (msg)
        {
        case MSG_MBB_GET_CURDEVICE_SPEED_INFO:
            sleep(1);
            printf("getCurDeviceSpeedInfo\n");
            break;

        default:
            break;
        }
    }
}
/**
 * @brief
 *
 */
void userEve_destroyMsg(void)
{
    pthread_mutex_destroy(&userMutex);
    sem_destroy(&fullMsg);
    sem_destroy(&emptyMsg);
}
/**
 * @brief
 *
 * @param data
 */
void userEve_analysisData(void *data)
{
    cJSON *servicesId = (cJSON *)data;
    printf("userEve_analysisData\n");
    if (strcmp(servicesId->valuestring, "getCurDeviceSpeedInfo") == 0)
    {
        userEve_setMsg(MSG_MBB_GET_CURDEVICE_SPEED_INFO);
    }
    else
    {
    }
}
/**
 * @brief
 *
 * @param data
 * @param len
 */
void userEve_createProcess(char *data, int len)
{

    pthread_t handlerMsgPthread;
    cJSON *root = cJSON_Parse(data);
    cJSON *param = cJSON_GetObjectItem(root, "params");

    pthread_create(&handlerMsgPthread, 0, (void *)userEve_handlerMsg, (void *)param);
    pthread_join(handlerMsgPthread, NULL);
}
/**
 * @brief
 *
 * @param data
 * @param len
 */
void userEve_createClient(char *data, int len)
{
    pthread_t recMsgPthread;
    cJSON *root = cJSON_Parse(data);
    cJSON *services = cJSON_GetObjectItem(root, "servicesId");
    pthread_create(&recMsgPthread, 0, (void *)userEve_analysisData, (void *)services);
    pthread_join(recMsgPthread, NULL);
}
/**
 * @brief
 *
 */
void main(void)
{
    char *temp = "{\"params\":{},\"servicesId\":\"getCurDeviceSpeedInfo\"}";
    userEve_init();
    userEve_createProcess(temp, strlen(temp));
    userEve_createClient(temp, strlen(temp));
    return;
}
