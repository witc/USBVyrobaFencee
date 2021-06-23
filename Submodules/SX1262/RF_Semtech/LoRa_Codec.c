/*
 * JustLoRa.c
 *
 *  Created on: 25. 4. 2018
 *      Author: ropek
 *
 *      __			  __    __       _	   _ _ _ _ _ _ _
 *      \ \			 / /   |  \     | |   |_ _ _ _ _ _ _|
 *       \ \    	/ /    | \ \    | |         | |
 *		  \	\  	   / /     | |\ \   | |         | |
 *         \ \    / /	   | | \ \  | |         | |
 *          \ \  / /	   | |  \ \ | |         | |
 *           \ \/ /		   | |   \ \| |         | |
 *            \__/         |_|    \___|         |_|			s.r.o
 *
 *        	  2018
 *
 */
#include "radio_general.h"
#include "radio_user.h"
#include "main.h"
#include "LoRa_Codec.h"
#include "radio.h"
#include "sx126x.h"

extern volatile osMessageQId QueueCoreHandle;
extern volatile osMessageQId QueueTaskRFHandle;

