#include <iostream>
#include <cmath>
#include <fstream>

const int pwm[50] = {1, 2, 5, 8, 11, 16, 21, 26, 32, 38, 44, 50, 56, 62, 68, 74, 79, 84, 89, 92, 95, 98, 99, 100, 100, 99, 98, 95, 92, 89, 84, 79, 74, 68, 62, 56, 50, 44, 38, 32, 26, 21, 16, 11, 8, 5, 2, 1, 0, 0};

int wave[100000] = {0};

void generateWave(int freq)
{
    const int sampleRate = 100000;        // 假设采样率为1000Hz
    const int period = sampleRate / freq; // 计算一个周期的采样点数

    for (int i = 0; i < 100000; ++i)
    {
        int pwmIndex = (i % period) * (sizeof(pwm) / sizeof(pwm[0]) - 1) / period; // 计算当前采样点对应的pwm索引
        wave[i] += pwm[pwmIndex];                                                  // 叠加到wave数组中
    }
}

int main()
{
    int freq;
    std::cout << "Enter the frequency (Hz): ";
    std::cin >> freq;

    generateWave(freq); // 生成指定频率的方波

    // 将wave数组保存到文件
    std::ofstream file("wave_data.txt");
    if (file.is_open())
    {
        for (int i = 0; i < 100000; ++i)
        {
            file << wave[i] << "\n";
        }
        file.close();
    }
    else
    {
        std::cerr << "Unable to open file" << std::endl;
        return -1;
    }

    return 0;
}