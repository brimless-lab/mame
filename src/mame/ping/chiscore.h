// license:BSD-3-Clause
// copyright-holders:xiaoheiyohehe


#ifndef MAME_EMU_PHISCORE_H
#define MAME_EMU_PHISCORE_H

#pragma once

#include "emu.h"
// #include "machine/gen_latch.h"
// #include "machine/watchdog.h"
#include <iostream>
#include <iomanip>

typedef struct chiscore_memory_s 
{
    offs_t address;
    int len;
} chiscore_memory;

#define MEM_LEN 20

class chiscore 
{
public:

    chiscore() 
    {
        
    }
    void init(device_t *device, cpu_device *maincpu, chiscore_memory mems[], int mem_num);

private:
    device_t* m_device;
    cpu_device* m_maincpu;
    double m_update_time;
    double m_reset_time = 0;
    // double m_game_time;

    // 分数和排行榜可能分为几个内存数据段
    chiscore_memory m_mems[MEM_LEN];
    int m_mem_num;
    int m_check_num;

    int memory_check_num();
    void check_frame();
    void check_reset();
    void check_exit();
    void check_count();
    void print(const char *event);
};

#endif // MAME_EMU_PHISCORE_H
