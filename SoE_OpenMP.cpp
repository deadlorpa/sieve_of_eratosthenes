#include <iostream>
#include <vector>
#include <cmath> 
#include <omp.h>
#include <chrono>
#include <fstream>

using namespace std;
int BLOCKS_AMOUNT = 8;
int THREADS_AMOUNT = 8;
vector<long int> SIZE = { 1000, 10000, 100000, 1000000, 10000000 };
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

    // в реализации ЛР2 распараллеливание происходит здесь
    // поэтому, чтобы сравнение было честным, 
    // применим средства OpenMP также только здесь
    cout << "| static 1";
    auto begin = chrono::high_resolution_clock::now();
    // динамически дадим 1 потоку по 1 блоку
    #pragma omp parallel for schedule(static,1) num_threads(THREADS_AMOUNT)
    for (int i = 0; i < BLOCKS_AMOUNT; ++i)
    {
        BlockBolting(border + step * i, border + step * (i + 1), base_primes);
    }
    
    auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - begin).count();
    cout << " - - " << elapsedTime << "ms" << endl;

    cout << "| dynamic 1";
    begin = chrono::high_resolution_clock::now();
    // динамически дадим 1 потоку по 4 блока (кому не досталось - отдыхают)
    #pragma omp parallel for schedule(dynamic,1) num_threads(THREADS_AMOUNT)
    for (int i = 0; i < BLOCKS_AMOUNT; ++i)
    {
        BlockBolting(border + step * i, border + step * (i + 1), base_primes);
    }

    elapsedTime = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - begin).count();
    cout << " - - " << elapsedTime << "ms" << endl;

    cout << "| dynamic 4";
    begin = chrono::high_resolution_clock::now();
    // динамически дадим 1 потоку по 4 блока (кому не досталось - отдыхают)
    #pragma omp parallel for schedule(dynamic,4) num_threads(THREADS_AMOUNT)
    for (int i = 0; i < BLOCKS_AMOUNT; ++i)
    {
        BlockBolting(border + step * i, border + step * (i + 1), base_primes);
    }

    elapsedTime = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - begin).count();
    cout << " - - " << elapsedTime << "ms" << endl;
}


int main()
{
    setlocale(LC_ALL, "Russian");
    ofstream fout;
    cout << "Реализация с OpenMP" << endl;
    cout << "Число блоков: " << BLOCKS_AMOUNT << endl;
    cout << "Число потоков: " << THREADS_AMOUNT << endl;
    cout << "--------------------" << endl;
    for (int s = 0; s < SIZE.size(); s++)
    {
        NUMBERS.clear();
        NUMBERS = vector<bool>(SIZE[s], true);
        NUMBERS[0] = NUMBERS[1] = false;
        cout << "Размерность: " << SIZE[s] << endl;

        ParallEratosthenes(SIZE[s]);

        fout.open("file.txt", ios::app);
        fout << endl;
        fout << SIZE[s] << endl;
        for (int i = 0; i < NUMBERS.size(); i++)
        {
            if(NUMBERS[i])
                fout << i << " ";
        }
        fout << endl;
        fout.close();
    }

}

