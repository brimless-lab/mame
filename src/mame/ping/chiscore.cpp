#include "chiscore.h"
#include "md5.h"

#define TAG "hiscore::"

int chiscore::memory_check_num() 
{
    int sum = 0;
    for (int i = 0; i < m_mem_num; i++) 
    {
        chiscore_memory mem = m_mems[i];
        for (int j = 0; j < mem.len; j++) 
        {
            sum += m_maincpu->space(AS_PROGRAM).read_byte(mem.address + j);
        }
    }
    return sum;
}

void chiscore::init(device_t* device, cpu_device* maincpu, chiscore_memory mems[], int mem_num)
{
    if (mem_num > MEM_LEN) 
    {
        throw emu_fatalerror("chiscore MEM_LEN limit!");
    }
    m_device = device;
    m_maincpu = maincpu;
    m_device->machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(&chiscore::check_reset, this));
    m_device->machine().add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&chiscore::check_frame, this));
    m_device->machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&chiscore::check_exit, this));
    m_device->machine().add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(&chiscore::check_exit, this));
    // m_device->machine().add_notifier(MACHINE_NOTIFY_COUNT, machine_notify_delegate(&chiscore::check_count, this));

    m_update_time = m_device->machine().time().as_double();
    m_mem_num = mem_num;
    memset(m_mems, 0, sizeof(m_mems));
    for (int i = 0; i < m_mem_num; i++)
    {
        m_mems[i] = mems[i];
    }
    m_check_num = memory_check_num();
}

void md5(char* hexbuf, unsigned char* decrypt32)
{
    // https://developer.51cto.com/article/646140.html
    int read_len;
    char temp[8] = {0}; 
    unsigned char decrypt[16] = {0};       
    MD5Context md5c;  
    MD5Init(&md5c); //初始化
    read_len = strlen(hexbuf); 
    MD5Update(&md5c,(unsigned char *)hexbuf, read_len);   
    MD5Final(decrypt, &md5c);
    strcpy((char *)decrypt32, ""); 

    for(int i = 0; i < 16; i++) 
    { 
        // 有点类似数字转字符串
        sprintf(temp, "%02x", decrypt[i]); 
        strcat((char *)decrypt32, temp); 
    }
}

void chiscore::check_count()
{
    std::cout << "count" << std::endl;
}

void chiscore::check_exit()
{
    print("exit");
    std::cout << TAG"time," << m_device->machine().time().as_double() << std::endl;
}

void chiscore::check_reset()
{
    m_reset_time++;
}

void chiscore::check_frame()
{
    // 至少间隔5秒才执行一次检测
    if (m_device->machine().time().as_double() - m_update_time > 5) 
    {
        m_update_time = m_device->machine().time().as_double();

        if (m_check_num == 0) 
        {
            m_check_num = memory_check_num();
        }else {
            int sum = memory_check_num();
            if (m_check_num != sum)
            {
                m_check_num = sum;
                print("newscore");
                std::cout << TAG"time," << m_device->machine().time().as_double() << std::endl;
            }
        }   
    }
}

void chiscore::print(const char* event) 
{
    std::cout << "-- hiscore output --" << std::endl;
    // for (int i = 0; i < 12; i++) 
    // {
    //     // 这里需要类型转换, 不然它会被当做char的asci码去处理, 这种情况下一般就是不打印(不可见字符)
    //     // eg:
    //     // char a = 65;
    //     // std::cout << a << std::endl;  // output A
    //     // https://stackoverflow.com/questions/19562103/uint8-t-cant-be-printed-with-cout
    //     // std::cout << std::hex << std::setw(2) << std::setfill('0') << /*这里是关键, 不然不会有输出*/ (int)m_maincpu->space(AS_PROGRAM).read_byte(0xe600 + i);
    // }

    // https://stackoverflow.com/questions/5193173/getting-cout-output-to-a-stdstring
    std::stringstream buffer;
    std::cout << TAG"event,";
    std::cout << event << ",data,";
    for (int i = 0; i < m_mem_num; i++)
    {
        chiscore_memory mem = m_mems[i];
        for (int j = 0; j < mem.len; j++) 
        {
            buffer << std::hex << std::setw(2) << std::setfill('0') << /*这里是关键, 不然不会有输出*/ (int)m_maincpu->space(AS_PROGRAM).read_byte(mem.address + j);
        }
    }
    std::cout << buffer.str();

    // sign
    unsigned char decrypt32[64] = {0};
    std::string text = std::string(buffer.str());
    text = "1008611" + text;
    md5(text.data(), decrypt32);

    std::cout << ",sign," << decrypt32 << std::endl;
}