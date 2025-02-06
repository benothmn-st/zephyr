/*
 * Copyright (c) 2023 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
#include "ll_sys.h"

#define LOG_LEVEL CONFIG_SOC_LOG_LEVEL
LOG_MODULE_REGISTER(ll_sys_if_adapt);

#define LL_THREAD_STACK_SIZE (1024 * 2)
#define LL_THREAD_PRIO (5)
K_THREAD_STACK_DEFINE(ll_stack_area, LL_THREAD_STACK_SIZE);

extern struct k_mutex ll_ctrl_mutex;
K_SEM_DEFINE(ll_semaphore, 0, 1);

static void ll_thread(void *arg1, void *arg2, void *arg3)
{
	while(1) {
		k_sem_take(&ll_semaphore, K_FOREVER);
		k_mutex_lock(&ll_ctrl_mutex, K_FOREVER);
		ll_sys_bg_process();
		k_mutex_unlock(&ll_ctrl_mutex);
	}
}

void ll_sys_schedule_bg_process(void)
{
	k_sem_give(&ll_semaphore);
}

void ll_sys_schedule_bg_process_isr(void)
{
	k_sem_give(&ll_semaphore);
}

void ll_sys_bg_process_init(void)
{
	
}

K_THREAD_DEFINE(ll_thread_id, K_THREAD_STACK_SIZEOF(ll_stack_area), ll_thread, NULL, NULL, NULL, LL_THREAD_PRIO, 0, 0);
