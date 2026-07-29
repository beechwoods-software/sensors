/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */


#pragma once

/**
 * @file sevensegment.h
 * @brief Seven-segment display helpers
 *
 * Small API to initialize and display numbers / text on a seven-segment
 * display module.
 */

void sevensegment_post();
int sevensegment_init();
int sevensegment_set_int(int number);
int sevensegment_set_text(char * text);
int sevensegment_set_element(int element);
int sevensegment_set_segment(int segment, bool on);

