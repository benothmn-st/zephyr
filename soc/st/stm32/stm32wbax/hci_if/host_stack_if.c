/*
 * Copyright (c) 2023 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>

#include "blestack.h"
#include "bpka.h"
#include "ll_intf.h"

/* The thread stacks' sizes to be updated depending on the application
 * complexity and on the number of links.
 */
#define BPKA_THREAD_STACK_SIZE (1024 + 512)
#define LL_CTRL_THREAD_STACK_SIZE (1024 + 512)

#define BPKA_THREAD_PRIO (14)
#define LL_CTRL_THREAD_PRIO (14)

K_MUTEX_DEFINE(ll_ctrl_mutex);
K_SEM_DEFINE(ll_ctrl_semaphore, 0, 1);
K_SEM_DEFINE(bpka_semaphore, 0, 1);

K_THREAD_STACK_DEFINE(bpka_stack_area, BPKA_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(ll_ctrl_stack_area, LL_CTRL_THREAD_STACK_SIZE);

uint8_t ll_state_busy;

void HostStack_Process(void);

static void ll_ctrl_thread(void *arg1, void *arg2, void *arg3)
{
	uint8_t running = 0x00;
	change_state_options_t options;
	while(1) {
		k_sem_take(&ll_ctrl_semaphore, K_FOREVER);
		k_mutex_lock(&ll_ctrl_mutex, K_FOREVER);
		running = BleStack_Process();
		k_mutex_unlock(&ll_ctrl_mutex);

		if (ll_state_busy == 1) {
			options.combined_value = 0x0F;
			ll_intf_chng_evnt_hndlr_state(options);
			ll_state_busy = 0;
		}

		if (running == BLE_SLEEPMODE_RUNNING) {
			HostStack_Process();
		}
	}
}

static void bpka_thread(void *arg1, void *arg2, void *arg3)
{
	while(1) {
		k_sem_take(&bpka_semaphore, K_FOREVER);
		BPKA_BG_Process();
	}
}


void HostStack_Process(void)
{
	k_sem_give(&ll_ctrl_semaphore);
}

void BPKACB_Process(void)
{
	k_sem_give(&bpka_semaphore);
}

K_THREAD_DEFINE(ll_ctrl_thread_id, K_THREAD_STACK_SIZEOF(ll_ctrl_stack_area), ll_ctrl_thread, NULL, NULL, NULL, LL_CTRL_THREAD_PRIO, 0, 0);
K_THREAD_DEFINE(bpka_thread_id, K_THREAD_STACK_SIZEOF(bpka_stack_area), bpka_thread, NULL, NULL, NULL, BPKA_THREAD_PRIO, 0, 0);

