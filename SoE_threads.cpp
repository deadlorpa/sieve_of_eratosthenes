#include <iostream>
#include <vector>
#include <cmath> 
#include <chrono>
#include <thread>
#include <fstream>

using namespace std;

int BLOCKS_AMOUNT = 8;
int THREADS_AMOUNT = 8;
vector<long int> SIZE = { 1000, 10000, 100000, 1000000, 10000000};
vector<bool> NUMBERS;

// классическое правило Эратосфена
vector<long int> Eratosthenes(long int n)
{
    vector<long int> result;
    for (long int i = 2; i < n; ++i)
    {
        for (int j = i + i; j < n; j += i)
        {
            NUMBERS[j] = false;
        }
        if (NUMBERS[i])
            result.push_back(i);
    }
    return result;
}


// проверка блока
void BlockBolting(long int begin, long int end, vector<long int> base_primes)
{
    for (long int i = begin; i < end; ++i)
    {
        for (long int j = 0; j < base_primes.size(); ++j)
        {
            if (i % base_primes[j] == 0)
            {
                NUMBERS[i] = false;
            }
        }
    }
}

// параллельный алгоритм
void ParallEratosthenes(long int n)
{
    long int border = sqrt(n);
    vector<long int> base_primes = Eratosthenes(border);
    long int step = (n - border) / BLOCKS_AMOUNT;
    vector<thread> threads;

    for (int i = 0; i < BLOCKS_AMOUNT; ++i)
    {   
        thread t(BlockBolting, border + step * i, border + step * (i + 1), base_primes);
        threads.push_back(move(t));
    }
    for (int i = 0; i<threads.size(); i++)
    {
        threads[i].join();
    }
}


int main()
{
    setlocale(LC_ALL, "Russian");
    ofstream fout;
    cout << "Реализация с Thread" << endl;
    cout << "Число блоков: " << BLOCKS_AMOUNT << endl;
    cout << "Число потоков: " << THREADS_AMOUNT << endl;
    cout << "--------------------" << endl;
    for (int s = 0; s < SIZE.size(); s++)
    {   
        NUMBERS.clear();
        NUMBERS = vector<bool>(SIZE[s], true);
        NUMBERS[0] = NUMBERS[1] = false;
        auto begin = chrono::high_resolution_clock::now();
        ParallEratosthenes(SIZE[s]);
        auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - begin).count();
        cout << SIZE[s] << " - - " << elapsedTime << "ms" << endl;
        
        fout.open("file.txt", ios::app);
        fout << endl;
        fout << SIZE[s] << endl;
        for (int i = 0; i < NUMBERS.size(); i++)
        {
            if (NUMBERS[i])
                fout << i << " ";
        }
        fout << endl;
        fout.close();
       
    }
}

